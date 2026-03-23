cmake_minimum_required(VERSION 3.15)
# Nome do seu projeto e a linguagem
project(Engine3D LANGUAGES CXX C)

# Define o padrão do C++ para C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ---------------------------------------------------------
# 1. BAIXAR E CONFIGURAR O GLFW AUTOMATICAMENTE
# ---------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8 # Uma versão muito estável
)
# Desliga a compilação de testes e docs do GLFW para ser mais rápido
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ---------------------------------------------------------
# 3. BAIXAR E CONFIGURAR O GLM (MATEMÁTICA)
# ---------------------------------------------------------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# ---------------------------------------------------------
# 2. CONFIGURAR A NOSSA ENGINE
# ---------------------------------------------------------
# Diz ao compilador onde encontrar os arquivos .h (o glad e o KHR)
include_directories(include)


# Cria o executável juntando o seu main e o arquivo C do glad
add_executable(Engine3D src/main.cpp
                src/glad.c 
                src/graphics/Shader.cpp 
                src/stb_image.cpp 
                src/graphics/Primitives.cpp
                src/tiny_obj_loader.cpp
                src/graphics/Model.cpp 
                src/graphics/UiManager.cpp
                src/scene/SceneNode.cpp
                src/scene/Scene.cpp
                src/graphics/Material.cpp
                src/graphics/TextureLoader.cpp
                include/imgui/imgui.cpp 
                include/imgui/imgui_tables.cpp 
                include/imgui/imgui_widgets.cpp 
                include/imgui/imgui_draw.cpp 
                include/imgui/backends/imgui_impl_glfw.cpp 
                include/imgui/backends/imgui_impl_opengl3.cpp 
                )

target_include_directories(Engine3D PRIVATE include include/imgui)

# Encontra a biblioteca base do OpenGL no seu sistema operacional
find_package(OpenGL REQUIRED)

# Linka o GLFW e o OpenGL ao nosso executável
target_link_libraries(Engine3D PRIVATE glfw OpenGL::GL glm::glm)

# Copia a pasta de shaders da raiz do projeto para a pasta de build
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/user_assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})