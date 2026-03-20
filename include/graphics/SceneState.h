#ifndef SCENESTATE_H
#define SCENESTATE_H

#include <glm/glm.hpp>

// "Avisa" o compilador que a classe SceneNode existe para podermos criar um ponteiro
class SceneNode; 

struct SceneState {
    bool wireframeMode = false;
    SceneNode* selectedNode = nullptr; 
};

#endif