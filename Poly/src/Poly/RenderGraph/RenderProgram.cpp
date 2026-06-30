#include "RenderProgram.h"

namespace Poly
{
	RenderProgram::RenderProgram(std::vector<ResolvedPass> sortedPasses)
	    : m_Passes(std::move(sortedPasses))
	{}
} // namespace Poly
