#include "RenderCatalog.h"
#include "RenderProgramBuilder.h"

namespace Poly
{
	class RenderGraph
	{
	public:
		RenderGraph();
		explicit RenderGraph(Ref<RenderCatalog> catalog);
		~RenderGraph() = default;

		RenderProgramBuilder Begin();

		IPassDeclaration& RegisterPass(std::string_view name);

		IFeatureDeclaration& RegisterFeature(std::string_view name);

		/*
		 * Registers a resource with the render graph.
		 * A registered resource is a contract that the resource will be available during the generated render program.
		 * Provided resources' lifetime are managed by the caller, the rest by the render program.
		 *
		 * @param name The name of the resource to register.
		 * @return A ResourceDeclaration that can optionally add additional information about the resource.
		 */
		IResourceDeclaration& RegisterResource(std::string_view name);

	private:
		Ref<RenderCatalog> m_Catalog;
	};

	// {
	// 	RenderCatalog catalog;
	// 	catalog.RegisterPass("imgui").WithShader(...).WithExecuteFn(...);
	// 	catalog.RegisterFeature("debug").WithPass("imgui");
	//
	// 	RenderGraph graph(catalog);
	// 	graph.RegisterResource("SceneAlbedo")
	// 	    .WithType(Type::Texture2D)
	// 	    .WithInitialState(ResourceState::ShaderResource);
	// }
} // namespace Poly
