#pragma once

#include <string>
#include <vector>

namespace Poly
{
	struct ResolvedPort
	{
		std::string shaderName;   // local name in the pass shader
		std::string resolvedName; // globally resolved name (e.g. "$Color", "SceneAlbedo")
		bool        isWrite;      // true = output, false = input
	};

	struct ResolvedPass
	{
		std::string               name;
		std::vector<ResolvedPort> ports;
	};

	class RenderProgram
	{
	public:
		explicit RenderProgram(std::vector<ResolvedPass> sortedPasses);

		const std::vector<ResolvedPass>& GetPasses() const { return m_Passes; }

	private:
		std::vector<ResolvedPass> m_Passes;
	};
} // namespace Poly
