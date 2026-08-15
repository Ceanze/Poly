#include "MaterialAsset.h"

namespace Poly
{
	const MaterialValues& MaterialAsset::GetValues() const
	{
		return m_Values;
	}

	AssetHandle<TextureAsset> MaterialAsset::GetTexture(Material::Type type) const
	{
		return m_Textures[static_cast<size_t>(type)];
	}

	void MaterialAsset::SetMaterialValues(MaterialValues&& materialValues)
	{
		m_Values = std::move(materialValues);
	}

	void MaterialAsset::SetTexture(Material::Type type, AssetHandle<TextureAsset> texture)
	{
		m_Textures[static_cast<size_t>(type)] = std::move(texture);
	}
} // namespace Poly