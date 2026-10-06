#include "ImGuiLayer.h"

#include "Platform/API/Buffer.h"
#include "Platform/API/CommandBuffer.h"
#include "Platform/API/Sampler.h"
#include "Poly/Core/Application.h"
#include "Poly/Events/KeyEvent.h"
#include "Poly/Events/MouseEvent.h"
#include "Poly/Events/WindowEvent.h"
#include "Poly/RenderGraph/ExecuteContext.h"
#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/RenderGraph/RenderCatalog.h"
#include "Poly/RenderGraph/RenderResourceTable.h"
#include "Poly/Rendering/Renderer.h"

#include <imgui.h>

namespace
{
	constexpr const char* GLOBALS_RESOURCE_NAME      = "UIGlobals";
	constexpr const char* FONT_TEXTURE_RESOURCE_NAME = "FontTexture";

	// Mirrors UIGlobalsBuffer in shaders/ui_bindless.vert byte-for-byte.
	struct UIGlobalsBuffer
	{
		glm::vec2 Scale;
		glm::vec2 Translate;
	};

	// Fixed-capacity ImGui vertex/index buffers, generous enough for ImGui::ShowDemoWindow(). Avoids
	// resizing them mid-session, which would need frame-in-flight-aware deferred destruction.
	constexpr uint32 MAX_UI_VERTICES = 64 * 1024;
	constexpr uint32 MAX_UI_INDICES  = 128 * 1024;

	ImGuiKey ToImGuiKey(Poly::EKey key)
	{
		using Poly::EKey;

		if (key >= EKey::F1 && key <= EKey::F12)
			return static_cast<ImGuiKey>(ImGuiKey_F1 + (static_cast<int>(key) - static_cast<int>(EKey::F1)));
		if (key >= EKey::KEYPAD_0 && key <= EKey::KEYPAD_9)
			return static_cast<ImGuiKey>(ImGuiKey_Keypad0 + (static_cast<int>(key) - static_cast<int>(EKey::KEYPAD_0)));
		if (key >= EKey::A && key <= EKey::Z)
			return static_cast<ImGuiKey>(ImGuiKey_A + (static_cast<int>(key) - static_cast<int>(EKey::A)));

		// clang-format off
		switch (key)
		{
		case EKey::KEYPAD_DECIMAL:  return ImGuiKey_KeypadDecimal;
		case EKey::KEYPAD_DIVIDE:   return ImGuiKey_KeypadDivide;
		case EKey::KEYPAD_MULTIPLY: return ImGuiKey_KeypadMultiply;
		case EKey::KEYPAD_SUBTRACT: return ImGuiKey_KeypadSubtract;
		case EKey::KEYPAD_ADD:      return ImGuiKey_KeypadAdd;
		case EKey::KEYPAD_ENTER:    return ImGuiKey_KeypadEnter;
		case EKey::KEYPAD_EQUAL:    return ImGuiKey_KeypadEqual;
		case EKey::ESC:             return ImGuiKey_Escape;
		case EKey::TAB:             return ImGuiKey_Tab;
		case EKey::LSHIFT:          return ImGuiKey_LeftShift;
		case EKey::RSHIFT:          return ImGuiKey_RightShift;
		case EKey::LCTRL:           return ImGuiKey_LeftCtrl;
		case EKey::RCTRL:           return ImGuiKey_RightCtrl;
		case EKey::LALT:            return ImGuiKey_LeftAlt;
		case EKey::RALT:            return ImGuiKey_RightAlt;
		case EKey::SPACE:           return ImGuiKey_Space;
		case EKey::CAPS_LOCK:       return ImGuiKey_CapsLock;
		case EKey::SCROLL_LOCK:     return ImGuiKey_ScrollLock;
		case EKey::NUM_LOCK:        return ImGuiKey_NumLock;
		case EKey::BACKSPACE:       return ImGuiKey_Backspace;
		case EKey::ENTER:           return ImGuiKey_Enter;
		case EKey::DEL:             return ImGuiKey_Delete;
		case EKey::LEFT:            return ImGuiKey_LeftArrow;
		case EKey::RIGHT:           return ImGuiKey_RightArrow;
		case EKey::UP:              return ImGuiKey_UpArrow;
		case EKey::DOWN:            return ImGuiKey_DownArrow;
		case EKey::PAGE_UP:         return ImGuiKey_PageUp;
		case EKey::PAGE_DOWN:       return ImGuiKey_PageDown;
		case EKey::HOME:            return ImGuiKey_Home;
		case EKey::END:             return ImGuiKey_End;
		case EKey::INSERT:          return ImGuiKey_Insert;
		case EKey::PRINT_SCREEN:    return ImGuiKey_PrintScreen;
		case EKey::PAUSE:           return ImGuiKey_Pause;
		case EKey::MENU:            return ImGuiKey_Menu;
		case EKey::APOSTROPHE:      return ImGuiKey_Apostrophe;
		case EKey::COMMA:           return ImGuiKey_Comma;
		case EKey::PERIOD:          return ImGuiKey_Period;
		case EKey::MINUS:           return ImGuiKey_Minus;
		case EKey::SLASH:           return ImGuiKey_Slash;
		case EKey::SEMICOLON:       return ImGuiKey_Semicolon;
		case EKey::EQUAL:           return ImGuiKey_Equal;
		case EKey::LBRACKET:        return ImGuiKey_LeftBracket;
		case EKey::RBRACKET:        return ImGuiKey_RightBracket;
		case EKey::BACKSLASH:       return ImGuiKey_Backslash;
		default:                    return ImGuiKey_None;
		}
		// clang-format on
	}

