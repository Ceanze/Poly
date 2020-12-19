#pragma once

#include "Core.h"

/*
*	TODO: Convert PVKInstance to not be a static class for it to have an interface in which it inherits from.
*			This will allow the instance to have create functions to API specific objects (like GraphicsDeviceVK).
*		For future reference, see this psudo code:

		RenderAPI::GetGraphicsInstance()->CreateBuffer(desc);
		RenderAPI::CreateBuffer(desc);
		RenderAPI holds a pointer to the current instance which is created with the correct API beforehand.
		RenderAPIs function GetInstance returns this pointer, from which the user can call the API agnostic
		functions such as CreateBuffer(desc) which will create API specific objects.
*/

namespace Poly
{
	class RenderAPI
	{
	public:
		CLASS_STATIC(RenderAPI);

		static void Init();
		static void Release();


	};
}