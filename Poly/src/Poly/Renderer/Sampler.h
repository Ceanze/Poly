#pragma once

#include "Poly/Core/Core.h"

#include <stdint.h>
#include <string>

namespace Poly
{
	class Sampler
	{
	public:
		enum class Filter
		{
			NEAREST = 0,
			LINEAR = 1,
			CUBIC_IMG = 1000015000,
		};

		enum class AddressMode
		{
			REPEAT = 0,
			MIRRORED_REPEAT = 1,
			CLAMP_TO_EDGE = 2,
			CLAMP_TO_BORDER = 3,
			MIRROR_CLAMP_TO_EDGE = 4
		};

	public:
		virtual ~Sampler() = default;

		static Ref<Sampler> create();
		static Ref<Sampler> create(Filter filter, AddressMode addressMode, Filter mipmap);
	};
}