#include "scene/SceneNode.h"
#include "graphics/Material.h"
#include "graphics/Primitives.h"
#include "graphics/Model.h"

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
        shader.setFloat("uReflectivity", material.reflectivity);
        
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

        glActiveTexture(GL_TEXTURE2);
        if (material.normalMap != 0) {
            glBindTexture(GL_TEXTURE_2D, material.normalMap);
            shader.setBool("uHasNormalMap", true);
        } else {
            glBindTexture(GL_TEXTURE_2D, 0);
            shader.setBool("uHasNormalMap", false);
        }
        shader.setInt("normalMap", 2); // Lê da Porta 2!

        glActiveTexture(GL_TEXTURE0); 
        
        mesh->draw(); 
    }

    // Manda a viewMatrix para os filhos também
    for (auto child : children) {
        child->draw(shader, viewMatrix, globalTransform);
    }
}

json SceneNode::toJson() {
    json j;
    
    // 1. Salva os dados básicos
    j["name"] = name;
    j["type"] = static_cast<int>(type); // Salva o Enum como um número
    
    // 2. Salva Vetores (O nlohmann converte arrays do C++ para arrays do JSON lindamente)
    // Assumindo que você tem transformações no seu SceneNode:
    j["position"] = { position.x, position.y, position.z };
    j["rotation"] = { rotation.x, rotation.y, rotation.z };
    j["scale"]    = { scale.x, scale.y, scale.z };

    if (this->mesh != nullptr) {
        // Se for um modelo importado, salvamos o tipo e o caminho do arquivo
        if (Model* m = dynamic_cast<Model*>(this->mesh)) {
            j["meshType"] = "model";
            j["meshPath"] = m->filePath;
        } 
        // Se forem primitivas, salvamos apenas o nome da forma
        else if (dynamic_cast<Box*>(this->mesh)) {
            j["meshType"] = "box";
        } 
        else if (dynamic_cast<Sphere*>(this->mesh)) {
            j["meshType"] = "sphere";
        } 
        else if (dynamic_cast<Plane*>(this->mesh)) {
            j["meshType"] = "plane";
        }
    } else {
        j["meshType"] = "none"; // Nós como Pastas ou Luzes não têm malha
    }

    // 3. Propriedades Específicas de Sistemas (O seu Environment!)
    if (type == NodeType::ENVIRONMENT) {
        j["ambientColor"] = { ambientColor.x, ambientColor.y, ambientColor.z };
        j["sunDirection"] = { sunDirection.x, sunDirection.y, sunDirection.z };
        j["sunColor"]     = { sunColor.x, sunColor.y, sunColor.z };
        j["sunIntensity"] = sunIntensity;
        // Nota: Não salvamos o ID da textura (skyboxTexture), porque esse ID (ex: 15) 
        // muda toda vez que abrimos a engine. Para texturas, salvaríamos o caminho (path) do arquivo!
    }

    // 4. A RECURSÃO: Salva todos os filhos dentro de uma lista JSON!
    j["children"] = json::array();
    for (SceneNode* child : children) {
        // Ignora o Environment na lista de filhos, pois a Scene já o cria por padrão
        if (child->type != NodeType::ENVIRONMENT) { 
            j["children"].push_back(child->toJson());
        }
    }

    return j;
}

void SceneNode::fromJson(const json& j, const std::string& currentProjectPath) {
    // 1. Carrega dados básicos usando o .value() para EXTENSIBILIDADE!
    // Se "name" não existir no arquivo, ele usa "Node Desconhecido"
    name = j.value("name", name);
    type = static_cast<NodeType>(j.value("type", 0));

    // 2. Carrega Vetores (Com checagem de segurança)
    if (j.contains("position")) {
         position.x = j["position"][0];
         position.y = j["position"][1];
         position.z = j["position"][2];
    }
    if (j.contains("rotation")) {
         rotation.x = j["rotation"][0];
         rotation.y = j["rotation"][1];
         rotation.z = j["rotation"][2];
    }
    if (j.contains("scale")) {
         scale.x = j["scale"][0];
         scale.y = j["scale"][1];
         scale.z = j["scale"][2];
    }

    // 3. Propriedades do Environment
    if (type == NodeType::ENVIRONMENT) {
        if (j.contains("ambientColor")) {
            ambientColor = glm::vec3(j["ambientColor"][0], j["ambientColor"][1], j["ambientColor"][2]);
        }
        if (j.contains("sunDirection")) {
            sunDirection = glm::vec3(j["sunDirection"][0], j["sunDirection"][1], j["sunDirection"][2]);
        }
        if (j.contains("sunColor")) {
            sunColor = glm::vec3(j["sunColor"][0], j["sunColor"][1], j["sunColor"][2]);
        }
        sunIntensity = j.value("sunIntensity", 1.0f); // Valor padrão de 1.0
    }

    // 4. A RECURSÃO: Recria os filhos!
    if (j.contains("children")) {
        for (const json& childJson : j["children"]) {
            
            std::string childName = childJson.value("name", "Node Desconhecido");
            NodeType childType = static_cast<NodeType>(childJson.value("type", 0));
            std::string meshType = childJson.value("meshType", "none");
            Primitive* childMesh = nullptr;

            if (meshType == "box") {
                childMesh = new Box(1.0f, 1.0f, 1.0f); 
            } 
            else if (meshType == "sphere") {
                childMesh = new Sphere(1.0f, 36, 18);
            } 
            else if (meshType == "plane") {
                childMesh = new Plane(10.0f, 10.0f, 10, 10, 1.0f);
            } 
            else if (meshType == "model") {
                std::string savedPath = childJson.value("meshPath", "");
                std::filesystem::path fullPath = savedPath;
                
                if (!currentProjectPath.empty() && savedPath.find(":") == std::string::npos) {
                    fullPath = std::filesystem::path(currentProjectPath) / savedPath;
                }
                
                if (!savedPath.empty() && std::filesystem::exists(fullPath)) {
                    Model* loadedModel = new Model(fullPath.string());
                    loadedModel->filePath = savedPath; 
                    childMesh = loadedModel;
                } else {
                    // +++ DEBUG IMPORTANTE +++
                    std::cout << "ALERTA: O arquivo do modelo sumiu ou o caminho esta errado: " << fullPath << std::endl;
                }
            }

            SceneNode* newChild = new SceneNode(childName, childType, childMesh);

            // +++ CORREÇÃO DA RECURSÃO: Passa o caminho do projeto adiante! +++
            newChild->fromJson(childJson, currentProjectPath);

            this->addChild(newChild);
        }
    }
}

