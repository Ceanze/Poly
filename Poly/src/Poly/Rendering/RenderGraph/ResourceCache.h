#pragma once

#include <unordered_map>

#include "ResourceGUID.h"
#include "RenderGraphTypes.h"
#include "Poly/Rendering/RenderGraph/Reflection/PassField.h"

namespace Poly
{
	class Resource;

	class ResourceCache
	{
	private:
		struct ResourceData
		{
			std::pair<uint32, uint32>	Lifetime			= {0, 0};
			Ref<Resource>				pResource			= nullptr;
			PassResID					PassResID			= PassResID::Invalid();
			PassField					PassField			= {};
			bool						IsOutput			= false;
			bool						IsBackbufferBound	= false;
		};

	public:
		ResourceCache(RenderGraphDefaultParams defaultParams);
		~ResourceCache() = default;

		static Ref<ResourceCache> Create(RenderGraphDefaultParams defaultParams);

		/**
		 * Register an external resource which is not handled by the resource cache
		 * External resources are expected to be alive during the render graphs lifespan
		 * Additional calls to this function with same name will update the resource pointer
		 * @param resID - id of the resource to register, following "resourceName" format, i.e. no pass name prefix
		 * @param resourceInfo - resource info (pResource & autobind)
		 */
		void RegisterExternalResource(const ResID& resID, ResourceInfo resourceInfo);

		/**
		 * Registers a resource to be created
		 * @param name - name of resource following renderPass.resourceName format
		 * @param timepoint - index of render pass (from execution order)
		 * @param iodata - IOData for the resource
		 * @param alias - [optional] informs that the resource is going by another name added previously
		 */
		void RegisterResource(const PassResID& passResID, uint32 timepoint, PassField iodata, const PassResID& aliasID = PassResID::Invalid());

		void RegisterSyncResource(const PassResID& passResID, const PassResID& aliasID);

		/**
		 * Mark a resource as being the output of the graph - this will
		 * make it and all aliases to it an alias to the backbuffer for each frame
		 * @param passResID - ID of the pass resource to mark as graph output, following "passName.resourceName" format
		 * @param iodata - IOData for that resource
		 */
		void MarkOutput(const PassResID& passResID, PassField iodata);

		/**
		 * Sets the backbuffer resource for the specified window/imageIndex pair.
		 * NOTE: Needs to be re-set each time a the backbuffer becomes stale (e.g. when resizing the window)
		 * @param windowID - the window to set the backbuffer for
		 * @param imageIndex - the image index for the specified window to set the backbuffer for
		 * @param pResource - current backbuffer
		 */
		void SetBackbuffer(PolyID windowID, uint32 imageIndex, Ref<Resource> pResource);

		/**
		* Sets the current back buffer indices
		* @param windowId - the current window ID
		* @param imageIndex - the current image index for the specified window
		*/
		void SetCurrentBackbufferIndices(PolyID windowID, uint32 imageIndex);

		/**
		 * Allocated the previously registered resoruces
		 */
		void AllocateResources();

		/**
		* Reallocate the previously registed backbuffer bound resources, needed when a resize has happened
		*/
		void ReallocateBackbufferBoundResources(uint32 width, uint32 height);

		/**
		 * Get a resource's existance
		 * @param passResID - pass resource ID
		 *
		 * @return true if resource exist, false otherwise
		 */
		bool HasResource(const PassResID& passResID) const;

		/**
		 * Checks if a resource is registered as external
		 * @param resID - resource ID
		 *
		 * @return true if resource is registered, false otherwise
		 */
		bool HasResource(const ResID& resID) const;

		/**
		* Checks if a resource is registered (but not necessarily created)
		* @param passResID - pass resource ID
		*
		* @return true if resource is registered, false otherwise
		*/
		bool IsResourceRegistered(const PassResID& passRessID) const;

		/**
		 * Checks if a resource is registered as external
		 * @param resID - resource ID
		 *
		 * @return true if resource is registered, false otherwise
		 */
		bool IsResourceRegistered(const ResID& resID) const;

		/**
		 * Get a resource
		 * @param passResID - pass resource ID
		 *
		 * @return pointer to resource
		 */
		Resource* GetResource(const PassResID& passResID) const;

		/**
		 * Get an external resource
		 * @param resID - resource ID
		 *
		 * @return pointer to resource
		 */
		Resource* GetResource(const ResID& resID) const;

		/**
		* Gets the mapped resource name, i.e. resource "firstPas.output" retruns the mapped name for "connectedPass.input" if "connectedPass.input" is mapped to "firstPass.output"
		* This is used to resolve the actual resource name for a pass's input when the input is mapped to another pass's output
		*
		* @param resPassID - resource to map from, following the "passName.resource" syntax
		* @param pass - the pass to map to, following "passName" syntax, i.e. no resource suffix
		*
		* @return "resource" name that belongs to the pass - invalid if not found or not mapped
		*/
		PassResID GetMappedResourceName(const PassResID& resPassID, const PassID& passID);

		/**
		* Get the canonical GUID representing a physical resource. The canonical GUID is unique per physical
		* resource, and any aliased resource maps to the same canonical GUID.
		* For instance, if "pass1.out" is aliased to "pass2.in", calling GetCanonicalGUID on either GUID
		* will return the same canonical GUID.
		*
		* @param PassResID - the pass resource ID to get the canonical GUID for, following "passName.resource" syntax
		*
		* @return canonical ResourceGUID, or ResourceGUID::Invalid() if not found
		*/
		PassResID GetCanonicalGUID(const PassResID& passResID);

		/**
		* Update a resource size
		* WARNING: Old data will be deleted when size is changed.
		*/
		Resource* UpdateResourceSize(const PassResID& passResID, uint64 size);

		/**
		 * OR additional bindpoints into an already-registered resource's PassField.
		 * Used to widen usage flags before allocation (e.g. adding SHADER_READ after the
		 * debug texture injector decides a resource needs to be sampled).
		 * @param resourceGUID - resource to update
		 * @param additionalBindpoint - bindpoint flags to add
		 */
		void AddBindpoint(const PassResID& passResID, FResourceBindPoint additionalBindpoint);

		/**
		 * Resets the cache, losing ownership of resource and clears vectors
		 */
		void Reset();

	private:
		void CalcLifetime(std::pair<uint32, uint32>& lifetime, uint32 newTimepoint);
		void AllocateResource(ResourceData& resourceData, uint32 backbufferWidth, uint32 backbufferHeight);

		RenderGraphDefaultParams m_DefaultParams;

		std::unordered_map<PassResID, uint32> m_NameToIndex;
		std::vector<ResourceData> m_Resources;
		std::unordered_map<PassResID, uint32> m_NameToExternalIndex;
		std::vector<ResourceInfo> m_ExternalResources;
		std::vector<PassResID> m_ExternalCanonicalGUIDs;
		std::vector<std::vector<Ref<Resource>>> m_Backbuffers;
		std::unordered_map<PolyID, uint32> m_WindowIDtoIndex;
		uint32 m_CurrentWindowIndex = 0;
		uint32 m_CurrentImageIndex = 0;
	};
}