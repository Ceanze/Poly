#pragma once

namespace Poly
{
	struct Event {};


	struct CloseWindowEvent : public Event
	{
		CloseWindowEvent() {};
	};

}