	void AddKeyEvent(Poly::EKey key, Poly::FKeyModifier modifiers, bool down)
	{
		using Poly::EKey;
		using Poly::FKeyModifier;

		const bool ctrl  = (key == EKey::LCTRL || key == EKey::RCTRL) ? down : BitsSet(modifiers, FKeyModifier::CTRL);
		const bool shift = (key == EKey::LSHIFT || key == EKey::RSHIFT) ? down : BitsSet(modifiers, FKeyModifier::SHIFT);
		const bool alt   = (key == EKey::LALT || key == EKey::RALT) ? down : BitsSet(modifiers, FKeyModifier::ALT);

		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(ImGuiMod_Ctrl, ctrl);
		io.AddKeyEvent(ImGuiMod_Shift, shift);
		io.AddKeyEvent(ImGuiMod_Alt, alt);

		const ImGuiKey imguiKey = ToImGuiKey(key);
		if (imguiKey != ImGuiKey_None)
			io.AddKeyEvent(imguiKey, down);
	}
} // namespace

namespace Poly
{
	ImGuiLayer::ImGuiLayer()
	{
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		Window*  pWindow           = Application::Get().GetWindow();
		ImGuiIO& io                = ImGui::GetIO();
		io.DisplaySize             = ImVec2(pWindow->GetWidth(), pWindow->GetHeight());
		io.DisplayFramebufferScale = ImVec2(pWindow->GetContentScaleX(), pWindow->GetContentScaleY());
	}

