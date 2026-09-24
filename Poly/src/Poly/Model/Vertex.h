#pragma once

#include "Platform/API/GraphicsPipeline.h"

namespace Poly
{
	struct Vertex
	{
		alignas(16) glm::vec4 Position;
		alignas(16) glm::vec4 Normal;
		alignas(16) glm::vec4 Tangent;
		alignas(16) glm::vec2 TexCoord;

		static std::vector<VertexInput> GetInputInfo()
		{
			std::vector<VertexInput> vertexInputs;

			VertexInput input;
			// Position
			input.Binding         = 0;
			input.Format          = EFormat::R32G32B32A32_SFLOAT;
			input.Location        = 0;
			input.Offset          = 0;
			input.Stride          = sizeof(Vertex);
			input.VertexInputRate = EVertexInputRate::INSTANCE;
			vertexInputs.push_back(input);

			// Normal
			input.Location = 1;
			input.Offset   = sizeof(glm::vec4);
			vertexInputs.push_back(input);

			// Tangent
			input.Location = 2;
			input.Offset   = 2 * sizeof(glm::vec4);
			vertexInputs.push_back(input);

			// TexCoord
			input.Location = 3;
			input.Offset   = 3 * sizeof(glm::vec4);
			input.Format   = EFormat::R32G32_SFLOAT;
			vertexInputs.push_back(input);

			return vertexInputs;
		}
	};
} // namespace Poly