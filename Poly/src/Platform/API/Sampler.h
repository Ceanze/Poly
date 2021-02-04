#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	struct SamplerDesc
	{
		EFilter				MinFilter		= EFilter::NONE;
		EFilter				MagFilter		= EFilter::NONE;
		ESamplerAddressMode AddressModeU	= ESamplerAddressMode::NONE;
		ESamplerAddressMode AddressModeV	= ESamplerAddressMode::NONE;
		ESamplerAddressMode AddressModeW	= ESamplerAddressMode::NONE;
		ESamplerMipmapMode	MipMapMode		= ESamplerMipmapMode::NONE;
		EBorderColor		BorderColor		= EBorderColor::NONE;
		float				MipLodBias		= 0.0f;
		float				MinLod			= 0.0f;
		float				MaxLod			= 0.0f;
		bool				AnistropyEnable	= false;
		float				MaxAnisotropy	= 16.0f;
	};

	class Sampler
	{
		friend class RenderAPI;

	public:
		CLASS_ABSTRACT(Sampler);

		/**
		 * Init the Sampler object
		 * @param desc	Sampler creation description
		*/
		virtual void Init(const SamplerDesc* pDesc) = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

		/**
		 * @return const TextureDesc
		 */
		inline const SamplerDesc& GetDesc() const
		{
			return p_SamplerDesc;
		}

		inline static Sampler* GetDefaultLinearSampler() { return s_pLinearSampler.get(); }
		inline static Sampler* GetDefaultNearestSampler() { return s_pNearestSampler.get(); }

	protected:
		SamplerDesc p_SamplerDesc;

	private:
		static void InitDefaults();
		static void CleanupDefaults();

		inline static Ref<Sampler> s_pLinearSampler;
		inline static Ref<Sampler> s_pNearestSampler;
	};
}