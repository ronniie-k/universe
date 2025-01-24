#include "Application.h"

#include <spdlog/spdlog.h>

#include "vulkan/Window.h"

Application::Application()
{
	spdlog::set_level(spdlog::level::trace);
	spdlog::set_pattern("(%T) [%^%l%$]: %v");
}

Application::~Application()
{
	cleanup();
}

void Application::run()
{
	m_window.create();
	m_renderer.initVulkan(&m_window);
	m_renderer.createPipeline("res/shaders/output/vert.spv", "res/shaders/output/frag.spv");
	mainLoop();
}

void Application::mainLoop()
{
	auto* window = m_window.getGLFWWindow();
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		m_renderer.drawFrame();
	}
	m_renderer.waitIdle();
}

void Application::cleanup()
{
	m_window.destroy();
}
