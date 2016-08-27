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
	VkCommandBuffer commandBuffers[1] = { commandBuffer };
	vkFreeCommandBuffers(defaultDevice, commandPool, bufferCount, commandBuffers);
	vkDestroyCommandPool(defaultDevice, commandPool, nullptr);
	vkDestroyDevice(defaultDevice, nullptr);
	vkDestroySurfaceKHR(instance, surface, VK_NULL_HANDLE);
	vkDestroyInstance(instance, nullptr);
	if (globalExtensionNames)
		delete[] globalExtensionNames;
	if (layerNames)
		delete[] layerNames;
}

bool Renderer::GetInstanceLayers()
{
	//Get Layer count and layer properties
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
	layerNames = new char*[layerPropertiesCount];

	for (uint32_t i = 0; i < layerPropertiesCount; ++i)
	{
		Debug::Log(std::string("Instance layer ") + ToString(i) + " : " + layerPropertiesArray[i].layerName + " Desc: " + layerPropertiesArray[i].description);
		Debug::Log(std::string("Require Api version: ") + ToString(VK_VERSION_MAJOR(layerPropertiesArray[i].specVersion)) + '.' + ToString(VK_VERSION_MINOR(layerPropertiesArray[i].specVersion)) + '.' + ToString(VK_VERSION_PATCH(layerPropertiesArray[i].specVersion)) );
		size_t nameLength = strlen(layerPropertiesArray[i].layerName) + 1;
		layerNames[i] = new char[nameLength];
		strcpy_s(layerNames[i], nameLength, layerPropertiesArray[i].layerName);
	}

	return true;
}

bool Renderer::GetInstanceExtensions(const char * layername, uint32_t &extensionCount, char**& extensionNames)
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
	extensionNames = new char*[extensionCount];

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
	instanceCreateInfo.enabledLayerCount = layerPropertiesCount - 1;
	instanceCreateInfo.ppEnabledLayerNames = layerNames;
	instanceCreateInfo.enabledExtensionCount = globalExtensionCount;
	instanceCreateInfo.ppEnabledExtensionNames = globalExtensionNames;

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

	auto surfaceFormats = new VkSurfaceFormatKHR[formatCount];
	err = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, surfaceFormats);

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

	auto err = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (err != VK_SUCCESS)
	{
		Debug::Log("Enumerate physical devices failed", DebugLevel::Error);
		return false;
	}

	Debug::Log(std::string("Enumerating physical devices: ") + ToString(deviceCount) + " found");

	gpus = new VkPhysicalDevice[deviceCount];

	err = vkEnumeratePhysicalDevices(instance, &deviceCount, gpus);

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
		auto queueProperties = new VkQueueFamilyProperties[queueCount];

		//Search for queue that supports the graphics bit
		vkGetPhysicalDeviceQueueFamilyProperties(gpus[i], &queueCount, queueProperties);
		VkBool32* supportsPresent = new VkBool32[queueCount];

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
		delete[] supportsPresent;
		delete[] queueProperties;
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

	//devices = new VkDevice[deviceCount];
	//defaultDevice = devices[0];

	Debug::Log("Creating default logical device");

	auto err = vkCreateDevice(defaultPhysicalDevice, &deviceCreateInfo, nullptr, &defaultDevice);
	//TODO:	Error check

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

	//Create Swapchain
	VkSwapchainCreateInfoKHR swapchainCreateInfo {};
	swapchainCreateInfo.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface			= surface;
	swapchainCreateInfo.imageFormat		= currentFormat;
	swapchainCreateInfo.minImageCount	= 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchainCreateInfo.imageExtent.height	= height;
	swapchainCreateInfo.imageExtent.width	= width;

	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.imageArrayLayers = 1;
//	swapchainCreateInfo.preTransform = preTransform;
//	swapchainCreateInfo.presentMode = swapchainPresentMode;

	auto err = vkCreateSwapchainKHR(defaultDevice, &swapchainCreateInfo, nullptr, &swapchain);
	if (err != VK_SUCCESS)
	{
		//TODO output an error 
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
	vbInfo.size = sizeof(vertices);
	vbInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	
	VkMemoryAllocateInfo vbAllocInfo = {};
	vbAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

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


bool Renderer::Init(HINSTANCE hInstance)
{
	winAppInstance = hInstance;
	//TODO Error checking
	GetInstanceLayers();
	GetInstanceExtensions();
	CreateInstance();
	
	CreateAppWindow();
	EnumerateDevices();
	CreateDevice();

	CreateSwapchain();
	CreateCommandBuffers();

	return true;
}

LRESULT CALLBACK Renderer::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
//	return 0;
}
