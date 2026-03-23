#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "graphics/Shader.h"
#include "graphics/Primitives.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include "graphics/Model.h"
#include "graphics/Camera.h"
#include "graphics/SceneState.h"
#include "graphics/UiManager.h"
#include "scene/Scene.h"
#include "graphics/TextureLoader.h"
#include <filesystem>

namespace fs = std::filesystem;

// Instancia a câmera globalmente
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// Variáveis de controle do mouse e tempo
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
bool firstMouse = true;

// Variáveis de tempo (DeltaTime garante que a velocidade seja a mesma em qualquer PC)
float deltaTime = 0.0f; 
float lastFrame = 0.0f;

bool uiMode = false; // Começa no modo Câmera (falso)
bool tabKeyPressed = false; // Para evitar que a tecla ative várias vezes num único clique

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (uiMode) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Invertido: coordenadas Y vão de baixo para cima no OpenGL

    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Lógica de Toggle (Alternar) com a tecla TAB
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (!tabKeyPressed) {
            uiMode = !uiMode; // Inverte o modo
            if (uiMode) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Solta o mouse
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Prende o mouse
                firstMouse = true; // Evita um "pulo" brusco da câmera ao voltar
            }
            tabKeyPressed = true;
        }
    } else {
        tabKeyPressed = false; // Reseta quando a tecla é solta
    }

    // Só move a câmera pelo teclado se NÃO estivermos no modo UI
    if (!uiMode) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.ProcessKeyboard(DOWN, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.ProcessKeyboard(UP, deltaTime);
    }
}

