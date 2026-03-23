#include "EdgeData.h"

namespace Poly
{
	EdgeData::EdgeData() {}

	EdgeData::EdgeData(const PassID& srcPass, const PassID& dstPass)
		: m_EdgeData(PassToPassEdgeData{ srcPass, dstPass }) {}

	EdgeData::EdgeData(const ResID& srcExternalResource, const PassResID& dstPass)
		: m_EdgeData(ExternalResourceToPassResourceEdgeData{ srcExternalResource, dstPass }) {}

	EdgeData::EdgeData(const PassResID& srcPassRes, const PassResID& dstPassRes)
		: m_EdgeData(PassResourceToPassResourceEdgeData{ srcPassRes, dstPassRes }) {}

	const ResID& EdgeData::GetSrcExternalResource() const
	{
		POLY_VALIDATE(IsExternalResourceToPassResource(), "Attempting to get source external resource of an edge that is not an external resource to pass edge");

		return std::get<ExternalResourceToPassResourceEdgeData>(m_EdgeData).SrcExternalResource;
	}

	const PassID& EdgeData::GetSrcPass() const
	{
		POLY_VALIDATE(IsPassToPass(), "Attempting to get source pass of an edge that is not a pass to pass edge");

		return std::get<PassToPassEdgeData>(m_EdgeData).SrcPass;
	}

	const PassID& EdgeData::GetDstPass() const
	{
		if (IsPassToPass())
		{
			return std::get<PassToPassEdgeData>(m_EdgeData).DstPass;
		}
		else if (IsExternalResourceToPassResource())
		{
			return std::get<ExternalResourceToPassResourceEdgeData>(m_EdgeData).DstPassRes.GetPass();
		}
		else
		{
			POLY_VALIDATE(false, "Attempting to get destination pass of an edge that is not a pass to pass or external resource to pass edge");
			static PassID dummy;
			return dummy;
		}
	}

	const PassResID& EdgeData::GetSrcPassRes() const
	{
		POLY_VALIDATE(IsPassResourceToPassResource(), "Attempting to get source pass resource of an edge that is not a pass resource to pass resource edge");

		return std::get<PassResourceToPassResourceEdgeData>(m_EdgeData).SrcPassRes;
	}

	const PassResID& EdgeData::GetDstPassRes() const
	{
		if (IsPassResourceToPassResource())
		{
			return std::get<PassResourceToPassResourceEdgeData>(m_EdgeData).DstPassRes;
		}
		else if (IsExternalResourceToPassResource())
		{
			return std::get<ExternalResourceToPassResourceEdgeData>(m_EdgeData).DstPassRes;
		}
		else
		{
			POLY_VALIDATE(false, "Attempting to get destination pass resource of an edge that is not a pass resource to pass resource or external resource to pass edge");
			static PassResID dummy;
			return dummy;
		}
	}

	PassResID EdgeData::GetSrcPassResOrExternal() const
	{
		POLY_VALIDATE(IsPassResourceToPassResource() || IsExternalResourceToPassResource(), "Attempting to get source pass resource of an edge that is not a pass or external resource to pass resource edge");

		if (IsExternalResourceToPassResource())
			return GetSrcExternalResource().GetAsExternal();

		return GetSrcPassRes();
	}

	bool EdgeData::IsExecutionDependency() const
	{
		return IsPassToPass();
	}

	bool EdgeData::IsDataDependency() const
	{
		return IsExternalResourceToPassResource() || IsPassResourceToPassResource();
	}

	bool EdgeData::IsExternalResourceToPassResource() const
	{
		return std::holds_alternative<ExternalResourceToPassResourceEdgeData>(m_EdgeData);
	}

	bool EdgeData::IsPassResourceToPassResource() const
	{
		return std::holds_alternative<PassResourceToPassResourceEdgeData>(m_EdgeData);
	}

	bool EdgeData::IsPassToPass() const
	{
		return std::holds_alternative<PassToPassEdgeData>(m_EdgeData);
	}
}