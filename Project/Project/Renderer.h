#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
//#include "vulkan\vk_cpp.hpp"
#include "vulkan\vulkan.h"

class Renderer
{
private:
	int					width	{ 640 };
	int					height	{ 480 };

protected:
	//instance layers
	uint32_t			layerPropertiesCount{};
	char**				layerNames{ nullptr };
	bool				GetInstanceLayers();

	//instance extensions
	uint32_t			globalExtensionCount {};
	char**				globalExtensionNames {nullptr};
	bool				GetInstanceExtensions(const char * layername, uint32_t &extensionCount, char**& extensionNames);
	bool				GetInstanceExtensions()
	{
		return GetInstanceExtensions(nullptr, globalExtensionCount, globalExtensionNames);
	}

	VkInstance			instance { nullptr };
	bool				CreateInstance();

	//devices
	uint32_t			deviceCount{};
	VkPhysicalDevice*	gpus { nullptr };
	VkDevice*			devices{ nullptr };
	bool				EnumerateDevices();

	HINSTANCE			winAppInstance;
	HWND				wnd;
	bool CreateAppWindow();

	VkSurfaceKHR		surface { nullptr };
	VkFormat			currentFormat;
	VkSwapchainKHR		swapchain{ nullptr };
	bool CreateSwapchain();


	bool CreateTri();

public:
	Renderer();
	~Renderer();

	bool Init(HINSTANCE hInstance);
};

