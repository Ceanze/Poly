#pragma once

#include "ExecutorRegistry.h"
#include "Poly/Core/Result.h"
#include "Registry.h"
#include "VertexLayoutRegistry.h"
#include "Pass/PassDeclaration.h"
#include "Feature/FeatureDeclaration.h"
#include "Resource/ResourceDeclaration.h"

namespace Poly
{
	class IPassDeclaration;
	class IFeatureDeclaration;
	class IResourceDeclaration;

	using PassRegistry     = Registry<PassDeclaration>;
	using FeatureRegistry  = Registry<FeatureDeclaration>;
	using ResourceRegistry = Registry<ResourceDeclaration>;

	class RenderCatalog
	{
	public:
		RenderCatalog()  = default;
		~RenderCatalog() = default;

		/*
		 * Registers a new, empty declaration, replacing one already registered with the same name.
		 * @param name The name of the declaration.
		 * @param source The path of the file the declaration is read from, empty if registered from code.
		 */
		IPassDeclaration&     RegisterPass(std::string_view name, std::string_view source = {});
		IFeatureDeclaration&  RegisterFeature(std::string_view name, std::string_view source = {});
		IResourceDeclaration& RegisterResource(std::string_view name, std::string_view source = {});

		/*
		 * Registers every pass (.polypass) and feature (.polyfeature, with the resources it owns) of a directory and its
		 * subdirectories, with its file as source. A file that cannot be read, or that declares a name already registered
		 * by code or by another file, is logged and left out.
		 * @param vfsDirectory VFS path of the directory, e.g. "assets/render/".
		 * @return True if every file was registered.
		 */
		bool LoadDirectory(std::string_view vfsDirectory);

		/*
		 * Registers the pass or feature of a file, replacing what was registered from the same file before. Nothing is
		 * registered if the file cannot be read, or if it declares a name already registered by code or another file.
		 * @param vfsPath VFS path of a .polypass or .polyfeature file.
		 */
		Result<void> LoadFile(std::string_view vfsPath);

		/*
		 * Removes every pass, feature and resource registered from a file, e.g. before the file is read again.
		 * @param source The path of the file.
		 */
		void RemoveFromSource(std::string_view source);

		PassRegistry&           GetPassRegistry() { return m_PassRegistry; }
		const PassRegistry&     GetPassRegistry() const { return m_PassRegistry; }
		FeatureRegistry&        GetFeatureRegistry() { return m_FeatureRegistry; }
		const FeatureRegistry&  GetFeatureRegistry() const { return m_FeatureRegistry; }
		ResourceRegistry&       GetResourceRegistry() { return m_ResourceRegistry; }
		const ResourceRegistry& GetResourceRegistry() const { return m_ResourceRegistry; }
		ExecutorRegistry&       GetExecutorRegistry() { return m_ExecutorRegistry; }
		const ExecutorRegistry& GetExecutorRegistry() const { return m_ExecutorRegistry; }

		VertexLayoutRegistry&       GetVertexLayoutRegistry() { return m_VertexLayoutRegistry; }
		const VertexLayoutRegistry& GetVertexLayoutRegistry() const { return m_VertexLayoutRegistry; }

	private:
		PassRegistry         m_PassRegistry{"Pass"};
		FeatureRegistry      m_FeatureRegistry{"Feature"};
		ResourceRegistry     m_ResourceRegistry{"Resource"};
		ExecutorRegistry     m_ExecutorRegistry;
		VertexLayoutRegistry m_VertexLayoutRegistry;
	};
} // namespace Poly
