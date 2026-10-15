#pragma once

#include "Poly/Core/Result.h"

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace Poly
{
	class ExecuteContext;

	using ExecuteFn = std::function<void(ExecuteContext&)>;

	/*
	 * Creates the execute function of one pass from the parameters the pass gives the executor.
	 * @param paramsJson The parameters as a json object, empty if the pass gives none.
	 * @return The execute function, or an error if the parameters are invalid.
	 */
	using ExecutorFactory = std::function<Result<ExecuteFn>(std::string_view paramsJson)>;

	struct ExecutorDesc
	{
		std::string     Name;
		std::string     Description;
		ExecutorFactory Factory;
	};

	/*
	 * An executor gets everything it needs through the ExecuteContext, so it should not capture state of its own.
	 *
	 * Usage:
	 *
	 *   catalog.GetExecutorRegistry().Register("FullscreenTriangle", "Draws a triangle covering the target",
	 *                                          [](ExecuteContext& ctx) { ctx.GetCommandBuffer()->Draw(3, 1, 0, 0); });
	 */
	class ExecutorRegistry
	{
	public:
		/*
		 * Registers an executor without parameters. A pass giving it parameters fails to build.
		 * Replaces an executor already registered with the same name.
		 * @param name The name passes reference the executor by.
		 * @param description What the executor does, shown to users choosing an executor.
		 * @param executeFn The function executed by every pass using the executor.
		 */
		void Register(std::string_view name, std::string_view description, ExecuteFn executeFn);

		/*
		 * Registers an executor creating the execute function of each pass from the parameters of the pass.
		 * Replaces an executor already registered with the same name.
		 * @param name The name passes reference the executor by.
		 * @param description What the executor does, shown to users choosing an executor.
		 * @param factory Creates the execute function of a pass.
		 */
		void RegisterFactory(std::string_view name, std::string_view description, ExecutorFactory factory);

		const ExecutorDesc* Get(std::string_view name) const;

		/*
		 * Creates the execute function of a pass.
		 * @param name The name of the executor.
		 * @param paramsJson The parameters the pass gives the executor, as a json object. Empty for none.
		 * @return The execute function, or an error if there is no such executor or the parameters are invalid.
		 */
		Result<ExecuteFn> Create(std::string_view name, std::string_view paramsJson) const;

		const std::unordered_map<std::string, ExecutorDesc>& GetAll() const { return m_Executors; }

	private:
		std::unordered_map<std::string, ExecutorDesc> m_Executors;
	};
} // namespace Poly
