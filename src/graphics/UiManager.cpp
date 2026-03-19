#include "graphics/UIManager.h"
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>

UIManager::UIManager(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

UIManager::~UIManager() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(nullptr);
}

void UIManager::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
}

void UIManager::render(SceneState& state) {
    ImGui::Begin("Propriedades da Cena");
    ImGui::Text("Pressione TAB para alternar Câmera/UI");
    ImGui::Separator();

    ImGui::Text("Iluminação");
    ImGui::ColorEdit3("Cor da Luz", glm::value_ptr(state.lightColor));
    ImGui::DragFloat3("Posição da Luz", glm::value_ptr(state.lightPos), 0.1f);
    ImGui::Separator();

    ImGui::Text("Visualização");
    if (ImGui::Checkbox("Modo Wireframe", &state.wireframeMode)) {
        if (state.wireframeMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    ImGui::Separator();

    ImGui::Text("Transformação do Objeto");
    ImGui::DragFloat3("Posição", glm::value_ptr(state.objPosition), 0.05f);
    ImGui::DragFloat3("Rotação", glm::value_ptr(state.objRotation), 1.0f);
    ImGui::DragFloat3("Escala", glm::value_ptr(state.objScale), 0.05f);
    
    ImGui::End();
}

void UIManager::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}