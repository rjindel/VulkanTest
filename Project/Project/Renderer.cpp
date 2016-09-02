#include "Renderer.h"
#include "Debug.h"

#include <memory>
#include <assert.h>

Renderer::Renderer()
{

}

Renderer::~Renderer()
{
	//TD move to cleanup function
	vkDeviceWaitIdle(defaultDevice);
	//vkDestroySemaphore(defaultDevice, semaphore, nullptr);

	VkCommandBuffer commandBuffers[1] = { commandBuffer };
	vkFreeCommandBuffers(defaultDevice, commandPool, bufferCount, commandBuffers);
	vkDestroyCommandPool(defaultDevice, commandPool, nullptr);
	vkDestroySwapchainKHR(defaultDevice, swapchain, nullptr);
	vkDestroyDevice(defaultDevice, nullptr);
	vkDestroySurfaceKHR(instance, surface, VK_NULL_HANDLE);
	DestroyDebug();
	vkDestroyInstance(instance, nullptr);
	if (globalExtensionNames.size() > 0)
	{
		for (auto name : globalExtensionNames)
		{
			delete[] name;
		}
		globalExtensionNames.clear();
	}
	if (layerNames.size() > 0)
	{
		for(auto layername : layerNames)
		{ 
			delete[] layername;
		}
		layerNames.clear();
	}
}

bool Renderer::GetInstanceLayers()
{
	//Get Layer count and layer properties
	uint32_t	layerPropertiesCount = 0;
	auto err = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, nullptr);

	if (err != VK_SUCCESS)
	{
		Debug::Log("Enumerate global instance layers failed", DebugLevel::Error);
		return false;
	}

	Debug::Log(std::string("Enumerating global instance layers: ") + ToString(layerPropertiesCount) + " found");
	if (layerPropertiesCount == 0)
	{
		return true;
	}
	std::unique_ptr<VkLayerProperties[]> layerPropertiesArray(new VkLayerProperties[layerPropertiesCount]);

	err = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, layerPropertiesArray.get());
	if (err != VK_SUCCESS)
	{
		Debug::Log("Enumerate instance layers failed while retrieving layers", DebugLevel::Error);
		return false;
	}

	//Get layer names
	layerNames.resize(layerPropertiesCount);

	for (uint32_t i = 0; i < layerNames.size(); ++i)
	{
		Debug::Log(std::string("Instance layer ") + ToString(i) + " : " + layerPropertiesArray[i].layerName + " Desc: " + layerPropertiesArray[i].description);
		Debug::Log(std::string("Require Api version: ") + ToString(VK_VERSION_MAJOR(layerPropertiesArray[i].specVersion)) + '.' + ToString(VK_VERSION_MINOR(layerPropertiesArray[i].specVersion)) + '.' + ToString(VK_VERSION_PATCH(layerPropertiesArray[i].specVersion)) );
		size_t nameLength = strlen(layerPropertiesArray[i].layerName) + 1;
		layerNames[i] = new char[nameLength];
		strcpy_s(layerNames[i], nameLength, layerPropertiesArray[i].layerName);
	}

	return true;
}

bool Renderer::GetInstanceExtensions(const char * layername, vector<char*> &extensionNames)
{
	if (layername)
	{
		Debug::Log(std::string("Enumerate ") + layername + " instance extensions");
	}
	else
	{
		Debug::Log("Enumerate global instance extensions");
	}
	// Get extension count and properties
	uint32_t			extensionCount {};
	auto err = vkEnumerateInstanceExtensionProperties(layername, &extensionCount, nullptr);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Enumerate instance extensions failed", DebugLevel::Error);
		return false;
	}
	Debug::Log(std::string("Enumerating instance extensions: ") + ToString(extensionCount) + " found");

	std::unique_ptr<VkExtensionProperties[]> extensionsArray(new VkExtensionProperties[extensionCount]);

	err = vkEnumerateInstanceExtensionProperties(layername, &extensionCount, extensionsArray.get());
	if (err != VK_SUCCESS)
	{
		Debug::Log("Enumerate instance extensions failed while retrieving extensions", DebugLevel::Error);
		return false;
	}

	//Get extension names
	extensionNames.resize(extensionCount);

	for (uint32_t i = 0; i < extensionCount; ++i)
	{
		Debug::Log(std::string("Instance extension ") + ToString(i) + " : " + extensionsArray[i].extensionName);
		size_t nameLength = strlen(extensionsArray[i].extensionName) + 1;
		extensionNames[i] = new char[nameLength];
		strcpy_s(extensionNames[i], nameLength, extensionsArray[i].extensionName);
	}

	return true;
}

