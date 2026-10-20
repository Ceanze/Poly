#include "Renderer.h"

#include "Platform/API/CommandQueue.h"
#include "Platform/API/SwapChain.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Core/Window.h"
#include "Poly/Events/WindowEvent.h"
#include "Poly/RenderGraph/RenderCatalog.h"
#include "Poly/RenderGraph/RenderPlan.h"
#include "Poly/RenderGraph/RenderProgramInstance.h"
#include "Poly/RenderGraph/RenderViewContext.h"
#include "Poly/RenderGraph/ResourceManager.h"
#include "Poly/Rendering/Features/GeometryFeature.h"
#include "polypch.h"

#include <algorithm>

namespace Poly
{
	constexpr const uint32 BUFFER_COUNT = 3;

	namespace
	{
		// True if the reader has bound the offscreen target of the producer as one of its resources
		bool ReadsTargetOf(const RenderView& reader, const RenderView& producer)
		{
			const TextureHandle target = producer.GetTarget();
			if (!target.IsValid() || target == reader.GetTarget())
				return false;

			return std::ranges::any_of(reader.GetResources().GetEntries(), [target](const auto& entry) { return entry.second.TexHandle == target; });
		}
	} // namespace

	Renderer::Renderer()
	    : m_pCatalog(CreateRef<RenderCatalog>())
	{
		GeometryFeature::Register(*m_pCatalog);
	}

	Renderer::~Renderer()
	{
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
	}

	Unique<Renderer> Renderer::Create()
	{
		return CreateUnique<Renderer>();
	}

	Ref<RenderView> Renderer::CreateView(const RenderViewDesc& desc)
	{
		return CreateRef<RenderView>(desc);
	}

	void Renderer::SubmitView(const Ref<RenderView>& pView)
	{
		if (!pView)
		{
			POLY_CORE_ERROR("Submitted view is null");
			return;
		}

		if (std::ranges::find(m_SubmittedViews, pView) != m_SubmittedViews.end())
		{
			POLY_CORE_ERROR("View '{}' has already been submitted this frame, a view can only be rendered once per frame", pView->GetDebugName());
			return;
		}

		m_SubmittedViews.push_back(pView);
	}

	void Renderer::SetRenderPlan(Ref<RenderPlan> pRenderPlan)
	{
		// Programs and pipelines are built from the plan
		ClearPrograms();
		m_pRenderPlan = std::move(pRenderPlan);
	}

	void Renderer::AddWindow(Window* pWindow)
	{
		if (!m_Windows.empty())
			POLY_CORE_WARN("Only the first window added to the renderer is rendered to, multiple windows are not supported yet");

		SwapChainDesc swapChainDesc = {
		    .pWindow     = pWindow,
		    .pQueue      = RenderAPI::GetCommandQueue(FQueueType::GRAPHICS),
		    .Width       = pWindow->GetWidth(),
		    .Height      = pWindow->GetHeight(),
		    .BufferCount = BUFFER_COUNT,
		    .Format      = EFormat::B8G8R8A8_UNORM};
		Ref<SwapChain> pSwapChain = RenderAPI::CreateSwapChain(&swapChainDesc);

		m_Windows.push_back({pWindow, pSwapChain});
	}

	void Renderer::RemoveWindow(Window* pWindow)
	{
		std::erase_if(m_Windows, [pWindow](const WindowContext& windowCtx) { return windowCtx.pWindow == pWindow; });
	}

	void Renderer::Render()
	{
		if (m_Windows.empty())
		{
			m_SubmittedViews.clear();
			return;
		}

		const WindowContext& windowCtx    = m_Windows.front();
		TextureView*         pBackbuffer  = windowCtx.pSwapChain->GetTextureView(windowCtx.pSwapChain->GetBackbufferIndex()).get();
		const uint32         windowWidth  = pBackbuffer->GetTexture()->GetWidth();
		const uint32         windowHeight = pBackbuffer->GetTexture()->GetHeight();

		std::vector<RenderView*> views;
		CachedProgram*           pProgram = nullptr;
		if (m_pRenderPlan)
		{
			views    = OrderSubmittedViews();
			pProgram = &GetOrBuildProgram(BuildProgramKey(views));
		}

		for (RenderView* pView : views)
		{
			const Texture* pTarget = ResourceManager::Resolve(pView->GetTarget());
			pView->PrepareFrame(pTarget ? pTarget->GetWidth() : windowWidth, pTarget ? pTarget->GetHeight() : windowHeight);
		}

		ResourceManager::Update();

		if (pProgram && !pProgram->pProgram->GetPasses().empty())
		{
			std::vector<RenderViewContext> contexts;
			contexts.reserve(views.size() + 1);
			contexts.push_back({.pGlobalResources = &m_GlobalResources, .pTarget = pBackbuffer, .pTransients = &m_FrameTransients});

			for (RenderView* pView : views)
			{
				const TextureHandle target = pView->GetTarget();
				contexts.push_back({.pGlobalResources = &m_GlobalResources,
				                    .pWorld           = pView->GetWorld(),
				                    .pViewResources   = &pView->m_FrameResources,
				                    .pTarget          = target.IsValid() ? ResourceManager::ResolveView(target) : pBackbuffer,
				                    .TargetHandle     = target,
				                    .Rect             = pView->GetRect(),
				                    .pTransients      = &pView->m_Transients});
			}

			pProgram->pInstance->Execute(contexts, windowCtx.pSwapChain->GetAcquireSemaphore());
		}

		std::vector<CommandBuffer*> emptyCommandbuffers;
		windowCtx.pSwapChain->Present(emptyCommandbuffers);

		m_SubmittedViews.clear();
		m_FrameCounter++;
	}

