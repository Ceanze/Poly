#pragma once

namespace Poly
{
	class Texture;
	class TextureView;

	struct ManagedTexture
	{
		Ref<Texture>		pTexture		= nullptr;
		Ref<TextureView>	pTextureView	= nullptr;
	};

	enum class ResourceType
	{
		NONE = 0,
		TEXTURE = 1,
		MODEL = 2,
		MESH = 3,
		MATERIAL = 4
	};
}