	ImGuiLayer::~ImGuiLayer()
	{
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnAttach()
	{
		ImGuiIO& io = ImGui::GetIO();

		unsigned char* pFontData = nullptr;
		int            width = 0, height = 0;
		io.Fonts->GetTexDataAsRGBA32(&pFontData, &width, &height);

		m_FontTextureHandle = ResourceManager::CreateTexture2D(width, height, EFormat::R8G8B8A8_UNORM, FTextureUsage::SAMPLED, "ImGui Font Atlas");
		ResourceManager::UploadTextureData(m_FontTextureHandle, pFontData, width, height);

		SamplerDesc samplerDesc  = {};
		samplerDesc.MagFilter    = EFilter::LINEAR;
		samplerDesc.MinFilter    = EFilter::LINEAR;
		samplerDesc.MipMapMode   = ESamplerMipmapMode::LINEAR;
		samplerDesc.AddressModeU = ESamplerAddressMode::CLAMP_TO_EDGE;
		samplerDesc.AddressModeV = ESamplerAddressMode::CLAMP_TO_EDGE;
		samplerDesc.AddressModeW = ESamplerAddressMode::CLAMP_TO_EDGE;
		samplerDesc.BorderColor  = EBorderColor::FLOAT_OPAQUE_WHITE;
		m_FontSamplerHandle      = ResourceManager::GetOrCreateSampler(samplerDesc);

		io.Fonts->TexID = (ImTextureID)m_FontTextureHandle.Get();

		m_GlobalsBufferHandle = ResourceManager::CreateUniformBuffer(sizeof(UIGlobalsBuffer), "UIGlobals");
		m_VertexBufferHandle  = ResourceManager::CreateVertexBuffer(MAX_UI_VERTICES * sizeof(ImDrawVert), EMemoryUsage::CPU_VISIBLE, "UI Vertices");
		m_IndexBufferHandle   = ResourceManager::CreateIndexBuffer(MAX_UI_INDICES * sizeof(ImDrawIdx), EMemoryUsage::CPU_VISIBLE, "UI Indices");

		RenderResourceTable& globalResources = Application::Get().GetRenderer()->GetGlobalResources();
		globalResources.Set(FONT_TEXTURE_RESOURCE_NAME, m_FontTextureHandle, m_FontSamplerHandle);
		globalResources.Set(GLOBALS_RESOURCE_NAME, m_GlobalsBufferHandle);
	}

	void ImGuiLayer::BeginFrame()
	{
		ImGui::NewFrame();
	}

	void ImGuiLayer::EndFrame()
	{
		ImGui::Render();

		ImDrawData* pDrawData = ImGui::GetDrawData();
		if (!pDrawData || !pDrawData->Valid)
			return;

		ImGuiIO&        io      = ImGui::GetIO();
		UIGlobalsBuffer globals = {};
		globals.Scale           = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		globals.Translate       = glm::vec2(-1.0f, -1.0f);
		ResourceManager::UploadBufferData(m_GlobalsBufferHandle, &globals, sizeof(UIGlobalsBuffer));

		Buffer* pVertexBuffer = ResourceManager::Resolve(m_VertexBufferHandle);
		Buffer* pIndexBuffer  = ResourceManager::Resolve(m_IndexBufferHandle);

		uint64 vertexOffset = 0;
		uint64 indexOffset  = 0;
		for (int i = 0; i < pDrawData->CmdListsCount; i++)
		{
			const ImDrawList* pCmdList = pDrawData->CmdLists[i];

			uint64 vertexBufferSize = pCmdList->VtxBuffer.Size * sizeof(ImDrawVert);
			uint64 indexBufferSize  = pCmdList->IdxBuffer.Size * sizeof(ImDrawIdx);

			if (vertexOffset + vertexBufferSize > pVertexBuffer->GetSize() || indexOffset + indexBufferSize > pIndexBuffer->GetSize())
			{
				POLY_CORE_WARN("ImGui draw data exceeds the fixed UI buffer capacity - dropping remaining draw lists");
				break;
			}

			ResourceManager::UploadBufferData(m_VertexBufferHandle, pCmdList->VtxBuffer.Data, vertexBufferSize, vertexOffset);
			ResourceManager::UploadBufferData(m_IndexBufferHandle, pCmdList->IdxBuffer.Data, indexBufferSize, indexOffset);

			vertexOffset += vertexBufferSize;
			indexOffset += indexBufferSize;
		}
	}

	void ImGuiLayer::RegisterRenderFeature(RenderCatalog& catalog)
	{
		catalog.RegisterResource(GLOBALS_RESOURCE_NAME).WithType(EResourceType::UniformBuffer);
		catalog.RegisterResource(FONT_TEXTURE_RESOURCE_NAME).WithType(EResourceType::SampledImage);

		// clang-format off
		catalog.RegisterPass("ui")
		    .WithShader("assets/shaders/ui_bindless.vert", FShaderStage::VERTEX)
		    .WithShader("assets/shaders/ui_bindless.frag", FShaderStage::FRAGMENT)
		    .MapResource(EFeaturePort::Color, "out_Color") // auto-inferred LOAD op: an earlier feature already wrote $Color first
		    .MapGlobal(GLOBALS_RESOURCE_NAME, "globals")
		    .MapGlobal(FONT_TEXTURE_RESOURCE_NAME, "sTexture")
		    .WithGraphicsPipeline()
		    .AddVertexInput()
				.Binding(0)
				// TODO: Set Stride/VertexInputRate on the pipeline, not per-vertex-input, since PVKGraphicsPipeline only reads them off the first vertex input.
				// PVKGraphicsPipeline only reads Stride/VertexInputRate off VertexInputs[0] (one shared
				// binding for the whole pipeline) - has to be set here even though it describes binding 0
				// as a whole, not just the "pos" attribute.
				.Stride(sizeof(ImDrawVert))
				.VertexInputRate(EVertexInputRate::VERTEX)
				.Location(0)
				.Format(EFormat::R32G32_SFLOAT)
				.Offset(offsetof(ImDrawVert, pos))
		    .AddVertexInput()
				.Binding(0)
				.Location(1)
				.Format(EFormat::R32G32_SFLOAT)
				.Offset(offsetof(ImDrawVert, uv))
		    .AddVertexInput()
				.Binding(0)
				.Location(2)
				.Format(EFormat::R8G8B8A8_UNORM)
				.Offset(offsetof(ImDrawVert, col))
		    .FinishVertexInput()
		    .Topology(ETopology::TRIANGLE_LIST)
		    .PolygonMode(EPolygonMode::FILL)
		    .CullMode(ECullMode::NONE)
		    .ClockwiseFrontFace(true)
		    .ViewportDynamic(true)
		    .ScissorDynamic(true)
		    .DepthTestEnable(false)
		    .DepthWriteEnable(false)
		    .AddColorBlendAttachment()
				.BlendEnable(true)
				.SrcColorBlendFactor(EBlendFactor::SRC_ALPHA)
				.DstColorBlendFactor(EBlendFactor::ONE_MINUS_SRC_ALPHA)
				.ColorBlendOp(EBlendOp::ADD)
				.SrcAlphaBlendFactor(EBlendFactor::ONE_MINUS_SRC_ALPHA)
				.DstAlphaBlendFactor(EBlendFactor::ZERO)
				.AlphaBlendOp(EBlendOp::ADD)
				.ColorWriteMask(FColorComponentFlag::RED | FColorComponentFlag::GREEN | FColorComponentFlag::BLUE |
								FColorComponentFlag::ALPHA)
		    .FinishColorBlendAttachment()
		    .FinishPipeline()
		    .WithExecuteFn([this](ExecuteContext& ctx) {
			    ImDrawData* pDrawData = ImGui::GetDrawData();
			    if (!pDrawData || !pDrawData->Valid || pDrawData->CmdListsCount == 0)
				    return;

			    CommandBuffer* pCmd = ctx.GetCommandBuffer();
			    pCmd->BindVertexBuffer(ResourceManager::Resolve(m_VertexBufferHandle), 0, 1, 0);
			    pCmd->BindIndexBuffer(ResourceManager::Resolve(m_IndexBufferHandle), 0, EIndexType::UINT16);

			    uint32 vertexOffset = 0;
			    uint32 indexOffset  = 0;
			    for (int i = 0; i < pDrawData->CmdListsCount; i++)
			    {
				    const ImDrawList* pCmdList = pDrawData->CmdLists[i];
				    for (int j = 0; j < pCmdList->CmdBuffer.Size; j++)
				    {
					    const ImDrawCmd* pImCmd = &pCmdList->CmdBuffer[j];

					    ScissorDesc scissor = {};
					    scissor.OffsetX     = std::max(static_cast<int>(pImCmd->ClipRect.x), 0);
					    scissor.OffsetY     = std::max(static_cast<int>(pImCmd->ClipRect.y), 0);
					    scissor.Width       = static_cast<uint32>(pImCmd->ClipRect.z - pImCmd->ClipRect.x);
					    scissor.Height      = static_cast<uint32>(pImCmd->ClipRect.w - pImCmd->ClipRect.y);
					    pCmd->SetScissor(&scissor);

					    ImTextureID   texID = pImCmd->TexRef.GetTexID();
					    TextureHandle textureHandle(static_cast<uint32>(texID));
					    ctx.SetTextureSlot(0, textureHandle, m_FontSamplerHandle);

					    pCmd->DrawIndexedInstanced(pImCmd->ElemCount, 1, indexOffset, vertexOffset, 0);
					    indexOffset += pImCmd->ElemCount;
				    }
				    vertexOffset += pCmdList->VtxBuffer.Size;
			    }
		    });
		// clang-format on

		catalog.RegisterFeature(FEATURE_NAME).WithPass("ui");
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		EventDispatcher eventDispatcher(event);
		eventDispatcher.Dispatch<Events::MouseMoved>([this](auto& event) { return OnMouseMoved(event); });
		eventDispatcher.Dispatch<Events::MouseButtonPressed>([this](auto& event) { return OnMouseButtonPressed(event); });
		eventDispatcher.Dispatch<Events::MouseButtonReleased>([this](auto& event) { return OnMouseButtonReleased(event); });
		eventDispatcher.Dispatch<Events::MouseScrolled>([this](auto& event) { return OnMouseScrolled(event); });
		eventDispatcher.Dispatch<Events::KeyPressed>([this](auto& event) { return OnKeyPressed(event); });
		eventDispatcher.Dispatch<Events::KeyReleased>([this](auto& event) { return OnKeyReleased(event); });
		eventDispatcher.Dispatch<Events::KeyTyped>([this](auto& event) { return OnKeyTyped(event); });
		eventDispatcher.Dispatch<Events::WindowResized>([this](auto& event) { return OnWindowResized(event); });
	}

	bool ImGuiLayer::OnMouseMoved(Events::MouseMoved& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(static_cast<float>(event.GetX()) * io.DisplayFramebufferScale.x,
		                     static_cast<float>(event.GetY()) * io.DisplayFramebufferScale.y);

		return false;
	}

	bool ImGuiLayer::OnMouseButtonPressed(Events::MouseButtonPressed& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (!io.WantCaptureMouse)
			return false;

		if (event.GetButton() == EKey::MOUSE_LEFT)
			io.MouseDown[0] = true;
		else if (event.GetButton() == EKey::MOUSE_RIGHT)
			io.MouseDown[1] = true;

		return true;
	}

	bool ImGuiLayer::OnMouseButtonReleased(Events::MouseButtonReleased& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (!io.WantCaptureMouse)
			return false;

		if (event.GetButton() == EKey::MOUSE_LEFT)
			io.MouseDown[0] = false;
		else if (event.GetButton() == EKey::MOUSE_RIGHT)
			io.MouseDown[1] = false;

		return true;
	}

	bool ImGuiLayer::OnMouseScrolled(Events::MouseScrolled& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (!io.WantCaptureMouse)
			return false;

		io.MouseWheel  = static_cast<float>(event.GetDeltaY());
		io.MouseWheelH = static_cast<float>(event.GetDeltaX());

		return true;
	}

	bool ImGuiLayer::OnKeyPressed(Events::KeyPressed& event)
	{
		AddKeyEvent(event.GetKey(), event.GetKeyModifier(), true);

		return ImGui::GetIO().WantCaptureKeyboard;
	}

	bool ImGuiLayer::OnKeyReleased(Events::KeyReleased& event)
	{
		AddKeyEvent(event.GetKey(), event.GetKeyModifier(), false);

		// Never consumed, a layer that saw the press must also see the release
		return false;
	}

	bool ImGuiLayer::OnKeyTyped(Events::KeyTyped& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(event.GetCodepoint());

		return io.WantTextInput;
	}

	bool ImGuiLayer::OnWindowResized(Events::WindowResized& event)
	{
		Window*  pWindow           = Application::Get().GetWindow();
		ImGuiIO& io                = ImGui::GetIO();
		io.DisplaySize             = ImVec2(static_cast<float>(event.GetWidth()), static_cast<float>(event.GetHeight()));
		io.DisplayFramebufferScale = ImVec2(pWindow->GetContentScaleX(), pWindow->GetContentScaleY());

		return false;
	}
} // namespace Poly
