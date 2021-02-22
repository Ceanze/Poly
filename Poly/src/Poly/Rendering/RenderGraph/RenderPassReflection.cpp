#include "polypch.h"
#include "RenderPassReflection.h"

namespace Poly
{
	void RenderPassReflection::AddInput(const std::string& name)
	{
		IOData data = {};
		data.Name			= name;
		data.IOType			= EIOType::INPUT;
		data.ResourceType	= EResourceType::UNKNOWN;
		AddIO(data);
	}

	void RenderPassReflection::AddOutput(const std::string& name)
	{
		IOData data = {};
		data.Name			= name;
		data.IOType			= EIOType::OUTPUT;
		data.ResourceType	= EResourceType::UNKNOWN;
		AddIO(data);
	}

	void RenderPassReflection::AddPassThrough(const std::string& name)
	{
		IOData data = {};
		data.Name			= name;
		data.IOType			= EIOType::PASS_THROUGH;
		data.ResourceType	= EResourceType::UNKNOWN;
		AddIO(data);
	}

	void RenderPassReflection::SetFormat(const std::string& name, EFormat format)
	{
		for (auto& existing : IOs)
		{
			if (existing.Name == name)
			{
				existing.Format = format;
				return;
			}
		}

		POLY_CORE_WARN("[RenderPassReflection]: Tried to set format of {}, but that IO does not exist with the given name!", name);
	}

	void RenderPassReflection::SetBindPoint(const std::string& name, FResourceBindPoint bindPoint)
	{
		for (auto& existing : IOs)
		{
			if (existing.Name == name)
			{
				existing.BindPoint = bindPoint;
				return;
			}
		}

		POLY_CORE_WARN("[RenderPassReflection]: Tried to set bindpoint of {}, but that IO does not exist with the given name!", name);
	}

	std::vector<IOData> RenderPassReflection::GetInputs()
	{
		std::vector<IOData> data;
		for (auto& IO : IOs)
		{
			if (IO.IOType == EIOType::INPUT)
				data.push_back(IO);
		}

		return data;
	}

	std::vector<IOData> RenderPassReflection::GetOutputs()
	{
		std::vector<IOData> data;
		for (auto& IO : IOs)
		{
			if (IO.IOType == EIOType::OUTPUT)
				data.push_back(IO);
		}

		return data;
	}

	std::vector<IOData> RenderPassReflection::GetPassThroughs()
	{
		std::vector<IOData> data;
		for (auto& IO : IOs)
		{
			if (IO.IOType == EIOType::PASS_THROUGH)
				data.push_back(IO);
		}

		return data;
	}

	void RenderPassReflection::AddIO(IOData io)
	{
		// Check for existing
		for (const auto& existing : IOs)
		{
			if (existing.Name == io.Name)
			{
				POLY_CORE_WARN("[RenderPassReflection]: Tried to add IO {} but it already exists!", existing.Name);
				return;
			}
		}

		IOs.push_back(io);
	}
}