void drop_callback(GLFWwindow* window, int count, const char** paths) {
    // Garante que a pasta existe
    if (!fs::exists("./user_assets")) {
        fs::create_directory("./user_assets");
    }

    for (int i = 0; i < count; i++) {
        fs::path sourcePath(paths[i]); // O caminho original do arquivo no PC do usuário
        fs::path destinationPath = fs::path("./user_assets") / sourcePath.filename();

        try {
            // Copia o arquivo para a nossa pasta. overwrite_existing substitui se já tiver um igual.
            fs::copy(sourcePath, destinationPath, fs::copy_options::overwrite_existing);
            std::cout << "[Importacao] Arquivo copiado com sucesso: " << destinationPath << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "[Erro] Falha ao copiar arquivo: " << e.what() << std::endl;
        }
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1200, 800, "RenderEngine", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. Constrói o nosso Shader Program lendo os arquivos
    Shader nossoShader("shaders/shader.vs", "shaders/shader.fs");

    Shader unshadedShader("shaders/unshaded.vs", "shaders/unshaded.fs");

    // --- CARREGAR O ÍCONE DA LUZ ---
    unsigned int lightIconTexture;
    glGenTextures(1, &lightIconTexture);
    glBindTexture(GL_TEXTURE_2D, lightIconTexture);
    
    // Configurações para imagens transparentes (Clamp to Edge evita bordas estranhas)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int w, h, nrC;
    // Opcional: Se a sua imagem estiver de cabeça para baixo, mantenha o stbi_set_flip_vertically_on_load(true);
    unsigned char *iconData = stbi_load("assets/light-icon.png", &w, &h, &nrC, 0);
    
    if (iconData) {
        GLenum format = (nrC == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, iconData);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        // O C++ AGORA VAI DEDURAR O MOTIVO REAL!
        std::cout << "Aviso: Falha ao carregar light-icon.png!" << std::endl;
        std::cout << "Motivo do erro: " << stbi_failure_reason() << std::endl;
    }
    stbi_image_free(iconData);

    // Instanciação das Primitivas (Agora elas ficam na memória aguardando serem usadas)
    Plane groundPlane(10.0f, 10.0f, 20, 20); 

    // Criando a Cena e Populando o Grafo!
    Scene minhaCena;

    Sphere originDot(0.02f, 10, 5);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetDropCallback(window, drop_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Vértices de um cubo simples de tamanho 1x1x1 (Apenas posições X, Y, Z)
    float skyboxVertices[] = {
        // posições          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // Compila os novos shaders do céu
    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");

    std::vector<std::string> faces
    {
        "assets/skyboxes/water/right.jpg",
        "assets/skyboxes/water/left.jpg",
        "assets/skyboxes/water/top.jpg",
        "assets/skyboxes/water/bottom.jpg",
        "assets/skyboxes/water/front.jpg",
        "assets/skyboxes/water/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // Avisa o shader do skybox que a textura estará na porta 0
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    
    SceneState sceneState;
    UIManager uiManager(window);

    // Loop de Renderização
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        uiManager.beginFrame();
        uiManager.render(sceneState, minhaCena);

        // Pega tamanho da tela e calcula Projection/View (Iguais para todos)
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        if (height == 0) height = 1; 
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ---------------------------------------------------------------------
        // CAÇADOR DE LUZES (Guarda as luzes numa lista)
        // ---------------------------------------------------------------------
        std::vector<SceneNode*> listaDeLuzes;
        for (auto node : minhaCena.root->children) {
            if (node->type == NodeType::LIGHT) {
                listaDeLuzes.push_back(node);
            }
        }

        // ---------------------------------------------------------------------
        // FASE 1: DESENHAR O EDITOR (Ground Grid e Ponto Central)
        // ---------------------------------------------------------------------
        unshadedShader.use();
        unshadedShader.setMat4("projection", projection);
        unshadedShader.setMat4("view", view);

        // A. Desenhar o Ground Plane como um Grid Verde
        glm::mat4 groundModel = glm::mat4(1.0f);
        groundModel = glm::translate(groundModel, glm::vec3(0.0f, -0.01f, 0.0f)); 
        unshadedShader.setMat4("model", groundModel);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
        unshadedShader.setVec3("uColor", glm::vec3(0.1f, 0.1f, 0.1f)); 
        groundPlane.draw();
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Reseta o preenchimento

        // B. Desenhar o Ponto Central (Origin Dot)
        glm::mat4 dotModel = glm::mat4(1.0f);
        unshadedShader.setMat4("model", dotModel);
        unshadedShader.setVec3("uColor", glm::vec3(1.0f, 0.0f, 0.0f)); 
        originDot.draw();

        // ---------------------------------------------------------------------
        // FASE 2: DESENHAR A CENA 3D
        // ---------------------------------------------------------------------
        if (!sceneState.wireframeMode) {
            // MODO NORMAL COM LUZES
            nossoShader.use();
            
            // AGORA SIM! Enviando a Câmera para o Shader com luzes
            nossoShader.setMat4("projection", projection);
            nossoShader.setMat4("view", view);
            nossoShader.setVec3("viewPos", camera.Position);

            // --- ATIVANDO OS REFLEXOS ---
            // Diz ao shader que a textura do céu estará na unidade 1
            nossoShader.setInt("skybox", 4); 
            
            // Ativa a porta 1 e conecta o nosso céu de água nela!
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

            glActiveTexture(GL_TEXTURE0);
            
            int countLuzes = std::min((int)listaDeLuzes.size(), 10);
            nossoShader.setInt("numLights", countLuzes);

            for (int i = 0; i < countLuzes; i++) {
                std::string posName = "lightPos[" + std::to_string(i) + "]";
                std::string colName = "lightColor[" + std::to_string(i) + "]";
                
                nossoShader.setVec3(posName, listaDeLuzes[i]->position);
                nossoShader.setVec3(colName, listaDeLuzes[i]->lightColor * listaDeLuzes[i]->lightIntensity);
            }
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            
        } else {
            // MODO WIREFRAME (Sem luz, usa o unshadedShader)
            unshadedShader.use();
            unshadedShader.setMat4("projection", projection);
            unshadedShader.setMat4("view", view);
            unshadedShader.setVec3("uColor", glm::vec3(1.0f, 0.5f, 0.0f)); // Cor das linhas do Wireframe
            
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        // Desenha todos os objetos da árvore de uma vez
        minhaCena.draw(sceneState.wireframeMode ? unshadedShader : nossoShader, view);

        // --- RENDERIZAÇÃO DO SKYBOX ---
        // 1. Muda a matemática de profundidade para aceitar pixels com Z = 1.0 (O fundo absoluto)
        if (!sceneState.wireframeMode) {
            glDepthFunc(GL_LEQUAL);  
            skyboxShader.use();
        
            // Isso prende o cubo gigante exatamente na cabeça da câmera. Você pode girar para olhar, 
            // mas se andar para frente, o céu "anda" junto com você, parecendo infinito.
            glm::mat4 viewSkybox = glm::mat4(glm::mat3(view)); 
            
            skyboxShader.setMat4("view", viewSkybox); // Passa a matriz sem posição!
            skyboxShader.setMat4("projection", projection);

            // 3. Desenha o Cubo
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);

        }

        // 4. Devolve o OpenGL para o comportamento de profundidade normal
        glDepthFunc(GL_LESS);

        // ---------------------------------------------------------------------
        // FASE 3: DESENHAR OS ÍCONES DE LÂMPADA (Billboards Transparentes)
        // ---------------------------------------------------------------------
        // Desenhamos por último para a transparência do .png não "furar" o cenário 3D!
        if (!sceneState.wireframeMode) {
            glActiveTexture(GL_TEXTURE0);
            nossoShader.use();
            nossoShader.setMat4("projection", projection);
            nossoShader.setMat4("view", view);
            nossoShader.setBool("uAffectedByLight", false); 
            nossoShader.setBool("uHasTexture", true); // <-- OBRIGATÓRIO AQUI!
            
            glBindTexture(GL_TEXTURE_2D, lightIconTexture);

            for (auto luz : listaDeLuzes) {
                glm::mat4 iconModel = glm::mat4(1.0f);
                iconModel = glm::translate(iconModel, luz->position);
                
                // Matemática do Billboard (Encarar a câmara)
                iconModel[0][0] = view[0][0]; iconModel[0][1] = view[1][0]; iconModel[0][2] = view[2][0];
                iconModel[1][0] = view[0][1]; iconModel[1][1] = view[1][1]; iconModel[1][2] = view[2][1];
                iconModel[2][0] = view[0][2]; iconModel[2][1] = view[1][2]; iconModel[2][2] = view[2][2];
                
                iconModel = glm::rotate(iconModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                iconModel = glm::scale(iconModel, glm::vec3(0.025f)); // Ajuste este valor se o ícone ficar muito grande
                
                nossoShader.setMat4("model", iconModel);
                
                // Pinta o ícone .png usando a cor escolhida para a luz no Inspetor!
                glUniform4f(glGetUniformLocation(nossoShader.ID, "uBaseColor"), luz->lightColor.r, luz->lightColor.g, luz->lightColor.b, 1.0f);
                
                groundPlane.draw(); 
            }
        }

        // Reseta o modo de desenho para Fill antes de desenhar a UI (O ImGui precisa disso)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 

        uiManager.endFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}