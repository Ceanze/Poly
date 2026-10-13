#include "Poly/Reflection/ComponentRegistration.h"
#include "Poly/Resources/AssetTypes/MaterialAsset.h"
#include "Poly/Resources/AssetTypes/MeshAsset.h"
#include "Poly/World/Components.h"
#include "Poly/World/Components/CameraComponent.h"
#include "Poly/World/Components/MaterialComponent.h"
#include "Poly/World/Components/MeshAssetComponent.h"
#include "Poly/World/Components/NameComponent.h"
#include "Poly/World/Components/PointLightComponent.h"

template<>
struct glz::meta<Poly::MeshAssetComponent>
{
	using T                     = Poly::MeshAssetComponent;
	static constexpr auto value = object("Mesh", &T::MeshHandle);
};

template<>
struct glz::meta<Poly::MaterialComponent>
{
	using T                     = Poly::MaterialComponent;
	static constexpr auto value = object("Material", &T::MaterialHandle);
};

namespace Poly
{
	void ComponentRegistry::RegisterBuiltInComponents()
	{
		if (s_BuiltInRegistered)
			return;

		s_BuiltInRegistered = true;

		RegisterComponent<NameComponent>("Name");
		RegisterComponent<TransformComponent>("Transform", {.Addable = false, .Removable = false}); // Every entity has one
		RegisterComponent<MeshAssetComponent>("MeshAsset");
		RegisterComponent<MaterialComponent>("Material");
		RegisterComponent<CameraComponent>("Camera");
		RegisterComponent<PointLightComponent>("PointLight");
	}
} // namespace Poly
