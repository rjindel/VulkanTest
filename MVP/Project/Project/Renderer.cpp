#include "Renderer.h"

#include "memory"

Renderer::Renderer()
{

}


Renderer::~Renderer()
{
}

bool Renderer::CreateInstance()
{
	//Get Layer count and layer properties
	uint32_t layerPropertiesCount;
	
	auto err = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, nullptr);

	if (err != VK_SUCCESS)
	{
		//TODO output an error 
		return false;
	}

	if (layerPropertiesCount == 0)
	{
		return true;
	}

	std::unique_ptr<VkLayerProperties[]> layerPropertiesArray(new VkLayerProperties[layerPropertiesCount]);	

	err = vkEnumerateInstanceLayerProperties(&layerPropertiesCount, layerPropertiesArray.get());
	if (err != VK_SUCCESS)
	{
		//TODO output debug error
		return false;
	}

	// Get extension count and properties
	uint32_t extensionCount;
	err = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	if (err != VK_SUCCESS)
	{
		//TODO output an error 
		return false;
	}

	std::unique_ptr<VkExtensionProperties[]> extensionsArray( new VkExtensionProperties[extensionCount] );

	err = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionsArray.get());
	if (err != VK_SUCCESS)
	{
		//TODO output an error 
		return false;
	}

	//Get layer names
	char ** layerNames = new char*[layerPropertiesCount];

	for (uint32_t i = 0; i < layerPropertiesCount; ++i)
	{
		size_t nameLength = strlen(layerPropertiesArray[i].layerName) + 1;
		layerNames[i] = new char[nameLength];
		strcpy_s(layerNames[i], nameLength, layerPropertiesArray[i].layerName);
	}


	//Get extension names
	char** extensionNames = new char*[extensionCount];

	for (uint32_t i = 0; i < extensionCount; ++i)
	{
		size_t nameLength = strlen(extensionsArray[i].extensionName) + 1;
		extensionNames[i] = new char[nameLength];
		strcpy_s(extensionNames[i], nameLength, extensionsArray[i].extensionName);
	}
	
	//Create instance

	VkApplicationInfo appInfo {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Test";
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 8); // VK_API_VERSION; is deprecated

	VkInstanceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = layerPropertiesCount;
	createInfo.ppEnabledLayerNames = layerNames;
	createInfo.enabledExtensionCount = extensionCount;
	createInfo.ppEnabledExtensionNames = extensionNames;

	err = vkCreateInstance(&createInfo, nullptr, &instance);
	if (err != VK_SUCCESS)
	{
		//TODO output an error 
		return false;
	}

	//Create device

	return true;
}

bool Renderer::Init()
{
	return CreateInstance();
}