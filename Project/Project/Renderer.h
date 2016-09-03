#pragma once
#define BUILD_ENABLE_VULKABN_DEBUG
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
//	PFN_vkDebugReportCallbackEXT VulkanDebugCallback;

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObj, size_t location, int32_t msgCode, const char * layerPrefix, const char * msg, void * userData);
	VkDebugReportCallbackEXT vulkanDebugReportCallbackHandle = VK_NULL_HANDLE;
	void				InitDebug();
	void				DestroyDebug();

	//instance layers
	vector<char*>		layerNames;
	bool				GetInstanceLayers();

	//instance extensions
	vector<char*>		globalExtensionNames;
	bool				GetInstanceExtensions(const char * layername, vector<char*> &extensionNames);
	bool				GetInstanceExtensions()
	{
		return GetInstanceExtensions(nullptr, globalExtensionNames);
	}

	VkInstance			instance{ VK_NULL_HANDLE };	//TODO Replace all VK handles with VK_NULL_HANDLES
	bool				CreateInstance();

	//devices
	vector<VkPhysicalDevice>	gpus;
	VkPhysicalDevice	defaultPhysicalDevice { VK_NULL_HANDLE };
	uint32_t			defaultQueueFamilyIndex;	
	bool				EnumerateDevices();		//SIDE EFFECT: creates surface, and selects default physical device
	
	VkDevice			defaultDevice { VK_NULL_HANDLE };
	bool				CreateDevice();
	VkQueue				primaryQueue{ VK_NULL_HANDLE };

	HINSTANCE			winAppInstance;
	HWND				wnd;
	bool CreateAppWindow();

	VkSurfaceKHR		surface { VK_NULL_HANDLE };
	VkFormat			currentFormat;
	bool CreateSurface(VkPhysicalDevice& device);

	uint32_t			defaultPresentIdx;	//RENAME
	VkSwapchainKHR		swapchain{ VK_NULL_HANDLE };
	bool CreateSwapchain();

	vector<VkImage>		swapchainImages;
	vector<VkImageView> imageViews;
	bool				CreateImageView();

	VkCommandPool		commandPool { VK_NULL_HANDLE };
	const uint32_t		bufferCount { 1 };
	VkCommandBuffer		commandBuffer { VK_NULL_HANDLE };
	bool CreateCommandBuffers();

	bool RecreateSwapChainAndBuffers();

	VkRenderPass renderPass;
	bool CreateRenderPass();

	VkFramebuffer frameBuffer;
	bool CreateFrameBuffer();

	VkPipeline pipeline;
	bool CreatePipeline();
	bool CreateShader();

	bool CreateTri();

	bool RenderClearScreen();
	bool RenderWithRenderPass();

public:
	Renderer();
	~Renderer();

	bool Init(HINSTANCE hInstance);

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

