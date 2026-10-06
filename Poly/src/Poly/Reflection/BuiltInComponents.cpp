#include "Poly/Reflection/ComponentRegistration.h"
#include "Poly/Resources/AssetTypes/MaterialAsset.h"
#include "Poly/Resources/AssetTypes/MeshAsset.h"
#include "Poly/Scene/Components.h"
#include "Poly/Scene/Components/MaterialComponent.h"
#include "Poly/Scene/Components/MeshAssetComponent.h"
#include "Poly/Scene/Components/NameComponent.h"

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
	}
} // namespace Poly
