#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
//#include "vulkan\vk_cpp.hpp"
#include "vulkan\vulkan.h"
#include <vector>
using namespace std;

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

	VkInstance			instance{ VK_NULL_HANDLE };	//TODO Replace all VK handles with VK_NULL_HANDLES
	bool				CreateInstance();

	//devices
	uint32_t			deviceCount{};

	VkPhysicalDevice	defaultPhysicalDevice { VK_NULL_HANDLE };
	VkPhysicalDevice*	gpus { VK_NULL_HANDLE };
	uint32_t			defaultQueueFamilyIndex;	
	bool				EnumerateDevices();		//SIDE EFFECT: creates surface, and selects default physical device
	
	//VkDevice*			devices{ VK_NULL_HANDLE };
	VkDevice			defaultDevice { VK_NULL_HANDLE };
	bool				CreateDevice();

	HINSTANCE			winAppInstance;
	HWND				wnd;
	bool CreateAppWindow();

	VkSurfaceKHR		surface { VK_NULL_HANDLE };
	VkFormat			currentFormat;
	bool CreateSurface(VkPhysicalDevice& device);

	uint32_t			defaultPresentIdx;	//RENAME
	VkSwapchainKHR		swapchain{ VK_NULL_HANDLE };
	bool CreateSwapchain();

	VkCommandPool		commandPool { VK_NULL_HANDLE };
	const uint32_t		bufferCount { 1 };
	VkCommandBuffer		commandBuffer { VK_NULL_HANDLE };
	bool CreateCommandBuffers();

	bool CreateTri();

public:
	Renderer();
	~Renderer();

	bool Init(HINSTANCE hInstance);

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

