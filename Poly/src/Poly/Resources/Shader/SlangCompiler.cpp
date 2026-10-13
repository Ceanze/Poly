#include "SlangCompiler.h"

#include "polypch.h"

#include <slang/slang.h>

namespace Poly
{
	SlangCompiler::SlangCompiler()
	{
		const SlangResult result = slang::createGlobalSession(&m_pGlobalSession);
		POLY_VALIDATE(SLANG_SUCCEEDED(result), "Slang global session could not be created - result was {}", static_cast<int>(result));

		if (m_pGlobalSession)
			POLY_CORE_INFO("Slang initialized, version {}", m_pGlobalSession->getBuildTagString());
	}

	SlangCompiler::~SlangCompiler()
	{
		if (m_pGlobalSession)
			m_pGlobalSession->release();

		slang::shutdown();
	}
} // namespace Poly
