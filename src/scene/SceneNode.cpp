#include "scene/SceneNode.h"
#include "graphics/Material.h"
#include "graphics/Primitives.h"
#include "graphics/Model.h"
#include "graphics/JsonHelpers.h"
#include "graphics/TextureLoader.h"

SceneNode::SceneNode(std::string name, NodeType type, Primitive* mesh) 
    : name(name), type(type), mesh(mesh), parent(nullptr), 
      position(0.0f), rotation(0.0f), scale(1.0f),
      affectedByLight(type != NodeType::BILLBOARD),
      lightColor(1.0f, 1.0f, 1.0f), lightIntensity(1.0f),
      fov(45.0f), environment(nullptr) // Inicializa nulo
{
    if (type == NodeType::ENVIRONMENT) {
        environment = new Environment();
    }
}

SceneNode::~SceneNode() {
    // Limpa o environment para não vazar memória
    if (environment != nullptr) {
        delete environment;
    }

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
    j["name"] = name;
    j["type"] = static_cast<int>(type);
    
    // Olha como fica dinâmico! Não precisamos mais separar eixos.
    j["position"] = position;
    j["rotation"] = rotation;
    j["scale"] = scale;
    
    // Salva o material INTEIRO de uma vez, usando a regra do Macro que criamos no .h
    j["material"] = material;
    
    j["affectedByLight"] = affectedByLight;

    if (type == NodeType::LIGHT) {
        j["lightColor"] = lightColor;
        j["lightIntensity"] = lightIntensity;
    }

    if (type == NodeType::ENVIRONMENT && environment != nullptr) {
        j["environment_data"] = *environment;
    }

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
    this->name = j.value("name", "Objeto");
    this->type = static_cast<NodeType>(j.value("type", 0));

    // Carrega o vetor inteiro de uma vez
    if (j.contains("position")) this->position = j["position"];
    if (j.contains("rotation")) this->rotation = j["rotation"];
    if (j.contains("scale"))    this->scale = j["scale"];
    
    if (j.contains("material")) {
        this->material = j["material"].get<Material>();

        // --- RECARREGAMENTO DINÂMICO DE TEXTURAS ---
        
        // 1. Diffuse Map
        if (!this->material.diffusePath.empty()) {
            std::string fullPath = (std::filesystem::path(currentProjectPath) / this->material.diffusePath).generic_string();
            if (std::filesystem::exists(fullPath)) {
                this->material.diffuseMap = TextureLoader::loadTexture(fullPath.c_str());
            }
        }

        // 2. Specular Map
        if (!this->material.specularPath.empty()) {
            std::string fullPath = (std::filesystem::path(currentProjectPath) / this->material.specularPath).generic_string();
            if (std::filesystem::exists(fullPath)) {
                this->material.specularMap = TextureLoader::loadTexture(fullPath.c_str());
            }
        }

        // 3. Normal Map
        if (!this->material.normalPath.empty()) {
            std::string fullPath = (std::filesystem::path(currentProjectPath) / this->material.normalPath).generic_string();
            if (std::filesystem::exists(fullPath)) {
                this->material.normalMap = TextureLoader::loadTexture(fullPath.c_str());
            }
        }
    }
    
    this->affectedByLight = j.value("affectedByLight", true);

    if (this->type == NodeType::LIGHT) {
        if (j.contains("lightColor")) this->lightColor = j["lightColor"];
        this->lightIntensity = j.value("lightIntensity", 1.0f);
    }

    if (this->type == NodeType::ENVIRONMENT && this->environment != nullptr) {
        if (j.contains("environment_data")) {
            *this->environment = j["environment_data"].get<Environment>();
        }
    }

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
                    Model* loadedModel = new Model(fullPath.generic_string());
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

