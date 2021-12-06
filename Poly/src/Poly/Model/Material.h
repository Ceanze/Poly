#pragma once

namespace Poly
{
	class Texture;
	class TextureView;

	struct MaterialValues
	{
		glm::vec4	Albedo		= {1.0, 1.0, 1.0, 1.0};
		float		AO			= 1.0;
		float		Metallic	= 1.0;
		float		Roughness	= 1.0;
		float		IsCombined	= 0.0;
	};

	class Material
	{
	public:
		enum class Type
		{
			NONE,
			ALBEDO,
			METALIC,
			NORMAL,
			ROUGHNESS,
			AMBIENT_OCCLUSION,
			COMBINED
		};

	public:
		Material() = default;
		~Material() = default;

		void SetTexture(Type type, Texture* pTexture) { m_Textures[type] = pTexture; }

		void SetTextureView(Type type, TextureView* pTextureView) { m_TextureViews[type] = pTextureView; }

		void SetMaterialValues(MaterialValues materialValues) { m_MaterialValues = materialValues; }

		void SetUsesCombinedPBRMaterial(bool isCombined) { m_MaterialValues.IsCombined = 1.0 ? isCombined : 0.0; }

		const Texture* GetTexture(Type type) const { return m_Textures.at(type); }

		const TextureView* GetTextureView(Type type) const { return m_TextureViews.at(type); }

		const MaterialValues* GetMaterialValues() const { return &m_MaterialValues; }

		bool UsesCombinedPBRMaterial() const { return m_MaterialValues.IsCombined; }

	private:
		std::unordered_map<Type, Texture*> m_Textures;
		std::unordered_map<Type, TextureView*> m_TextureViews;
		MaterialValues m_MaterialValues = {};
	};
}