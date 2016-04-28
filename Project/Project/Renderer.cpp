#include "Renderer.h"

#include "vulkan.h"

#include "memory"

Renderer::Renderer()
{

}


Renderer::~Renderer()
{
}

bool Renderer::CreateInstance()
{
	uint32_t LayerPropertiesCount;
	
	auto err = vkEnumerateInstanceLayerProperties(&LayerPropertiesCount, nullptr);

	if (err != VK_SUCCESS)
	{
		//TODO output an error 
		return false;
	}

	if (LayerPropertiesCount == 0)
	{
		return true;
	}

	std::unique_ptr<VkLayerProperties[]> LayerPropertiesArray(new VkLayerProperties[LayerPropertiesCount]);

	err = vkEnumerateInstanceLayerProperties(&LayerPropertiesCount, LayerPropertiesArray.get());
	if (err != VK_SUCCESS)
	{
		//TODO output debug error
		return false;
	}

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

	VkApplicationInfo appInfo{};

	VkInstanceCreateInfo createInfo{};
	createInfo. sType = VK_STRUCTURE_TYPE_APPLICATION_INFO ;

	VkInstance *instance;

	err = vkCreateInstance(&createInfo, nullptr, instance);
	if (err != VK_SUCCESS)
	{
		//TODO output an error 
		return false;
	}


	return true;
}

bool Renderer::Init()
{
	return CreateInstance();
}