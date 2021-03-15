#pragma once
#include "Pass.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	/**
	 * Sync passes are created automatically by the RenderGraphCompiler after resources has been created.
	 * The sync passes will be added in the graph inbetween already existing passes to transition
	 * resources.
	 */

	class SyncPass : public Pass
	{
	public:
		// static const constexpr uint32 SAME_QUEUE = UINT32_MAX;

		enum class SyncType
		{
			NONE	= 0,
			TEXTURE	= 1,
			BUFFER	= 2,
			MEMORY	= 3
		};
		struct SyncData
		{
			SyncType			Type			= SyncType::NONE;
			std::string			ResourceName	= "";
			ETextureLayout		SrcLayout		= ETextureLayout::UNDEFINED;
			ETextureLayout		DstLayout		= ETextureLayout::UNDEFINED;
			FResourceBindPoint	SrcBindPoint	= FResourceBindPoint::NONE;
			FResourceBindPoint	DstBindPoint	= FResourceBindPoint::NONE;

			bool operator== (const SyncData& other)
			{
				return ResourceName == other.ResourceName;
			}
		};

	public:
		SyncPass();
		~SyncPass() = default;

		/**
		 * Execute the SyncPass
		 */
		virtual void Execute(const RenderContext& context, const RenderData& renderData) override final;

		/**
		 * OPTIONAL
		 * Compile or recompile the SyncPass
		 */
		virtual void Compile() override final {};

		void AddSyncData(SyncData syncData);

		static Ref<SyncPass> Create();

		const std::string& GetName() const { return p_Name; }

	private:
		FAccessFlag ConvertToAccessFlag(FResourceBindPoint bindPoint);

		std::vector<SyncData> m_SyncData;
	};
}