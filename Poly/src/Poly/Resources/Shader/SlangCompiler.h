#pragma once

namespace slang
{
	struct IGlobalSession;
}

namespace Poly
{
	class SlangCompiler
	{
	public:
		CLASS_REMOVE_COPY(SlangCompiler);
		CLASS_REMOVE_MOVE(SlangCompiler);

		SlangCompiler();
		~SlangCompiler();

	private:
		slang::IGlobalSession* m_pGlobalSession = nullptr;
	};
} // namespace Poly
