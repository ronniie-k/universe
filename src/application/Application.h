#pragma once

#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/Window.h"
#include "renderer/Renderer.h"

class Application
{
public:
	Application();
	~Application() = default;
	void run();

private:
	void mainLoop();
	void cleanup();

private:
	Window m_window;
	Renderer m_renderer;
};