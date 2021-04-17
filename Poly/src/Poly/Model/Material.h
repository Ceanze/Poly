#pragma once

namespace Poly
{
	class Texture;
	class TextureView;

	class Material
	{
	public:
		Material() = default;
		~Material() = default;

		static Ref<Material> Create() { return CreateRef<Material>(); }

		void SetTexture(Ref<Texture> pTexture) { m_pTexture = pTexture; }

		void SetTextureView(Ref<TextureView> pTextureView) { m_pTextureView = pTextureView; }

		const Texture* GetTexture() const { return m_pTexture.get(); }

		const TextureView* GetTextureView() const { return m_pTextureView.get(); }

	private:
		Ref<Texture> m_pTexture;
		Ref<TextureView> m_pTextureView;
	};
}