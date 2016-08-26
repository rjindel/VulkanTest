#pragma once

#include "vulkan.h"

class Renderer
{
private:
	VkInstance instance;

protected:

	bool CreateInstance();
public:
	Renderer();
	~Renderer();

	bool Init();
};

