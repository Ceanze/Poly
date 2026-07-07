#pragma once

#include <string>
#include <vector>

namespace Poly
{
	struct ResolvedPort
	{
		std::string ShaderName;   // local name in the pass shader
		std::string ResolvedName; // globally resolved name (e.g. "$Color", "SceneAlbedo")
		bool        IsWrite;      // true = output, false = input
	};

	struct ResolvedPass
	{
		std::string               Name;
		std::vector<ResolvedPort> Ports;
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
