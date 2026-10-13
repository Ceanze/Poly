#pragma once

#include "Poly/Resources/AssetHandle.h"
#include "Poly/Resources/AssetTypes/IAsset.h"

namespace Poly
{
	class TextureAsset;

	struct MaterialValues
	{
		glm::vec4 Albedo     = {1.0, 1.0, 1.0, 1.0};
		float     AO         = 1.0;
		float     Metallic   = 1.0;
		float     Roughness  = 1.0;
		float     IsCombined = 0.0;
	};

	enum class EMaterialTextureType
	{
		NONE,
		ALBEDO,
		METALIC,
		NORMAL,
		ROUGHNESS,
		AMBIENT_OCCLUSION,
		COMBINED
	};

	class MaterialAsset : public IAsset
	{
	public:
		MaterialAsset() = default;
		explicit MaterialAsset(AssetID assetID);

		const MaterialValues&     GetValues() const;
		AssetHandle<TextureAsset> GetTexture(EMaterialTextureType type) const;

		void SetMaterialValues(MaterialValues&& materialValues);
		void SetTexture(EMaterialTextureType type, AssetHandle<TextureAsset> texture);

	private:
		MaterialValues                           m_Values;
		std::array<AssetHandle<TextureAsset>, 7> m_Textures;
	};
} // namespace Poly