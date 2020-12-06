#pragma once

#include "Poly/Core/Core.h"

#include <stdint.h>
#include <string>

namespace Poly
{
	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetWidth() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(uint32_t width, uint32_t height);
		static Ref<Texture2D> Create(std::string path);
	};
}