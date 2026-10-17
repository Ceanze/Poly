#include "ExecutorRegistry.h"

#include <algorithm>
#include <cctype>
#include <iterator>

namespace
{
	bool IsEmptyParams(std::string_view paramsJson)
	{
		std::string compact;
		std::ranges::copy_if(paramsJson, std::back_inserter(compact), [](char c) { return !std::isspace(static_cast<unsigned char>(c)); });
		return compact.empty() || compact == "{}";
	}
} // namespace

namespace Poly
{
	void ExecutorRegistry::Register(std::string_view name, std::string_view description, ExecuteFn executeFn)
	{
		RegisterFactory(name, description, [executeFn = std::move(executeFn), name = std::string(name)](std::string_view paramsJson) -> Result<ExecuteFn> {
			if (!IsEmptyParams(paramsJson))
				return MakeError(ErrorCode::InvalidValue, fmt::format("Executor '{}' takes no parameters, got {}", name, paramsJson));

			return executeFn;
		});
	}

	void ExecutorRegistry::RegisterFactory(std::string_view name, std::string_view description, ExecutorFactory factory)
	{
		auto [it, inserted] = m_Executors.insert_or_assign(std::string(name), ExecutorDesc{std::string(name), std::string(description), std::move(factory)});
		if (!inserted)
			POLY_CORE_WARN("Executor '{}' was already registered, it is replaced", name);
	}

	const ExecutorDesc* ExecutorRegistry::Get(std::string_view name) const
	{
		auto it = m_Executors.find(name);
		return it != m_Executors.end() ? &it->second : nullptr;
	}

	Result<ExecuteFn> ExecutorRegistry::Create(std::string_view name, std::string_view paramsJson) const
	{
		const ExecutorDesc* pExecutor = Get(name);
		if (!pExecutor)
			return MakeError(ErrorCode::ExecutorNotFound, fmt::format("No executor named '{}' is registered", name));

		return pExecutor->Factory(paramsJson);
	}
} // namespace Poly
