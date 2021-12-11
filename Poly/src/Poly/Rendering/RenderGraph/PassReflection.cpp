#include "polypch.h"
#include "PassReflection.h"

namespace Poly
{
	void PassReflection::AddInput(const std::string& name, uint32 set, uint32 binding)
	{
		IOData data = {};
		data.Name			= name;
		data.IOType			= FIOType::INPUT;
		data.BindPoint		= FResourceBindPoint::NONE;
		data.Set			= set;
		data.Binding		= binding;
		AddIO(data);
	}

	void PassReflection::AddOutput(const std::string& name)
	{
		IOData data = {};
		data.Name			= name;
		data.IOType			= FIOType::OUTPUT;
		data.BindPoint		= FResourceBindPoint::NONE;
		AddIO(data);
	}

	void PassReflection::AddPassThrough(const std::string& name)
	{
		IOData data = {};
		data.Name			= name;
		data.IOType			= FIOType::INPUT | FIOType::OUTPUT;
		data.BindPoint		= FResourceBindPoint::NONE;
		AddIO(data);
	}

	void PassReflection::AddSpecialInput(const std::string& name, uint32 set, uint32 binding, ESpecialInput input)
	{
		FResourceBindPoint bindPoint = GetResourceBindPoint(input);
		IOData data = {};
		data.Name		= name;
		data.IOType		= FIOType::INPUT;
		data.BindPoint	= bindPoint;
		data.Set		= set;
		data.Binding	= binding;

		// SCENE_TEXTURES requires multiple bindings and will therefore create multiple IOData to do that
		// This is a temporary measure until either reflection or a render graph editor is fixed
		if (BitsSet(bindPoint, FResourceBindPoint::SCENE_TEXTURES))
		{
			for (uint32 i = 0; i < 6; i++)
			{
				data.Name = name + std::to_string(i);
				data.Binding = binding + i;
				AddIO(data);
			}
		}
		else
			AddIO(data);

	}

	void PassReflection::AddPushConstant(const std::string& name, FShaderStage shaderStage, uint64 size, uint64 offset)
	{
		PushConstantData data = {};
		data.Name			= name;
		data.Size			= size;
		data.Offset			= offset;
		data.ShaderStage	= shaderStage;

		auto pos = std::find_if(m_PushConstants.begin(), m_PushConstants.end(), [data](const PushConstantData& in) { return in.Offset == data.Offset || in.Name == data.Name; });
		if (pos != m_PushConstants.end())
		{
			POLY_CORE_WARN("PushConstant '{}' has already been added with the same name or offset of {}", name, offset);
			return;
		}

		m_PushConstants.push_back(data);
	}

	void PassReflection::SetFormat(const std::string& name, EFormat format)
	{
		for (auto& existing : m_IOs)
		{
			if (existing.Name == name)
			{
				existing.Format = format;

				// Check if we can set the layout that is required for this resource
				if (existing.IOType == (FIOType::INPUT | FIOType::OUTPUT)) // TODO: Check for edge cases
				{
					if (format == EFormat::D24_UNORM_S8_UINT)
						existing.TextureLayout = ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					else
						existing.TextureLayout = ETextureLayout::COLOR_ATTACHMENT_OPTIMAL;
				}
				else if (existing.IOType == FIOType::INPUT)
				{
					if (format == EFormat::D24_UNORM_S8_UINT)
						existing.TextureLayout = ETextureLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					else
						existing.TextureLayout = ETextureLayout::SHADER_READ_ONLY_OPTIMAL;
				}
				else if (existing.IOType == FIOType::OUTPUT)
				{
					if (format == EFormat::D24_UNORM_S8_UINT)
						existing.TextureLayout = ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					else
						existing.TextureLayout = ETextureLayout::COLOR_ATTACHMENT_OPTIMAL;
				}

				return;
			}
		}

		POLY_CORE_WARN("[PassReflection]: Tried to set format of {}, but that IO does not exist with the given name!", name);
	}

	void PassReflection::SetBufferSize(const std::string& name, uint32 size)
	{
		for (auto& existing : m_IOs)
		{
			if (existing.Name == name)
			{
				existing.Size = size;
				return;
			}
		}

		POLY_CORE_WARN("[PassReflection]: Tried to set size of {}, but that IO does not exist with the given name!", name);
	}

	void PassReflection::SetTextureSize(const std::string& name, uint32 width, uint32 height)
	{
		for (auto& existing : m_IOs)
		{
			if (existing.Name == name)
			{
				existing.Width = width;
				existing.Height = height;
				return;
			}
		}

		POLY_CORE_WARN("[PassReflection]: Tried to set width and height of {}, but that IO does not exist with the given name!", name);
	}

	void PassReflection::SetBindPoint(const std::string& name, FResourceBindPoint bindPoint)
	{
		for (auto& existing : m_IOs)
		{
			if (existing.Name == name)
			{
				existing.BindPoint = bindPoint;

				return;
			}
		}

		POLY_CORE_WARN("[PassReflection]: Tried to set bindpoint of {}, but that IO does not exist with the given name!", name);
	}

	void PassReflection::SetSampler(const std::string& name, Ref<Sampler> pSampler)
	{
		for (auto& existing : m_IOs)
		{
			if (existing.Name == name)
			{
				if (BitsSet(existing.BindPoint, FResourceBindPoint::SAMPLER))
				{
					existing.pSampler = pSampler;
					return;
				}

				POLY_CORE_WARN("[PassReflection]: Tried to set sampler of {}, but that IO is not bound as a sampler type!", name);
				return;
			}
		}

		POLY_CORE_WARN("[PassReflection]: Tried to set sampler of {}, but that IO does not exist with the given name!", name);
	}

	std::vector<IOData> PassReflection::GetIOData(FIOType IOType, FResourceBindPoint excludeFlags) const
	{
		std::vector<IOData> data;
		for (auto& IO : m_IOs)
		{
			if (BitsSet(IO.IOType, IOType) && !BitsSet(IO.BindPoint, excludeFlags))
				data.push_back(IO);
		}

		return data;
	}

	const IOData& PassReflection::GetIOData(const std::string& resName) const
	{
		//auto it = std::find(m_IOs.begin(), m_IOs.end(), resName);
		auto it = std::find_if(m_IOs.begin(), m_IOs.end(), [resName](const IOData& io) { return io.Name == resName; });
		if (it != m_IOs.end())
			return *it;

		POLY_CORE_WARN("Reflection with resource name {} could not be found", resName);
		return IOData();
	}

	void PassReflection::AddIO(IOData io)
	{
		// Check for existing
		for (const auto& existing : m_IOs)
		{
			if (existing.Name == io.Name)
			{
				POLY_CORE_WARN("[PassReflection]: Tried to add IO {} but it already exists!", existing.Name);
				return;
			}
		}

		m_IOs.push_back(io);
	}

	FResourceBindPoint PassReflection::GetResourceBindPoint(ESpecialInput input)
	{
		switch (input)
		{
			case ESpecialInput::SCENE_INSTANCE:	return FResourceBindPoint::SCENE_INSTANCE;
			case ESpecialInput::SCENE_MATERIAL:	return FResourceBindPoint::SCENE_MATERIAL;
			case ESpecialInput::SCENE_TEXTURES:	return FResourceBindPoint::SCENE_TEXTURES;
			case ESpecialInput::SCENE_VERTEX:	return FResourceBindPoint::SCENE_VERTEX;
			default:							return FResourceBindPoint::NONE;
		}
	}

}