bool Renderer::CreateInstance()
{
	//Create instance
	VkApplicationInfo appInfo {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Rkj Vulkan Sample app";
	appInfo.pEngineName = "Rkj Vulkan Sample app";
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 8); // VK_API_VERSION; is deprecated

	VkInstanceCreateInfo instanceCreateInfo {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledLayerCount = layerNames.size() - 1;
	instanceCreateInfo.ppEnabledLayerNames = layerNames.data();
	instanceCreateInfo.enabledExtensionCount = globalExtensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = globalExtensionNames.data();

	//Debug create instance
	VkDebugReportCallbackCreateInfoEXT callbackCreatInfo{};
	callbackCreatInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackCreatInfo.pNext = nullptr;
	callbackCreatInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	callbackCreatInfo.pfnCallback = &VulkanDebugCallback;

	instanceCreateInfo.pNext = &callbackCreatInfo;

	auto err = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Create Vulkan instance failed");
		return false;
	}

	return true;
}

bool Renderer::CreateSurface(VkPhysicalDevice& device)
{
	VkWin32SurfaceCreateInfoKHR surfaceInfo{};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.hinstance = winAppInstance;
	surfaceInfo.hwnd = wnd;
	surfaceInfo.pNext = nullptr;

	auto err = vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Create Win32 surface", DebugLevel::Error);
		return false;
	}
	//Get surface format
	uint32_t formatCount;
	err = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (err != VK_SUCCESS)
	{
		Debug::Log("Get surface count", DebugLevel::Error);
		return false;
	}

	vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	err = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, surfaceFormats.data());

	if (err != VK_SUCCESS)
	{
		Debug::Log("Get surface formats", DebugLevel::Error);
		return false;
	}

	// TODO: Define how to choose ideal surface format
	if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		currentFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		currentFormat = surfaceFormats[0].format;
	}

	return true;
}

bool Renderer::EnumerateDevices()
{
	assert(instance && "VkInstance not initialised");	//TODO: improve error handling

	uint32_t deviceCount = 0;
	auto err = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (err != VK_SUCCESS)
	{
		Debug::Log("Enumerate physical devices failed", DebugLevel::Error);
		return false;
	}

	Debug::Log(std::string("Enumerating physical devices: ") + ToString(deviceCount) + " found");

	gpus.resize(deviceCount);

	err = vkEnumeratePhysicalDevices(instance, &deviceCount, gpus.data());

	if (err != VK_SUCCESS)
	{
		Debug::Log("Enumerate physical devices: Failed to retrieve devices", DebugLevel::Error);
		return false;
	}

	Debug::Log("Searching for first graphics capable queue");
	for (uint32_t i = 0; i < deviceCount; ++i)
	{
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(gpus[i], &properties);
		Debug::Log(std::string("Device Name ") + properties.deviceName);

		if (!CreateSurface(gpus[i]))
		{
			continue;
		}

		//Get queue data
		uint32_t queueCount;
		vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queueCount, nullptr);

		//Get Present mode?
		vector<VkQueueFamilyProperties> queueProperties(queueCount);

		//Search for queue that supports the graphics bit
		vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queueCount, queueProperties.data());
		vector<VkBool32> supportsPresent (queueCount);

		for (uint32_t j = 0; j < queueCount; ++j)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(gpus[i], j, surface, &supportsPresent[j]);
			if (queueProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT && supportsPresent[j])
			{
				Debug::Log(std::string("Using device: ") + properties.deviceName + "Which has " + ToString(queueCount) + " queues");
				defaultQueueFamilyIndex = j;
				defaultPhysicalDevice = gpus[i];
				break;
			}
		}

		if (defaultPhysicalDevice == VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(instance, surface, VK_NULL_HANDLE);
		}
	}

	if (defaultPhysicalDevice == VK_NULL_HANDLE)
	{
		Debug::Log("No graphics capable device found", DebugLevel::Error);
		return false;
	}

	return true;
}

