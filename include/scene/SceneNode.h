#ifndef SCENENODE_H
#define SCENENODE_H

#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "graphics/Primitives.h"
#include "graphics/Shader.h"
#include "graphics/Material.h"
#include "json.hpp"

using json = nlohmann::json;

// 1. As nossas categorias oficiais!
enum class NodeType { MESH, LIGHT, CAMERA, FOLDER, BILLBOARD, ENVIRONMENT };

class SceneNode {
public:
    bool isDraggable = true;
    bool hasRightclick = true;

    std::string name;
    NodeType type; // 2. Guarda o tipo deste nó
    Primitive* mesh; 

    // Transformações Locais (Todos os nós têm)
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    // --- PROPRIEDADES ESPECÍFICAS ---
    // Luz
    glm::vec3 lightColor;
    float lightIntensity;
    // Câmera
    float fov;

    bool affectedByLight;

    Material material;

    SceneNode* parent;
    std::vector<SceneNode*> children;

    //Environment variables
    glm::vec3 ambientColor;
    unsigned int skyboxTexture;
    glm::vec3 sunDirection;
    glm::vec3 sunColor;
    float sunIntensity;

    // 3. Atualizamos o construtor para pedir o Tipo
    SceneNode(std::string name, NodeType type = NodeType::FOLDER, Primitive* mesh = nullptr);
    ~SceneNode();

    void addChild(SceneNode* child);

    glm::mat4 getLocalTransform(const glm::mat4& viewMatrix);
    void draw(Shader& shader, const glm::mat4& viewMatrix, glm::mat4 parentTransform = glm::mat4(1.0f));

    json toJson();
    void fromJson(const json& j, const std::string& currentProjectPath = "");

    void clearNonSystemChildren() {
        for (int i = children.size() - 1; i >= 0; i--) {
            if (children[i]->type != NodeType::ENVIRONMENT) {
                delete children[i]; // Libera a RAM
                children.erase(children.begin() + i); // Tira da lista
            }
        }
    }
};



#endif