#ifndef SCENESTATE_H
#define SCENESTATE_H

#include <glm/glm.hpp>
#include <string>

// "Avisa" o compilador que a classe SceneNode existe para podermos criar um ponteiro
class SceneNode; 

struct SceneState {
    bool wireframeMode = false;
    bool showSkybox = true;
    SceneNode* selectedNode = nullptr;

    float viewportWidth = 1200.0f;
    float viewportHeight = 800.0f;

    std::string currentProjectPath = "";
};

#endif