bool Renderer::CreateDevice()
{
	//Create device
	float queue_priorities[1] = { 0.0f };
	VkDeviceQueueCreateInfo queue = {};
	queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue.queueFamilyIndex = defaultQueueFamilyIndex;
	queue.queueCount = 1;
	queue.pQueuePriorities = queue_priorities;

	VkDeviceCreateInfo deviceCreateInfo = {};

	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = &queue;
	//deviceCreateInfo.enabledExtensionCount = globalExtensionCount;		//TODO: get this programatically
	//deviceCreateInfo.ppEnabledExtensionNames = globalExtensionNames;

	deviceCreateInfo.enabledExtensionCount = 1;		//TODO: get this programatically
	char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

	Debug::Log("Creating default logical device");

	auto err = vkCreateDevice(defaultPhysicalDevice, &deviceCreateInfo, nullptr, &defaultDevice);
	
	if (err != VK_SUCCESS)
	{
		Debug::Log("Create Device failed", DebugLevel::Error);
		return false;
	}

	vkGetDeviceQueue(defaultDevice, defaultQueueFamilyIndex, 0, &primaryQueue);

	return true;
}

bool Renderer::CreateAppWindow()
{
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = winAppInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"Classname";

	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	wnd = CreateWindowEx(0, wc.lpszClassName, L"Appname", WS_VISIBLE, 0, 0, 640, 480, nullptr, nullptr, wc.hInstance, nullptr);

	if (wnd == nullptr)
	{
		return false;
	}
	

	return true;
}

bool Renderer::CreateCommandBuffers()
{
	VkCommandPoolCreateInfo cmd_pool_info{};
	cmd_pool_info.sType		= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.queueFamilyIndex = defaultQueueFamilyIndex;
	cmd_pool_info.flags		= 0;
	cmd_pool_info.pNext		= nullptr;

	auto err = vkCreateCommandPool(defaultDevice, &cmd_pool_info, nullptr, &commandPool);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Create command pool", DebugLevel::Error);
		return false;
	}

	VkCommandBufferAllocateInfo bufferAllocInfo {};
	bufferAllocInfo.sType		= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferAllocInfo.commandBufferCount = bufferCount;
	bufferAllocInfo.commandPool = commandPool;
	bufferAllocInfo.level		= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	bufferAllocInfo.pNext		= nullptr;

	err = vkAllocateCommandBuffers(defaultDevice, &bufferAllocInfo, &commandBuffer);

	if (err != VK_SUCCESS)
	{
		Debug::Log("Allocate command buffer", DebugLevel::Error);
		return false;
	}

	return true;
}

bool Renderer::CreateSwapchain()
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	auto err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(defaultPhysicalDevice, surface, &surfaceCapabilities);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Get Surface capabilities", DebugLevel::Error);
		return false;
	}

	height = surfaceCapabilities.currentExtent.height;
	width = surfaceCapabilities.currentExtent.width;

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	/*
	uint32_t presentModeCount;
	err = vkGetPhysicalDeviceSurfacePresentModesKHR(defaultPhysicalDevice, surface, &presentModeCount, nullptr);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Get Surface present mode count", DebugLevel::Error);
		return false;
	}

	vector<VkPresentModeKHR> presentModes(presentModeCount);
	err = vkGetPhysicalDeviceSurfacePresentModesKHR(defaultPhysicalDevice, surface, &presentModeCount, presentModes.data());
	if (err != VK_SUCCESS)
	{
		Debug::Log("Get Surface present mode", DebugLevel::Error);
		return false;
	}
	*/

	uint32_t swapchainImageCount = 2;
	if (surfaceCapabilities.minImageCount > swapchainImageCount)
	{
		swapchainImageCount = surfaceCapabilities.minImageCount;
	}
	if (surfaceCapabilities.maxImageCount > 0 && swapchainImageCount > surfaceCapabilities.maxImageCount)
	{
		swapchainImageCount = surfaceCapabilities.maxImageCount;
	}

		
	//Create Swapchain
	VkSwapchainCreateInfoKHR swapchainCreateInfo {};
	swapchainCreateInfo.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext			= nullptr;
	swapchainCreateInfo.flags			= 0;
	swapchainCreateInfo.surface			= surface;
	swapchainCreateInfo.minImageCount	= swapchainImageCount;
	swapchainCreateInfo.imageFormat		= currentFormat;
	swapchainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageExtent		= surfaceCapabilities.currentExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;// defaultQueueFamilyIndex;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;

	swapchainCreateInfo.preTransform	= surfaceCapabilities.currentTransform; //	VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
	swapchainCreateInfo.compositeAlpha	= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode		= presentMode;
	swapchainCreateInfo.clipped			= VK_TRUE;

	err = vkCreateSwapchainKHR(defaultDevice, &swapchainCreateInfo, nullptr, &swapchain);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Create Swap Chain", DebugLevel::Error);
		return false;
	}

	return true;
}

