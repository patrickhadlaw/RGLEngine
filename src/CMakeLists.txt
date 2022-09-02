cmake_minimum_required(VERSION 3.12)

set(
  RGLE_LIB_SRC
  rgle/gfx/Camera.cpp
  rgle/gfx/Font.cpp
  rgle/gfx/Graphics.cpp
  rgle/gfx/Image.cpp
  rgle/gfx/Renderable.cpp
  rgle/gfx/ShaderProgram.cpp
  rgle/gfx/Spatial.cpp
  rgle/math/Quadratic.cpp
  rgle/sync/Thread.cpp
  rgle/ui/Interface.cpp
  rgle/util/Color.cpp
  rgle/util/Config.cpp
  rgle/util/Console.cpp
  rgle/util/Tester.cpp
  rgle/util/Utility.cpp
  rgle/Application.cpp
  rgle/Event.cpp
  rgle/Exception.cpp
  rgle/Logger.cpp
  rgle/Node.cpp
  rgle/Platform.cpp
  rgle/Raycast.cpp
  rgle/Window.cpp
)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS YES)

add_library(rgle SHARED ${RGLE_LIB_SRC})
target_compile_definitions(rgle PUBLIC RGLE_DLL_BUILD_MODE)

target_link_libraries(rgle ${RGLE_LINK_LIBS})
target_include_directories(rgle PUBLIC ${RGLE_INCLUDE_DIRS})
target_include_directories(rgle SYSTEM PUBLIC ${RGLE_LIB_INCLUDE_DIRS})
set_target_properties(rgle PROPERTIES PUBLIC_HEADER "include/rgle.h")

if (MSVC)
	add_compile_options(/W4 /WX)
else()
	add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

install(TARGETS rgle
  LIBRARY DESTINATION lib
	ARCHIVE DESTINATION bin
	PUBLIC_HEADER DESTINATION include
	COMPONENT LIBRARY
)
