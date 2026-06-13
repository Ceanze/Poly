

namespace Poly
{
	class RenderGraph
	{
	public:
		RenderGraph()  = default;
		~RenderGraph() = default;

		RenderProgramBuilder Begin();

		PassDeclaration& RegisterPass(std::string_view name);

		FeatureDeclaration& RegisterFeature(std::string_view name);

		/*
		 * Registers a resource with the render graph.
		 * A registered resource is a contract that the resource will be available during the generated render program.
		 * Provided resources' lifetime are managed by the caller, the rest by the render program.
		 *
		 * @param name The name of the resource to register.
		 * @return A ResourceDeclaration that can optionally add additional information about the resource.
		 */
		ResourceDeclaration& RegisterResource(std::string_view name);

	private:
		PassRegistry     m_PassRegistry;
		FeatureRegistry  m_FeatureRegistry;
		ResourceRegistry m_ResourceRegistry;
	};

	{
		RenderGraph graph;
		graph.RegisterResource("SceneAlbedo")
		    .WithType(Type::Texture2D)
		    .WithInitialState(ResourceState::ShaderResource);
	}
} // namespace Poly

// Future me: The builders have changed from being named builders to declarations. This is because
// they will instead return reference to a proper delcaration that is inside the registry. This way
// we simply populate the optional data, and the registry will handle the rest. There is no need
// for any ".Build()" or similar functions, since the passes/features/resources are not accessable by the user
// outside of the render graph.