bool Renderer::CreateImageView()
{
	uint32_t swapchainImageCount = 0;
	auto err = vkGetSwapchainImagesKHR(defaultDevice, swapchain, &swapchainImageCount, nullptr);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Get Swap chain image count");
		return false;
	}

	swapchainImages.resize(swapchainImageCount);
	err = vkGetSwapchainImagesKHR(defaultDevice, swapchain, &swapchainImageCount, swapchainImages.data());
	if (err != VK_SUCCESS)
	{
		Debug::Log("Get Swap chain images");
		return false;
	}

	imageViews.reserve(swapchainImageCount);
	for (auto swapchainImage : swapchainImages)
	{
		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.pNext = nullptr;
		imageViewCreateInfo.image = swapchainImage;
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = currentFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;  //VK_COMPONENT_SWIZZLE_R;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.flags = 0;

		VkImageView imageView;
		vkCreateImageView(defaultDevice, &imageViewCreateInfo, nullptr, &imageView);
		imageViews.push_back(imageView);
	}

	return true;
}

bool Renderer::RecreateSwapChainAndBuffers()
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	auto err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(defaultPhysicalDevice, surface, &surfaceCapabilities);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Get Surface capabilities", DebugLevel::Error);
		return false;
	}

	height = surfaceCapabilities.currentExtent.height;
	width = surfaceCapabilities.currentExtent.width;

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

	//Create Swapchain
	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = swapchainImages.size();
	swapchainCreateInfo.imageFormat = currentFormat;
	swapchainCreateInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;// defaultQueueFamilyIndex;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;

	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform; //	VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = swapchain;

	err = vkCreateSwapchainKHR(defaultDevice, &swapchainCreateInfo, nullptr, &swapchain);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Recreate Swap Chain", DebugLevel::Error);
		return false;
	}

	return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::VulkanDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t srcObj,
	 size_t location, int32_t msgCode, const char * layerPrefix, const char * msg, void * userData)
{
	DebugLevel level = DebugLevel::Informational;
	switch (flags)
	{
	case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
		level = DebugLevel::Warning;
		break;
	case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
		level = DebugLevel::Log;
		break;
	case VK_DEBUG_REPORT_WARNING_BIT_EXT:
		level = DebugLevel::Warning;
		break;
	case VK_DEBUG_REPORT_ERROR_BIT_EXT:
		level = DebugLevel::Error;
		break;
	case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
	default:
		level = DebugLevel::Verbose;
	}

	Debug::Log(msg, level, location, layerPrefix);
	return VK_FALSE;
}

void Renderer::InitDebug()
{
	VkDebugReportCallbackCreateInfoEXT callbackCreatInfo{};
	callbackCreatInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackCreatInfo.pNext = nullptr;
	callbackCreatInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	callbackCreatInfo.pfnCallback = &VulkanDebugCallback;

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =  reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));

	vkCreateDebugReportCallbackEXT(instance, &callbackCreatInfo, nullptr, &vulkanDebugReportCallbackHandle );
}

void Renderer::DestroyDebug()
{
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =  reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

	vkDestroyDebugReportCallbackEXT(instance, vulkanDebugReportCallbackHandle, nullptr);
}

