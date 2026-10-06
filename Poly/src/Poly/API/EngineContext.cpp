#include "EngineContext.h"

namespace Poly::API
{
	EngineContext::EngineContext(std::string_view worldName)
	    : m_World(worldName)
	    , m_History(*this)
	    , m_Worlds(*this)
	    , m_Entities(*this)
	    , m_Components(*this)
	{}

	void EngineContext::Tick()
	{
		m_Events.Flush();
		m_World.Update();
	}
} // namespace Poly::API
