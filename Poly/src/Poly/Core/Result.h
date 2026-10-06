#pragma once

#include <expected>
#include <string>

namespace Poly
{
	enum class ErrorCode : uint16
	{
		None,
		EntityNotFound,
		ComponentNotFound,
		ComponentAlreadyPresent,
		ComponentNotPresent,
		ComponentNotRemovable,
		ComponentNotAddable,
		FieldNotFound,
		InvalidValue,
		InvalidParent,
		IOError,
		NothingToUndo,
		NothingToRedo,
	};

	struct Error
	{
		ErrorCode   Code = ErrorCode::None;
		std::string Message;
	};

	/**
	 * Result of an operation that can fail, either the value or an Error describing why it failed
	 * @code
	 * Result<PolyID> result = context.Entities().Create();
	 * if (!result)
	 *     POLY_WARN("{}", result.error().Message);
	 * @endcode
	 */
	template<typename T = void>
	using Result = std::expected<T, Error>;

	inline std::unexpected<Error> MakeError(ErrorCode code, std::string message)
	{
		return std::unexpected(Error{code, std::move(message)});
	}
} // namespace Poly
