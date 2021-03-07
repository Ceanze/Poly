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
		struct SyncData
		{
			ETextureLayout		srcLayout		= ETextureLayout::UNDEFINED;
			ETextureLayout		dstLayout		= ETextureLayout::UNDEFINED;
			FResourceBindPoint	srcBindPoint	= FResourceBindPoint::NONE;
			FResourceBindPoint	dstBindPoint	= FResourceBindPoint::NONE;
		};

	public:
		SyncPass() = default;
		~SyncPass() = default;

		/**
		 * Execute the SyncPass
		 */
		virtual void Execute(/* Render Context, Render Data */) override final;

		/**
		 * OPTIONAL
		 * Compile or recompile the SyncPass
		 */
		virtual void Compile() override final {};

		void AddSyncData(SyncData syncData);

		static Ref<SyncPass> Create();

	private:
		std::vector<SyncData> m_SyncData;
	};
}