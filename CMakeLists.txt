cmake_minimum_required(VERSION 3.13)
project(V2Engine)

set(CMAKE_CXX_STANDARD 17)
set(CFLAGS -std=c++17 -O2)
#set(LDFLAGS -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi)
set(CMAKE_BUILD_TYPE Debug)

#glfw
set ( GLFW_BUILD_DOCS OFF )
set( GLFW_INSTALL OFF )
set( GLFW_BUILD_TESTS OFF )
set( GLFW_BUILD_EXAMPLES OFF )
if ( MSVC )
	set( GLFW_BUILD_WIN32 ON )
else()
	set( GLFW_BUILD_X11 ON )
	set( GLFW_BUILD_WAYLAND OFF )
endif()
#set (GLFW_VULKAN_STATIC ON)

if( MSVC )
	set(ENV{VULKAN_SDK} "C:/VulkanSDK/1.3.296.0")
	include_directories("C:/VulkanSDK/1.3.296.0/Include")	
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
endif()

add_subdirectory(libraries/glfw-3.4)
include_directories(libraries/glfw-3.4/include)
include_directories(libraries/tinyobjloader)
include_directories(libraries/stb)
include_directories(libraries/imgui)
include_directories(libraries/imgui/backends)
#other libs
find_package(Vulkan REQUIRED)
#link_libraries(Vulkan)

#glm
include_directories(libraries/glm)

#src files
file(GLOB SRC src/*.cpp)

file(GLOB IMGUI_SRC
	libraries/imgui/*.cpp
	libraries/imgui/backends/imgui_impl_glfw.cpp
	libraries/imgui/backends/imgui_impl_vulkan.cpp
)

add_executable(V2Engine src/main.cpp ${LDFLAGS})
target_sources(V2Engine PUBLIC ${SRC} ${IMGUI_SRC})
#add_executable(V2Engine src/main.cpp ${LDFLAGS})

if (VULKAN_FOUND)
  message(STATUS "Found vulkan!")
  target_link_libraries(V2Engine Vulkan::Vulkan)
  target_link_libraries(V2Engine glfw)
if ( MSVC )
else()
  #-lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
  target_link_libraries(V2Engine dl pthread X11 Xxf86vm Xrandr Xi)
endif()
endif(VULKAN_FOUND)
