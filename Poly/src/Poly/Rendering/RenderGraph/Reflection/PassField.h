#pragma once

#include "FieldVisibility.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Sampler;

		struct PushConstantData
		{
			std::string		Name = "";
			uint64			Size = 0;
			uint64			Offset = 0;
			FShaderStage	ShaderStage = FShaderStage::NONE;
		};

	class PassField
	{
	public:
		PassField(std::string name, FFieldVisibility visibility);

		PassField& Set(uint32 set);
		PassField& Binding(uint32 binding);

		PassField& Buffer(uint32 size);
		PassField& Texture2D(uint32 width = 0, uint32 height = 0);
		PassField& Texture3D(uint32 width = 0, uint32 height = 0, uint32 depth = 0);

		PassField& Format(EFormat format);
		PassField& TextureLayout(ETextureLayout textureLayout);
		PassField& BindPoint(FResourceBindPoint bindPoint);
		PassField& SetSampler(Ref<Sampler> pSampler);

		const std::string& GetName() const;
		FFieldVisibility GetVisibility() const;
		EFormat GetFormat() const;
		FResourceBindPoint GetBindPoint() const;
		ETextureLayout GetTextureLayout() const;
		uint32 GetSize() const;
		uint32 GetWidth() const;
		uint32 GetHeight() const;
		uint32 GetDepth() const;
		uint32 GetSet() const;
		uint32 GetBinding() const;
		Ref<Sampler> GetSampler() const;

		auto operator <=>(const PassField& other) const;

	private:
		void TryToSetTextureLayout();

		std::string	m_Name				= "";
		FFieldVisibility m_Visibility	= FFieldVisibility::UNKNOWN;
		EFormat	m_Format				= EFormat::UNDEFINED;
		FResourceBindPoint m_BindPoint	= FResourceBindPoint::NONE;
		ETextureLayout m_TextureLayout	= ETextureLayout::UNDEFINED;
		uint32 m_Size					= 0;
		uint32 m_Width					= 0;
		uint32 m_Height					= 0;
		uint32 m_Depth					= 0;
		uint32 m_Set					= 0;
		uint32 m_Binding				= 0;
		Ref<Sampler> m_pSampler			= nullptr;
	};
}