bool Renderer::RenderClearScreen()
{
	uint32_t timeout = 30; // ms
	VkSemaphore semaphore;
	VkSemaphoreCreateInfo semaphoreCreatInfo{};
	semaphoreCreatInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreatInfo.pNext = nullptr;
	semaphoreCreatInfo.flags = 0;
	auto err = vkCreateSemaphore(defaultDevice, &semaphoreCreatInfo, nullptr, &semaphore);
	uint32_t imageIndex;
	
	err = vkAcquireNextImageKHR(defaultDevice, swapchain, timeout, semaphore, nullptr, &imageIndex);

	VkCommandBufferBeginInfo cmdBufferBeginInfo{};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.pNext = nullptr;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	cmdBufferBeginInfo.pInheritanceInfo = nullptr;

	VkClearColorValue clearColour = { 1.0f, 0.8f, 0.4f, 0.0f };

	VkImageSubresourceRange imageSubresourceRange{};
	imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresourceRange.baseMipLevel = 0;
	imageSubresourceRange.levelCount = 1;
	imageSubresourceRange.baseArrayLayer = 0;
	imageSubresourceRange.layerCount = 1;

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = defaultQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = defaultQueueFamilyIndex;
	barrier.image = swapchainImages[imageIndex];
	barrier.subresourceRange = imageSubresourceRange;
	
	VkImageMemoryBarrier barrier2 = barrier;
	barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier2.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	//Begin 
	err = vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	
	vkCmdClearColorImage(commandBuffer, swapchainImages[imageIndex] , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColour, 1, &imageSubresourceRange);

	//End
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier2);

	err = vkEndCommandBuffer(commandBuffer);

	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &semaphore;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &semaphore; //TODO: we need different semaphores here
	
	err = vkQueueSubmit(primaryQueue, 1, &submitInfo, nullptr);
	
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &semaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;
	
	err = vkQueuePresentKHR(primaryQueue, &presentInfo);

	return true;
}

bool Renderer::CreateRenderPass()
{
	VkAttachmentDescription attachmentDescription{};
	attachmentDescription.flags = 0;
	attachmentDescription.format;		//Get SwapChain Format
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	VkAttachmentReference colourAttachmentReference{};
	colourAttachmentReference.attachment = 0;
	colourAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription{};
	subpassDescription.flags = 0; 
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colourAttachmentReference;
	subpassDescription.pResolveAttachments = nullptr;
	subpassDescription.pDepthStencilAttachment = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = nullptr;

	auto err = vkCreateRenderPass(defaultDevice, &renderPassCreateInfo, nullptr, &renderPass);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Create Render pass");
		return false;
	}

	return true;
}

bool Renderer::CreateFrameBuffer()
{
	VkFramebufferCreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.pNext = nullptr;
	framebufferCreateInfo.flags = 0;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = imageViews.size();
	framebufferCreateInfo.pAttachments = imageViews.data();
	framebufferCreateInfo.width = width;
	framebufferCreateInfo.height = height;
	framebufferCreateInfo.layers = 1;

	auto err = vkCreateFramebuffer(defaultDevice, &framebufferCreateInfo, nullptr, &frameBuffer);
	if (err != VK_SUCCESS)
	{
		Debug::Log("Create Frame buffer");
		return false;
	}

	return true;
}

bool Renderer::CreateTri()
{
	float vertices[] = {
		0, -1, 0,
		-1, 1, 0,
		1, 1,  0
	};

	VkBufferCreateInfo vbInfo = {};
	vbInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vbInfo.pNext = nullptr;
	vbInfo.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
	vbInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vbInfo.size = sizeof(vertices);
	vbInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vbInfo.queueFamilyIndexCount = 0;
	vbInfo.pQueueFamilyIndices = &defaultQueueFamilyIndex;
	
	VkMemoryAllocateInfo vbAllocInfo = {};
	vbAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vbAllocInfo.pNext = nullptr;
	vbAllocInfo.allocationSize = 0;
	vbAllocInfo.memoryTypeIndex = 0;

	VkBuffer vertexBuffer;

	auto err = vkCreateBuffer(defaultDevice, &vbInfo, nullptr, &vertexBuffer);
	if (err != VK_SUCCESS)
	{
		//TODO output an error 
		return false;
	}

	VkMemoryRequirements memRequirements;
	
	vkGetBufferMemoryRequirements(defaultDevice, vertexBuffer, &memRequirements);


	return true;
}

LRESULT CALLBACK Renderer::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
	//	return 0;
}

bool Renderer::Init(HINSTANCE hInstance)
{
	winAppInstance = hInstance;
	//TODO Error checking
	GetInstanceLayers();
	GetInstanceExtensions();
	CreateInstance();
	InitDebug();
	
	CreateAppWindow();
	EnumerateDevices();
	CreateDevice();

	CreateSwapchain();
	CreateCommandBuffers();
	CreateImageView();

	RenderClearScreen();

	return true;
}


