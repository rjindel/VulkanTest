#include <Windows.h> //May not be need as it's included by vulkan. May want to define WIN32_LEAN_AND_MEAN

#include "Renderer.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstancee, LPSTR mCommandline, int commandShow)
{
	Renderer r;
	r.Init(hInstance);

}