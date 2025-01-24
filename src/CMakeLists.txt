file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(universe ${SOURCE_FILES})

# libs
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(spdlog REQUIRED)

# link Vulkan and GLFW to the executable
target_link_libraries(universe 
                      Vulkan::Vulkan 
                      glfw
                      spdlog::spdlog)
