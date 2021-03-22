#include "polypch.h"
#include "PassReflection.h"

namespace Poly
{
	void PassReflection::AddInput(const std::string& name)
	{
		IOData data = {};
		data.Name			= name;
		data.IOType			= FIOType::INPUT;
		data.BindPoint		= FResourceBindPoint::NONE;
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

	std::vector<IOData> PassReflection::GetIOData(FIOType IOType) const
	{
		std::vector<IOData> data;
		for (auto& IO : m_IOs)
		{
			if (BitsSet(IO.IOType, IOType))
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
}