#include <vulkan/vulkan.hpp>
#include <vector>

class Device
{
public:
	Device();
	~Device();

	vk::Instance& getInstance() { return m_instance; }

	void createInstance();
	void printAvailableExtensions();

private:
	bool hasValidationLayerSupport();

private:
	vk::Instance m_instance;

#ifdef DEBUG
	bool m_enableValidationLayers = true;
#else
	bool m_enableValidationLayers = false;
#endif
	const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
};