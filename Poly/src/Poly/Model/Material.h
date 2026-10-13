#pragma once

namespace Poly
{
	struct MaterialValues
	{
		glm::vec4 Albedo     = {1.0, 1.0, 1.0, 1.0};
		float     AO         = 1.0;
		float     Metallic   = 1.0;
		float     Roughness  = 1.0;
		float     IsCombined = 0.0;
	};

	class Material
	{
	public:
		enum class Type
		{
			NONE,
			ALBEDO,
			METALIC,
			NORMAL,
			ROUGHNESS,
			AMBIENT_OCCLUSION,
			COMBINED
		};
	};
} // namespace Poly