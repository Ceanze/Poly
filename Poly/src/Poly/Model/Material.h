#pragma once

namespace Poly
{
	class Texture;
	class TextureView;

	class Material
	{
	public:
		enum class Type
		{
			NONE,
			ALBEDO,
			METALIC,
			NORMAL
		};

	public:
		Material() = default;
		~Material() = default;

		void SetTexture(Type type, Texture* pTexture) { m_Textures[type] = pTexture; }

		void SetTextureView(Type type, TextureView* pTextureView) { m_TextureViews[type] = pTextureView; }

		const Texture* GetTexture(Type type) const { return m_Textures.at(type); }

		const TextureView* GetTextureView(Type type) const { return m_TextureViews.at(type); }

	private:
		std::unordered_map<Type, Texture*> m_Textures;
		std::unordered_map<Type, TextureView*> m_TextureViews;
	};
}