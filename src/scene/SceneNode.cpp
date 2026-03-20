#include "scene/SceneNode.h"

// Função que gera um pixel branco de 1x1 na GPU e guarda o ID dele
static unsigned int GetDefaultTexture() {
    static unsigned int whiteTex = 0;
    if (whiteTex == 0) { // Só gera na primeira vez que for chamada
        glGenTextures(1, &whiteTex);
        glBindTexture(GL_TEXTURE_2D, whiteTex);
        
        // Um array de 4 bytes: R=255, G=255, B=255, Alpha=255 (Branco Puro)
        unsigned char whitePixel[] = {255, 255, 255, 255}; 
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
        
        // Filtros simples porque é só 1 pixel
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    return whiteTex;
}

SceneNode::SceneNode(std::string name, NodeType type, Primitive* mesh) 
    : name(name), type(type), mesh(mesh), parent(nullptr), 
      position(0.0f), rotation(0.0f), scale(1.0f),
      baseColor(1.0f, 1.0f, 1.0f, 1.0f),
      affectedByLight(type != NodeType::BILLBOARD), // Billboards iniciam a 'false'
      lightColor(1.0f, 1.0f, 1.0f), lightIntensity(1.0f),
      fov(45.0f),
      textureID(0)
{}

SceneNode::~SceneNode() {
    // Limpa os filhos da memória quando este nó for destruído
    for (auto child : children) {
        delete child;
    }
}

void SceneNode::addChild(SceneNode* child) {
    child->parent = this;
    children.push_back(child);
}

glm::mat4 SceneNode::getLocalTransform(const glm::mat4& viewMatrix) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    if (type == NodeType::BILLBOARD) {
        // 2. Aplica o Scale primeiro (para não achatar a rotação final)
        model = glm::scale(model, scale);

        // 3. A MÁGICA DO BILLBOARD (Anula a rotação em relação à câmera)
        glm::mat4 alignMatrix = glm::mat4(1.0f);
        alignMatrix[0][0] = viewMatrix[0][0]; alignMatrix[0][1] = viewMatrix[1][0]; alignMatrix[0][2] = viewMatrix[2][0];
        alignMatrix[1][0] = viewMatrix[0][1]; alignMatrix[1][1] = viewMatrix[1][1]; alignMatrix[1][2] = viewMatrix[2][1];
        alignMatrix[2][0] = viewMatrix[0][2]; alignMatrix[2][1] = viewMatrix[1][2]; alignMatrix[2][2] = viewMatrix[2][2];
        
        model = model * alignMatrix;

        // 4. A CORREÇÃO DE 90 GRAUS! (Para o Plane "ficar em pé" no eixo local)
        // Como o Plane padrão é deitado (XZ), giramos 90 no X local para ele ficar de pé no XY, 
        // olhando para o Z local (que o billboard travou na câmera!)
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    } else {
        // Rotação normal 3D para os outros objetos
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    
    model = glm::scale(model, scale);
    return model;
}

void SceneNode::draw(Shader& shader, const glm::mat4& viewMatrix, glm::mat4 parentTransform) {
    // Passa a viewMatrix para pegar o transform correto
    glm::mat4 globalTransform = parentTransform * getLocalTransform(viewMatrix);

    if (mesh != nullptr) {
        shader.setMat4("model", globalTransform);
        glUniform4f(glGetUniformLocation(shader.ID, "uBaseColor"), baseColor.r, baseColor.g, baseColor.b, baseColor.a);
        shader.setBool("uAffectedByLight", affectedByLight);
        
        // A SUA SOLUÇÃO APLICADA!
        if (textureID != 0) {
            glBindTexture(GL_TEXTURE_2D, textureID); // Usa a textura do arquivo
        } else {
            glBindTexture(GL_TEXTURE_2D, GetDefaultTexture()); // Usa a nossa plain texture 1x1 branca!
        }
        
        mesh->draw();
    }

    // Manda a viewMatrix para os filhos também
    for (auto child : children) {
        child->draw(shader, viewMatrix, globalTransform);
    }
}