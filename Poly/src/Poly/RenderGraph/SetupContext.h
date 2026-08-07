#pragma once

#include "Pass/PassDeclaration.h"

namespace Poly
{
	// Passed to PassDeclaration::WithSetupFn() callbacks. Static declarations
	// (RegisterPass("x").WithShader(...).MapResource(...)) already cover the normal case without
	// needing this at all - it exists as an escape hatch for programmatic/conditional setup, so it
	// just exposes the same builder already used everywhere else instead of duplicating its API.
	class SetupContext
	{
	public:
		explicit SetupContext(PassDeclaration& pass)
		    : m_Pass(pass)
		{}

		PassDeclaration* operator->() const { return &m_Pass; }
		PassDeclaration& GetPass() const { return m_Pass; }

	private:
		PassDeclaration& m_Pass;
	};
} // namespace Poly