	void Renderer::OnEvent(Event& event)
	{
		EventDispatcher eventDispatcher(event);
		eventDispatcher.Dispatch<Events::WindowResized>([this](Events::WindowResized& event) {
			for (auto& context : m_Windows)
			{
				context.pSwapChain->OnWindowResized(event.GetWidth(), event.GetHeight());
			}

			return false;
		});
	}

	std::vector<RenderView*> Renderer::OrderSubmittedViews() const
	{
		struct Candidate
		{
			RenderView* pView;
			uint32      EntryIndex;
		};

		std::vector<Candidate> candidates;
		for (const Ref<RenderView>& pView : m_SubmittedViews)
		{
			const uint32 entryIndex = m_pRenderPlan->FindViewStage(pView->GetStage());
			if (entryIndex == RenderPlan::INVALID_ENTRY)
			{
				POLY_CORE_ERROR("View '{}' belongs to view stage '{}', which is not part of the render plan; the view is not rendered", pView->GetDebugName(), pView->GetStage());
				continue;
			}

			if (!pView->GetWorld())
			{
				POLY_CORE_ERROR("View '{}' has no world; the view is not rendered", pView->GetDebugName());
				continue;
			}

			if (pView->GetTarget().IsValid() && !ResourceManager::ResolveView(pView->GetTarget()))
			{
				POLY_CORE_ERROR("The target of view '{}' is not a valid texture; the view is not rendered", pView->GetDebugName());
				continue;
			}

			candidates.push_back({pView.get(), entryIndex});
		}

		std::ranges::stable_sort(candidates, {}, &Candidate::EntryIndex);

		std::vector<RenderView*> ordered;
		ordered.reserve(candidates.size());

		for (size_t stageBegin = 0; stageBegin < candidates.size();)
		{
			std::vector<RenderView*> remaining;
			size_t                   stageEnd = stageBegin;
			for (; stageEnd < candidates.size() && candidates[stageEnd].EntryIndex == candidates[stageBegin].EntryIndex; stageEnd++)
				remaining.push_back(candidates[stageEnd].pView);

			// Repeatedly take the first view that reads none of the views still waiting
			while (!remaining.empty())
			{
				auto it = std::ranges::find_if(remaining, [&remaining](const RenderView* pReader) {
					return std::ranges::none_of(remaining, [pReader](const RenderView* pProducer) { return pProducer != pReader && ReadsTargetOf(*pReader, *pProducer); });
				});

				if (it == remaining.end())
				{
					POLY_CORE_ERROR("View '{}' is part of a cycle of views reading the targets of each other, it is rendered with what was rendered last frame",
					                remaining.front()->GetDebugName());
					it = remaining.begin();
				}

				ordered.push_back(*it);
				remaining.erase(it);
			}

			stageBegin = stageEnd;
		}

		return ordered;
	}

	ProgramKey Renderer::BuildProgramKey(const std::vector<RenderView*>& views) const
	{
		ProgramKey key(views.size());

		for (uint32 i = 0; i < views.size(); i++)
		{
			const RenderView& view = *views[i];
			ProgramViewDesc&  desc = key[i];

			desc.EntryIndex = m_pRenderPlan->FindViewStage(view.GetStage());

			// Slots and indices refer to the first view with the same target or world, so that the key only says
			// which views share them and not which targets and worlds they are
			for (uint32 j = 0; j <= i; j++)
			{
				if (view.GetTarget().IsValid() && views[j]->GetTarget() == view.GetTarget())
				{
					desc.TargetSlot = j + 1;
					break;
				}
			}

			for (uint32 j = 0; j <= i; j++)
			{
				if (views[j]->GetWorld() == view.GetWorld())
				{
					desc.WorldIndex = j;
					break;
				}
			}

			for (uint32 j = 0; j < views.size(); j++)
			{
				if (j == i || !ReadsTargetOf(view, *views[j]))
					continue;

				// Rendered after this view, so it cannot be waited for
				if (j > i)
				{
					POLY_CORE_ERROR("View '{}' reads the target of view '{}', which is rendered later in the frame; what was rendered last frame is read instead",
					                view.GetDebugName(), views[j]->GetDebugName());
					continue;
				}

				for (const auto& [name, entry] : view.GetResources().GetEntries())
				{
					if (entry.TexHandle == views[j]->GetTarget())
						desc.TargetReads.emplace_back(name, key[j].TargetSlot);
				}
			}

			// The resource table is unordered
			std::ranges::sort(desc.TargetReads);
			desc.TargetReads.erase(std::unique(desc.TargetReads.begin(), desc.TargetReads.end()), desc.TargetReads.end());
		}

		return key;
	}

	Renderer::CachedProgram& Renderer::GetOrBuildProgram(const ProgramKey& key)
	{
		auto it = std::ranges::find(m_ProgramCache, key, &CachedProgram::Key);
		if (it == m_ProgramCache.end())
		{
			if (m_ProgramCache.size() >= MAX_CACHED_PROGRAMS)
				m_ProgramCache.erase(std::ranges::min_element(m_ProgramCache, {}, &CachedProgram::LastUsedFrame));

			Ref<RenderProgram> pProgram = RenderProgramBuilder::Build(*m_pRenderPlan, key);
			m_ProgramCache.push_back({.Key = key, .pProgram = pProgram, .pInstance = CreateUnique<RenderProgramInstance>(pProgram, m_PipelineCache)});
			it = m_ProgramCache.end() - 1;
		}

		it->LastUsedFrame = m_FrameCounter;
		return *it;
	}

	void Renderer::ClearPrograms()
	{
		// Destroying a program instance waits for the frames it has in flight
		m_ProgramCache.clear();

		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
		m_PipelineCache.Clear();
	}
} // namespace Poly
