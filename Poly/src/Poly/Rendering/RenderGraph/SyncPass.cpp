#include "polypch.h"
#include "SyncPass.h"

namespace Poly
{
	void SyncPass::Execute(/* Render Context, Render Data */)
	{

	}

	void SyncPass::AddSyncData(SyncData syncData)
	{
		m_SyncData.push_back(syncData);
	}

	Ref<SyncPass> SyncPass::Create()
	{
		return CreateRef<SyncPass>();
	}
}