#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	/**
	 * Reflection for render passes informs the render graph about inputs and outputs
	 * that the render pass is going to use.
	 */

	class Sampler;

	enum class FIOType
	{
		UNKNOWN			= 0,
		INPUT			= FLAG(1),
		OUTPUT			= FLAG(2)
	};
	ENABLE_BITMASK_OPERATORS(FIOType);

	struct IOData
	{
		std::string			Name			= "";
		FIOType				IOType			= FIOType::UNKNOWN;
		EFormat				Format			= EFormat::UNDEFINED;
		FResourceBindPoint	BindPoint		= FResourceBindPoint::NONE;
		ETextureLayout		TextureLayout	= ETextureLayout::UNDEFINED;
		union { uint32 Width; uint32 Size; };
		uint32				Height			= 0;
		uint32				Set				= 0;
		uint32				Binding			= 0;
		Ref<Sampler>		pSampler		= nullptr;

		bool operator== (const IOData& other) const { return Name == other.Name; }

		void Merge(const IOData& other)
		{
			if (Format != other.Format)
			{
				POLY_CORE_WARN("Cannot merge IOData of {} with {}, different formats!", Name, other.Name);
				return;
			}
			if (Height != other.Height || Width != other.Width)
			{
				POLY_CORE_WARN("Cannot merge IOData of {} with {}, height  and/or width  does not match!", Name, other.Name);
				return;
			}
			BindPoint |= other.BindPoint;
		}
	};

	class PassReflection
	{
	public:
		PassReflection() = default;
		~PassReflection() = default;

		/**
		 * Adds an input to the render pass
		 * @param name - name of resource, only resource name is required - not render pass name
		 * @param set - the set of the resource to use in the shader
		 * @param binding - the binding of the set to use in the shader
		 */
		void AddInput(const std::string& name, uint32 set, uint32 binding);

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
		 * Adds an internal input used for internal types of the render graph, e.g. scene instance and vertex
		 * @param name - name of binding - only provides debugging
		 * @param set - the set of the resource to be used in the shader
		 * @param binding - the binding of the resource to be used in the shader
		 * @param bindPoing - the type of input to bind, only a few selected are allowed for internal input, these are:
		 * 						FResourceBindPoint::SCENE_VERTEX and FResourceBindPoint::SCENE_INSTANCE
		 */
		void AddInternalInput(const std::string& name, uint32 set, uint32 binding, FResourceBindPoint bindPoint);

		/**
		 * Sets the format of the resource, only necessary when creating a new output in a pass and it's a texture
		 * @param name - name of the resource, only resource name is required - not render pass name
		 * @param format - format of the texture
		 */
		void SetFormat(const std::string& name, EFormat format);

		/**
		 * Sets the size of the resource, only necessary when creating a new output in a pass and it's a buffer
		 * @param name - name of the resource, only resource name is required - not render pass name
		 * @param size - size of buffer
		 */
		void SetBufferSize(const std::string& name, uint32 size);

		/**
		 * Sets the width and height of the resource, only necessary when creating a new output in a pass and it's a texture
		 * @param name - name of the resource, only resource name is required - not render pass name
		 * @param width - width of texture
		 * @param height - height of texture
		 */
		void SetTextureSize(const std::string& name, uint32 width, uint32 height);

		/**
		 * Sets the bind point of the resource, necessary for internal resource creation and syncronization
		 * @param name - name of the resource, only resource name is required - not render pass name
		 * @param bindPoint - binding point of the resource
		 */
		void SetBindPoint(const std::string& name, FResourceBindPoint bindPoint);

		/**
		 * Sets the sampler to be used for a given resource - resource must be a texture resource
		 * @param name - name of the resource - only resource name is required - not render pass name
		 * @param pSampler - sampler to be set
		 */
		void SetSampler(const std::string& name, Ref<Sampler> pSampler);

		std::vector<IOData> GetIOData(FIOType IOType, FResourceBindPoint excludeFlags) const;
		std::vector<IOData> GetAllIOs() const { return m_IOs; }
		const IOData& GetIOData(const std::string& resName) const;

	private:
		void AddIO(IOData io);

		std::vector<IOData> m_IOs;
	};
}