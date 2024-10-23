#include "Application.h"

#include <spdlog/spdlog.h>

Application::Application()
{
	spdlog::set_level(spdlog::level::debug); // Set global log level to debug
	spdlog::set_pattern("(%T) [%^%l%$]: %v");
}

Application::~Application()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Application::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

void Application::mainLoop()
{
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
	}
}

void Application::initVulkan()
{
	m_device.createInstance();
	m_device.printAvailableExtensions();
}

void Application::initWindow()
{
	const uint32_t width  = 800;
	const uint32_t height = 600;

	glfwInit();
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
}

void Application::cleanup()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}
