file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
#include_directories(thirdparty/vma/include)
add_executable(universe ${SOURCE_FILES})

target_include_directories(universe 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    SYSTEM PRIVATE thirdparty/vma/include
)


# libs
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)

# link Vulkan and GLFW to the executable
target_link_libraries(universe 
                      Vulkan::Vulkan 
                      glfw
                      spdlog::spdlog
                      GPUOpen::VulkanMemoryAllocator)
