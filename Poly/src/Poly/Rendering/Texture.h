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

		virtual void setData(void* data, uint32_t size) = 0;

		virtual uint32_t getHeight() const = 0;
		virtual uint32_t getWidth() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> create(uint32_t width, uint32_t height);
		static Ref<Texture2D> create(std::string path);
	};
}