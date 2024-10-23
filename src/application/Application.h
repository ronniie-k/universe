#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vulkan/Device.h"

class Application
{
public:
	Application();
	~Application();
	void run();

private:
	void initVulkan();
	void initWindow();
	void mainLoop();
	void cleanup();

private:
	GLFWwindow* m_window;

	Device m_device;
};