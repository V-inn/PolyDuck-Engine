#ifndef SCENENODE_H
#define SCENENODE_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "graphics/Primitives.h"
#include "graphics/Shader.h"

// 1. As nossas categorias oficiais!
enum class NodeType { MESH, LIGHT, CAMERA, FOLDER, BILLBOARD };

class SceneNode {
public:
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

    // Malha (Mesh)
    glm::vec4 baseColor; // RGBA (O 'A' é a transparência!)
    bool affectedByLight;

    unsigned int textureID;

    SceneNode* parent;
    std::vector<SceneNode*> children;

    // 3. Atualizamos o construtor para pedir o Tipo
    SceneNode(std::string name, NodeType type = NodeType::FOLDER, Primitive* mesh = nullptr);
    ~SceneNode();

    void addChild(SceneNode* child);

    glm::mat4 getLocalTransform(const glm::mat4& viewMatrix);
    void draw(Shader& shader, const glm::mat4& viewMatrix, glm::mat4 parentTransform = glm::mat4(1.0f));
};

#endif