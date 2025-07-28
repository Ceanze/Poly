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

		if (!m_ManualSets.contains(set))
			m_AutoBindedSets.insert(set);
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
					if (format == EFormat::DEPTH_STENCIL)
						existing.TextureLayout = ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					else
						existing.TextureLayout = ETextureLayout::COLOR_ATTACHMENT_OPTIMAL;
				}
				else if (existing.IOType == FIOType::INPUT)
				{
					if (format == EFormat::DEPTH_STENCIL)
						existing.TextureLayout = ETextureLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					else
						existing.TextureLayout = ETextureLayout::SHADER_READ_ONLY_OPTIMAL;
				}
				else if (existing.IOType == FIOType::OUTPUT)
				{
					if (format == EFormat::DEPTH_STENCIL)
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

	void PassReflection::DisableAutoBind(uint32 setIndex)
	{
		m_AutoBindedSets.erase(setIndex);
		m_ManualSets.insert(setIndex);
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
		auto it = std::find_if(m_IOs.begin(), m_IOs.end(), [resName](const IOData& io) { return io.Name == resName; });
		POLY_VALIDATE(it != m_IOs.end(), "Reflection with resource name {} could not be found", resName);

		return *it;
	}


	uint32 PassReflection::AddIO(IOData io)
	{
		// Check for existing
		for (const auto& existing : m_IOs)
		{
			if (existing.Name == io.Name)
			{
				POLY_CORE_WARN("[PassReflection]: Tried to add IO {} but it already exists!", existing.Name);
				return UINT32_MAX;
			}
		}

		uint32 index = static_cast<uint32>(m_IOs.size());
		m_IOs.push_back(io);
		return index;
	}
}
