#include "PolyID.h"

#include <random>

namespace Poly
{
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	PolyID::PolyID() : m_Id(s_UniformDistribution(s_Engine)) {}
}