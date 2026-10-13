#include "LightSystem.h"

#include "Platform/API/Buffer.h"
#include "Poly/RenderGraph/RenderCatalog.h"
#include "Poly/RenderGraph/Shader/GPULightData.h"
#include "Poly/Scene/Components.h"
#include "Poly/Scene/Components/PointLightComponent.h"
#include "Poly/World/World.h"

#include <cstring>

namespace Poly
{
	LightSystem::LightSystem(RenderCatalog& catalog)
	    : m_Catalog(catalog)
	{}

	void LightSystem::OnInit(World& world)
	{
		m_Catalog.RegisterResource(LIGHTS_RESOURCE_NAME).WithType(EResourceType::StorageBuffer).WithScope(EResourceScope::World);
	}

	void LightSystem::Update(World& world)
	{
		if (!m_NeedsRebuild && !world.HasPendingChanges())
			return;

		m_NeedsRebuild = false;

		std::vector<GPUPointLight> pointLights;
		for (auto [entity, light, transform] : world.View<PointLightComponent, WorldTransformComponent>().each())
			pointLights.push_back({.Color = glm::vec4(light.Color * light.Intensity, 1.0f), .Position = glm::vec4(glm::vec3(transform.Matrix[3]), 1.0f)});

		const GPULightsHeader header = {.Count = glm::vec4(static_cast<float>(pointLights.size()), 0.0f, 0.0f, 0.0f)};

		std::vector<byte> data(sizeof(GPULightsHeader) + sizeof(GPUPointLight) * pointLights.size());
		std::memcpy(data.data(), &header, sizeof(GPULightsHeader));
		if (!pointLights.empty())
			std::memcpy(data.data() + sizeof(GPULightsHeader), pointLights.data(), sizeof(GPUPointLight) * pointLights.size());

		// The buffer only ever grows
		const Buffer* pBuffer = ResourceManager::Resolve(m_LightsBufferHandle);
		if (!pBuffer || pBuffer->GetDesc().Size < data.size())
		{
			ResourceManager::Destroy(m_LightsBufferHandle);
			m_LightsBufferHandle = ResourceManager::CreateStorageBuffer(data.size(), EMemoryUsage::CPU_VISIBLE, "LightSystem.Lights");
		}

		ResourceManager::UploadBufferData(m_LightsBufferHandle, data.data(), data.size());
		world.GetRenderResources().Set(LIGHTS_RESOURCE_NAME, m_LightsBufferHandle);
	}

	void LightSystem::OnShutdown(World& world)
	{
		world.GetRenderResources().Remove(LIGHTS_RESOURCE_NAME);

		ResourceManager::Destroy(m_LightsBufferHandle);
		m_LightsBufferHandle = {};
	}
} // namespace Poly
