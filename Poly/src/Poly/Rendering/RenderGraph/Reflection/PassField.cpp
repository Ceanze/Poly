#include "PassField.h"

namespace Poly
{
	PassField::PassField(std::string name, FFieldVisibility visibility)
		: m_Name(std::move(name)), m_Visibility(visibility) {}

	PassField& PassField::Set(uint32 set)
	{
		m_Set = set;
		return *this;
	}

	PassField& PassField::Binding(uint32 binding)
	{
		m_Binding = binding;
		return *this;
	}

	PassField& PassField::Buffer(uint32 size)
	{
		m_Size = size;
		return *this;
	}

	PassField& PassField::Texture2D(uint32 width, uint32 height)
	{
		m_Width = width;
		m_Height = height;
		return *this;
	}

	PassField& PassField::Texture3D(uint32 width, uint32 height, uint32 depth)
	{
		m_Width = width;
		m_Height = height;
		m_Depth = depth;
		return *this;
	}

	PassField& PassField::Format(EFormat format)
	{
		m_Format = format;

		// Sets a guessed layout based on the format - does not set if a TextureLayout already is set
		TryToSetTextureLayout();

		return *this;
	}

	PassField& PassField::TextureLayout(ETextureLayout textureLayout)
	{
		m_TextureLayout = textureLayout;
		return *this;
	}

	PassField& PassField::Bindpoint(FResourceBindPoint bindPoint)
	{
		m_BindPoint = bindPoint;
		return *this;
	}

	PassField& PassField::SetSampler(Ref<Sampler> pSampler)
	{
		m_pSampler = pSampler;
		return *this;
	}

	const std::string& PassField::GetName() const
	{
		return m_Name;
	}

	FFieldVisibility PassField::GetVisibility() const
	{
		return m_Visibility;
	}

	EFormat PassField::GetFormat() const
	{
		return m_Format;
	}

	FResourceBindPoint PassField::GetBindPoint() const
	{
		return m_BindPoint;
	}

	ETextureLayout PassField::GetTextureLayout() const
	{
		return m_TextureLayout;
	}

	uint32 PassField::GetSize() const
	{
		return m_Size;
	}

	uint32 PassField::GetWidth() const
	{
		return m_Width;
	}

	uint32 PassField::GetHeight() const
	{
		return m_Height;
	}

	uint32 PassField::GetDepth() const
	{
		return m_Depth;
	}

	uint32 PassField::GetSet() const
	{
		return m_Set;
	}

	uint32 PassField::GetBinding() const
	{
		return m_Binding;
	}

	Ref<Sampler> PassField::GetSampler() const
	{
		return m_pSampler;
	}

	auto PassField::operator<=>(const PassField& other) const
	{
		return m_Name <=> other.m_Name;
	}

	void PassField::TryToSetTextureLayout()
	{
		if (m_TextureLayout == ETextureLayout::UNDEFINED || m_Format == EFormat::UNDEFINED)
			return;


		// Check if we can set the layout that is required for this resource
		if (m_Visibility == FFieldVisibility::IN_OUT)
		{
			if (m_Format == EFormat::DEPTH_STENCIL)
				m_TextureLayout= ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			else
				m_TextureLayout = ETextureLayout::COLOR_ATTACHMENT_OPTIMAL;
		}
		else if (m_Visibility == FFieldVisibility::INPUT)
		{
			if (m_Format == EFormat::DEPTH_STENCIL)
				m_TextureLayout = ETextureLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			else
				m_TextureLayout = ETextureLayout::SHADER_READ_ONLY_OPTIMAL;
		}
		else if (m_Visibility == FFieldVisibility::OUTPUT)
		{
			if (m_Format == EFormat::DEPTH_STENCIL)
				m_TextureLayout = ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			else
				m_TextureLayout = ETextureLayout::COLOR_ATTACHMENT_OPTIMAL;
		}
	}
}