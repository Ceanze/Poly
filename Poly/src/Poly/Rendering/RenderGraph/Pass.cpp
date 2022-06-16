#include "Pass.h"

namespace Poly
{
	void Pass::SetAutoBind(uint32 setIndex, bool autoBind)
	{
		auto it = std::find(p_AutoBindedSets.begin(), p_AutoBindedSets.end(), setIndex);
		auto index = std::distance(p_AutoBindedSets.begin(), it);

		if (it != p_AutoBindedSets.end() && !autoBind)
		{
			p_AutoBindedSets[index] = p_AutoBindedSets.back();
			p_AutoBindedSets.pop_back();
		}
		else if (it == p_AutoBindedSets.end() && autoBind)
		{
			p_AutoBindedSets.push_back(setIndex);
		}
	}

	bool Pass::IsAutoBindEnabled(uint32 setIndex) const
	{
		auto it = std::find(p_AutoBindedSets.begin(), p_AutoBindedSets.end(), setIndex);
		return it != p_AutoBindedSets.end();
	}

}