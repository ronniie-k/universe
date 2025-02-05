#include <cstdlib>
#include <iostream>
#include <stdexcept>

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "application/Application.h"

int main()
{
	Application app;
	try
	{
		app.run();
	}
	catch (const std::exception &e)
	{
		std::cout << "failure\n";
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

// refactor: Renderer and pipeline classes