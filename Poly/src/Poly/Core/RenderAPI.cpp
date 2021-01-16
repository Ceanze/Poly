#include "polypch.h"
#include "RenderAPI.h"

#include "Platform/Vulkan/PVKInstance.h"
#include "Poly/Core/Window.h"

namespace Poly
{
	void RenderAPI::Init(BackendAPI backendAPI, Window* pWindow)
	{
		switch (backendAPI)
		{
		case BackendAPI::VULKAN:
		{
			m_pGraphicsInstance = new PVKInstance();
			m_pGraphicsInstance->Init(pWindow);
			break;
		}
		default:
			POLY_ASSERT(false, "No valid backendAPI selected!");
		}
	}

	void RenderAPI::Release()
	{
		if (m_pGraphicsInstance)
		{
			m_pGraphicsInstance->Cleanup();
			delete m_pGraphicsInstance;
		}
	}

	Ref<Buffer> RenderAPI::CreateBuffer(const BufferDesc* pDesc)
	{
		return m_pGraphicsInstance->CreateBuffer(pDesc);
	}
}