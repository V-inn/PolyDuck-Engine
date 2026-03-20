#include <glad/glad.h>
#include <stb_image.h>
#include "graphics/UIManager.h"
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <filesystem>
#include "graphics/Model.h"

namespace fs = std::filesystem;

double modulo(double x, double y) {
    return fmod(fmod(x, y) + y, y);
}

static unsigned int CarregarTexturaDoArquivo(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    // Força inverter o Y na leitura para alinhar com o OpenGL
    stbi_set_flip_vertically_on_load(true); 
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA; // Suporte a Transparência!

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Configurações padrão de repetição e filtro
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Falha ao carregar textura no caminho: " << path << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}

UIManager::UIManager(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

UIManager::~UIManager() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(nullptr);
}

void UIManager::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
}

void UIManager::render(SceneState& state, Scene& scene) {
    // ---------------------------------------------------
    // 0. MAIN MENU BAR (A barra superior clássica!)
    // ---------------------------------------------------
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Arquivo")) {
            ImGui::MenuItem("Nova Cena", "Ctrl+N");
            ImGui::MenuItem("Salvar", "Ctrl+S");
            if (ImGui::MenuItem("Sair", "Alt+F4")) {
                // No futuro, conectaremos isso ao glfwSetWindowShouldClose
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Adicionar")) {
            if (ImGui::MenuItem("Pasta (Folder)")) { 
                scene.root->addChild(new SceneNode("Nova Pasta", NodeType::FOLDER)); 
            }
            if (ImGui::MenuItem("Luz Direcional")) { 
                scene.root->addChild(new SceneNode("Nova Luz", NodeType::LIGHT)); 
            }
            if (ImGui::MenuItem("Billboard (Sprite 2D)")) { 
                // Precisa instanciar um Plane para servir de tela para a imagem
                scene.root->addChild(new SceneNode("Novo Billboard", NodeType::BILLBOARD, new Plane(1.0f, 1.0f, 1, 1))); 
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // ---------------------------------------------------
    // JANELA 1: Configurações Globais da Cena
    // ---------------------------------------------------
    ImGui::Begin("Cena");
    ImGui::Separator();
    ImGui::Checkbox("Modo Wireframe", &state.wireframeMode);
    ImGui::End();

    // ---------------------------------------------------
    // JANELA 2: HIERARQUIA (Lista de Objetos)
    // ---------------------------------------------------
    ImGui::Begin("Hierarquia");
    // Lemos todos os filhos diretos do nó raiz da nossa cena
    for (auto child : scene.root->children) {
        // Verifica se o item atual do loop é o mesmo que está selecionado na memória
        bool isSelected = (state.selectedNode == child);
        
        // ImGui::Selectable cria um botão invisível que fica azul quando clicado
        if (ImGui::Selectable(child->name.c_str(), isSelected)) {
            state.selectedNode = child; // Atualiza o ponteiro!
        }
    }
    ImGui::End();

    // ---------------------------------------------------
    // JANELA 3: INSPETOR DINÂMICO
    // ---------------------------------------------------
    ImGui::Begin("Inspetor");
    if (state.selectedNode != nullptr) {
        char nameBuf[256];
        strcpy(nameBuf, state.selectedNode->name.c_str());
        if (ImGui::InputText("Nome", nameBuf, sizeof(nameBuf))) {
            state.selectedNode->name = std::string(nameBuf); // Atualiza em tempo real!
        }
        ImGui::Separator();
        
        // 2. SLIDER DE SENSIBILIDADE (STEP)
        static float step = 0.05f; // 'static' mantém o valor guardado entre os frames
        ImGui::DragFloat("Sensibilidade dos Sliders", &step, 0.01f, 0.01f, 2.0f);
        ImGui::Separator();

        // 3. TRANSFORMAÇÕES COM MÓDULO
        ImGui::Text("Transformacoes");
        ImGui::DragFloat3("Posicao", glm::value_ptr(state.selectedNode->position), step);
        
        // Usamos DragFloat3 normal, mas aplicamos a sua função 'modulo' logo em seguida!
        if (ImGui::DragFloat3("Rotacao", glm::value_ptr(state.selectedNode->rotation), step)) {
            state.selectedNode->rotation.x = (float)modulo(state.selectedNode->rotation.x, 360.0);
            state.selectedNode->rotation.y = (float)modulo(state.selectedNode->rotation.y, 360.0);
            state.selectedNode->rotation.z = (float)modulo(state.selectedNode->rotation.z, 360.0);
        }
        
        ImGui::DragFloat3("Escala", glm::value_ptr(state.selectedNode->scale), step);
        ImGui::Separator();
        
        // 4. PAINEL DINÂMICO
        if (state.selectedNode->type == NodeType::MESH) {
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "[Malha 3D]");
            // ColorEdit4 ao invés de 3, para termos a barrinha de Alpha (Transparência)!
            ImGui::ColorEdit4("Cor Base", glm::value_ptr(state.selectedNode->baseColor));
            ImGui::Checkbox("Afetado pela Iluminacao", &state.selectedNode->affectedByLight);
        } 
        else if (state.selectedNode->type == NodeType::LIGHT) {
            ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.2f, 1.0f), "[Luz]");
            ImGui::ColorEdit3("Cor da Luz", glm::value_ptr(state.selectedNode->lightColor));
            ImGui::DragFloat("Intensidade", &state.selectedNode->lightIntensity, 0.05f, 0.0f, 10.0f);
        }
        else if (state.selectedNode->type == NodeType::FOLDER) {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "[Pasta / Grupo]");
            ImGui::TextDisabled("Use para agrupar outros objetos.");
        }
        else if (state.selectedNode->type == NodeType::BILLBOARD) {
            ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "[Billboard 2D]");
            ImGui::Text("Textura: %s", (state.selectedNode->textureID == 0) ? "Nenhuma (Clique em um .png abaixo)" : "Carregada!");
            ImGui::ColorEdit4("Cor/Filtro", glm::value_ptr(state.selectedNode->baseColor));
            ImGui::Checkbox("Afetado pela Iluminacao", &state.selectedNode->affectedByLight);
        }
    } else {
        ImGui::TextDisabled("Selecione um objeto na Hierarquia.");
    }
    ImGui::End();

    // ---------------------------------------------------
    // JANELA 4: NAVEGADOR DE ARQUIVOS (Asset Browser)
    // ---------------------------------------------------
    ImGui::Begin("Navegador de Arquivos");
    ImGui::Text("Diretorio: ./assets/");
    ImGui::Separator();

    // Varre a pasta assets do seu HD em tempo real
    for (const auto& entry : fs::directory_iterator("./assets")) {
        std::string path = entry.path().string();
        std::string extension = entry.path().extension().string();
        std::string filename = entry.path().filename().string();

        if (extension == ".obj") {
            std::string buttonLabel = "[3D] " + filename;
            if (ImGui::Button(buttonLabel.c_str())) {
                // Aloca o modelo na memória da GPU lendo o arquivo clicado
                Model* novoModelo = new Model(path.c_str());
                // Cria um nó na nossa árvore de cena com o nome do arquivo
                SceneNode* novoNode = new SceneNode(filename, NodeType::MESH, novoModelo);
                // Adiciona o nó à cena
                scene.root->addChild(novoNode);
                // Já deixa o objeto recém-criado selecionado no Inspetor!
                state.selectedNode = novoNode; 
            }
        }
        else if (extension == ".jpg" || extension == ".png") {
            std::string buttonLabel = "[IMG] " + filename;
            if (ImGui::Button(buttonLabel.c_str())) {
                // Se eu clicar numa imagem e tiver um Billboard selecionado...
                if (state.selectedNode != nullptr && state.selectedNode->type == NodeType::BILLBOARD) {
                    // Carrega a textura e injeta o ID no nó selecionado!
                    state.selectedNode->textureID = CarregarTexturaDoArquivo(path.c_str());
                    // Bônus: Reseta a cor base para branco puro para não filtrar a imagem
                    state.selectedNode->baseColor = glm::vec4(1.0f); 
                }
            }
        }
    }
    ImGui::End();
}

void UIManager::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}