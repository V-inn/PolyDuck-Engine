#include "scene/SceneNode.h"
#include "graphics/Material.h"

SceneNode::SceneNode(std::string name, NodeType type, Primitive* mesh) 
    : name(name), type(type), mesh(mesh), parent(nullptr), 
      position(0.0f), rotation(0.0f), scale(1.0f),
      affectedByLight(type != NodeType::BILLBOARD), // Billboards iniciam a 'false'
      lightColor(1.0f, 1.0f, 1.0f), lightIntensity(1.0f),
      fov(45.0f)
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
        shader.setBool("uAffectedByLight", affectedByLight);
        
        // --- PROPRIEDADES DO MATERIAL ---
        glUniform4f(glGetUniformLocation(shader.ID, "uBaseColor"), material.baseColor.r, material.baseColor.g, material.baseColor.b, material.baseColor.a);
        shader.setFloat("uSpecularStrength", material.specularStrength);
        shader.setFloat("uShininess", material.shininess);
        
        // --- A MAGIA DO RESET DE ESTADO ---
        
        // 1. OBRIGA o OpenGL a usar o Slot 0 para o Diffuse!
        // Se pularmos isso, ele pode tentar desenhar a cor no Slot 1.
        glActiveTexture(GL_TEXTURE0); 
        
        if (material.diffuseMap != 0) {
            glBindTexture(GL_TEXTURE_2D, material.diffuseMap); 
        } else {
            glBindTexture(GL_TEXTURE_2D, Material::GetDefaultTexture()); 
        }
        shader.setInt("diffuseMap", 0); 

        // 2. Slot 1: Specular Map
        glActiveTexture(GL_TEXTURE1);
        if (material.specularMap != 0) {
            glBindTexture(GL_TEXTURE_2D, material.specularMap);
            shader.setBool("uHasSpecularMap", true);
        } else {
            glBindTexture(GL_TEXTURE_2D, 0); // Desativa se não tiver
            shader.setBool("uHasSpecularMap", false);
        }
        shader.setInt("specularMap", 1); 
        
        // 3. O RESET FINAL DE SEGURANÇA!
        // Após configurar tudo, VOLTA para o Slot 0 antes de desenhar!
        glActiveTexture(GL_TEXTURE0); 
        
        mesh->draw(); 
    }

    // Manda a viewMatrix para os filhos também
    for (auto child : children) {
        child->draw(shader, viewMatrix, globalTransform);
    }
}