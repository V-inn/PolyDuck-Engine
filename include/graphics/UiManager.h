#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "SceneState.h"
#include "scene/Scene.h"
class UIManager {
public:
    UIManager(GLFWwindow* window); // Inicializa o ImGui
    ~UIManager();                  // Limpa a memória do ImGui no final

    void beginFrame();             // Prepara para desenhar
    void render(SceneState& state, Scene& scene);
    void endFrame();               // Envia para a placa de vídeo
};

#endif