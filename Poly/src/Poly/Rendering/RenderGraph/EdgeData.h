#pragma once

#include "Poly/Rendering/RenderGraph/PassID.h"
#include "Poly/Rendering/RenderGraph/PassResID.h"
#include "Poly/Rendering/RenderGraph/ResID.h"

#include <variant>

namespace Poly
{
	class EdgeData
	{
	public:
		EdgeData();

		/**
		 * Constructor for a Pass - Pass connection.
		 * This is used for execution dependencies and will not have any resource data associated with it.
		 * @param srcPass - Source pass of the edge
		 * @param dstPass - Destination pass of the edge
		 */
		EdgeData(const PassID& srcPass, const PassID& dstPass);

		/**
		 * Constructor for an external resource to pass connection.
		 * This is used for data dependencies where the source is an external resource and the destination is a pass resource.
		 * @param srcExternalResource - Source external resource of the edge
		 * @param dstPass - Destination pass resource of the edge
		 */
		EdgeData(const ResID& srcExternalResource, const PassResID& dstPass);

		/**
		 * Constructor for a pass resource to a pass resource connection.
		 * This is used for data dependencies where the source is a pass resource and the destination is a pass resource.
		 * @param srcPassRes - Source pass resource of the edge
		 * @param dstPassRes - Destination pass resource of the edge
		 */
		EdgeData(const PassResID& srcPassRes, const PassResID& dstPass);

		/**
		 * Gets the source external resource of the edge. Will only be valid if this edge is an external resource to pass edge, asserts otherwise.
		 * @return Source external resource of the edge
		 */
		const ResID& GetSrcExternalResource() const;

		/**
		 * Gets the source pass of the edge. Will only be valid if this edge is a pass to pass edge, asserts otherwise.
		 * @return Source pass of the edge
		 */
		const PassID& GetSrcPass() const;

		/**
		 * Gets the destination pass of the edge. Will be valid for both pass to pass edges and external resource to pass edges, asserts otherwise.
		 * @return Destination pass of the edge
		 */
		const PassID& GetDstPass() const;

		/**
		 * Gets the source pass resource of the edge. Will only be valid if this edge is a pass resource to pass resource edge, asserts otherwise.
		 * @return Source pass resource of the edge
		 */
		const PassResID& GetSrcPassRes() const;

		/**
		 * Gets the destination pass resource of the edge. Will only be valid if this edge is a pass resource to pass resource, or external resource to pass resource edge, asserts otherwise.
		 * @return Destination pass resource of the edge
		 */
		const PassResID& GetDstPassRes() const;

		/**
		 * Gets the source pass resource of the edge. Will be valid for both pass to pass edges and external resource to pass edges, asserts otherwise.
		 * Creates a new PassResID to support external resources - prefer to use other specific functions if behaviour is known.
		 */
		PassResID GetSrcPassResOrExternal() const;

		/**
		 * @returns true if this edge is an execution (pass-to-pass) dependency, false otherwise
		 */
		bool IsExecutionDependency() const;

		/**
		 * @returns true if this edge is a data (external/pass resource to pass resource) dependency, false otherwise
		 */
		bool IsDataDependency() const;

		bool IsExternalResourceToPassResource() const;
		bool IsPassResourceToPassResource() const;
		bool IsPassToPass() const;

	private:
		struct PassToPassEdgeData
		{
			PassID SrcPass;
			PassID DstPass;
		};

		struct ExternalResourceToPassResourceEdgeData
		{
			ResID     SrcExternalResource;
			PassResID DstPassRes;
		};

		struct PassResourceToPassResourceEdgeData
		{
			PassResID SrcPassRes;
			PassResID DstPassRes;
		};

		std::variant<PassToPassEdgeData, ExternalResourceToPassResourceEdgeData, PassResourceToPassResourceEdgeData> m_EdgeData;
	};
} // namespace Poly
