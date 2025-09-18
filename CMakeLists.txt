cmake_minimum_required(VERSION 3.19)
project(Clay_OpenGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find FreeType
find_package(Freetype REQUIRED)

# Find GLFW
find_package(glfw3 REQUIRED)

# Find GLM
find_package(glm REQUIRED)

# Conditional Glad
if(NOT TARGET glad)
    add_library(glad STATIC third_party/glad/src/glad.c)
    target_include_directories(glad PUBLIC third_party/glad/include)
endif()

# Clay renderer library
add_library(clay_renderer STATIC
  ${CMAKE_SOURCE_DIR}/src/clay_gl.h
  ${CMAKE_SOURCE_DIR}/src/clay_gl.cpp

  ${CMAKE_SOURCE_DIR}/src/text.h
  ${CMAKE_SOURCE_DIR}/src/text.cpp
  
  ${CMAKE_SOURCE_DIR}/src/rect.h
  ${CMAKE_SOURCE_DIR}/src/rect.cpp

  ${CMAKE_SOURCE_DIR}/src/stb_image_write.h
  ${CMAKE_SOURCE_DIR}/src/stb_image.h

  ${CMAKE_SOURCE_DIR}/src/clay.h

  ${CMAKE_SOURCE_DIR}/src/impl.cpp

  ${CMAKE_SOURCE_DIR}/src/gl_util.h
  ${CMAKE_SOURCE_DIR}/src/gl_util.cpp
)

target_link_libraries(clay_renderer PUBLIC OpenGL::GL glad glfw glm::glm ${FREETYPE_LIBRARIES})
target_include_directories(clay_renderer PUBLIC ${CMAKE_SOURCE_DIR}/src ${FREETYPE_INCLUDE_DIRS})

# Build example executable if enabled
option(CLAY_BUILD_EXAMPLES "Build example executables for testing" OFF)
if(CLAY_BUILD_EXAMPLES)
    add_executable(clay_example
      ${CMAKE_SOURCE_DIR}/src/example.cpp
    )
    target_link_libraries(clay_example PRIVATE clay_renderer)
endif()
