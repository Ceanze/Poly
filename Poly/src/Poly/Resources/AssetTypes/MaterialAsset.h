#pragma once

#include "Poly/Model/Material.h" // TODO: Remove, contains MaterialValues for now
#include "Poly/Resources/AssetHandle.h"
#include "Poly/Resources/AssetTypes/IAsset.h"

namespace Poly
{
	class TextureAsset;

	class MaterialAsset : public IAsset
	{
	public:
		explicit MaterialAsset(AssetID assetID);

		const MaterialValues&     GetValues() const;
		AssetHandle<TextureAsset> GetTexture(Material::Type type) const;

		void SetMaterialValues(MaterialValues&& materialValues);
		void SetTexture(Material::Type type, AssetHandle<TextureAsset> texture);

	private:
		MaterialValues                           m_Values;
		std::array<AssetHandle<TextureAsset>, 7> m_Textures;
	};
} // namespace Poly