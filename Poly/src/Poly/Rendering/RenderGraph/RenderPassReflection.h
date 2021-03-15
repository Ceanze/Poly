#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	/**
	 * Reflection for render passes informs the render graph about inputs and outputs
	 * that the render pass is going to use.
	 */

	enum class EIOType
	{
		UNKNOWN			= 0,
		INPUT			= 1,
		OUTPUT			= 2,
		PASS_THROUGH	= 3
	};

	struct IOData
	{
		std::string			Name			= "";
		EIOType				IOType			= EIOType::UNKNOWN;
		EFormat				Format			= EFormat::UNDEFINED;
		FResourceBindPoint	BindPoint		= FResourceBindPoint::NONE;
		ETextureLayout		TextureLayout	= ETextureLayout::UNDEFINED;
		union { uint32 Width; uint32 Size; };
		uint32				Height			= 0;

		bool operator== (const IOData& other) const { return Name == other.Name; }
	};

	class RenderPassReflection
	{
	public:
		RenderPassReflection() = default;
		~RenderPassReflection() = default;

		/**
		 * Adds an input to the render pass
		 * @param name - name of resource, only resource name is required - not render pass name
		 */
		void AddInput(const std::string& name);

		/**
		 * Adds an output to the render pass
		 * @param name - name of resource, only resource name is required - not render pass name
		 */
		void AddOutput(const std::string& name);

		/**
		 * Adds an input-output to the render pass. This tells the render graph
		 * that the resource will be changed in the pass
		 * @param name - name of resource, only resource name is required - not render pass name
		 */
		void AddPassThrough(const std::string& name);

		/**
		 * Sets the format of the resource, only necessary when creating a new output in a pass and it's a texture
		 * @param name - name of the resource, only resource name is required - not render pass name
		 * @param format - format of the texture
		 */
		void SetFormat(const std::string& name, EFormat format);

		/**
		 * Sets the bind point of the resource, necessary for internal resource creation and syncronization
		 * @param name - name of the resource, only resource name is required - not render pass name
		 * @param bindPoint - binding point of the resource
		 */
		void SetBindPoint(const std::string& name, FResourceBindPoint bindPoint);

		std::vector<IOData> GetInputs() const;
		std::vector<IOData> GetOutputs() const;
		std::vector<IOData> GetPassThroughs() const;
		std::vector<IOData> GetAllIOs() const { return m_IOs; }
		const IOData& GetIOData(const std::string& resName) const;

	private:
		void AddIO(IOData io);

		std::vector<IOData> m_IOs;
	};
}