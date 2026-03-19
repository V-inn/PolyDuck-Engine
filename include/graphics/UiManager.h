#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <GLFW/glfw3.h>
#include "SceneState.h"

class UIManager {
public:
    UIManager(GLFWwindow* window); // Inicializa o ImGui
    ~UIManager();                  // Limpa a memória do ImGui no final

    void beginFrame();             // Prepara para desenhar
    void render(SceneState& state);// Desenha os menus e altera o State
    void endFrame();               // Envia para a placa de vídeo
};

#endif