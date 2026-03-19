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

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Minha Engine 3D", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    // 1. Constrói o nosso Shader Program lendo os arquivos
    Shader nossoShader("shaders/shader.vs", "shaders/shader.fs");

    // Geração e configuração da Textura no OpenGL
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Define como a textura deve se comportar se for esticada ou encolhida
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Carregamento da imagem via CPU
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // Diz ao stb_image para inverter o eixo Y
    unsigned char *data = stbi_load("./assets/earth.jpg", &width, &height, &nrChannels, 0);

    if (data) {
        // Descobre automaticamente se a imagem tem canal Alpha ou não
        GLenum format;
        if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        // Envia os pixels para a Memória da Placa de Vídeo usando o formato correto
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Falha ao carregar a textura" << std::endl;
    }
    stbi_image_free(data); // Libera a memória RAM, pois os dados já estão na GPU


    //Box firstBox(1.0f, 1.0f, 1.0f);
    //Cylinder firstCylinder(0.5f, 0.0f, 1.0f, 36);
    //Sphere firstSphere(0.5f, 10, 5);
    //Plane groundPlane(4.0f, 4.0f, 10, 10);
    Model newModel("./assets/suzanne.obj");

    // Posição estática da câmera (lembra que movemos o mundo -3 no eixo Z? 
    // Logo, a nossa câmera está em +3 no eixo Z do mundo real!)
    glm::vec3 viewPos(0.0f, 0.0f, 3.0f);

    // Cor da luz (Branca pura)
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    // Posição inicial da luz no mundo 3D
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

    // Controle de visualização
    bool wireframeMode = false;

    // Transformações do Objeto
    glm::vec3 objPosition(0.0f, 0.0f, 0.0f);
    glm::vec3 objRotation(0.0f, 0.0f, 0.0f); // Em graus
    glm::vec3 objScale(1.0f, 1.0f, 1.0f);

    glfwSetCursorPosCallback(window, mouse_callback);
    // Esconde o cursor e o "prende" dentro da janela (estilo FPS)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
        uiManager.render(sceneState);

        nossoShader.use();

        // 1. PROJEÇÃO (A Lente Dinâmica que se ajusta ao tamanho da tela)
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        if (height == 0) height = 1; 
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        int projLoc = glGetUniformLocation(nossoShader.ID, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // 2. VISUALIZAÇÃO (A Câmera FPS)
        glm::mat4 view = camera.GetViewMatrix();
        int viewLoc = glGetUniformLocation(nossoShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // 3. MODELO (As Transformações lendo os dados da Interface!)
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sceneState.objPosition);
        model = glm::rotate(model, glm::radians(sceneState.objRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(sceneState.objRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(sceneState.objRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, sceneState.objScale);
        
        int modelLoc = glGetUniformLocation(nossoShader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // 4. LUZ E MATERIAIS (Lendo os dados da Interface!)
        int lightColorLoc = glGetUniformLocation(nossoShader.ID, "lightColor");
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(sceneState.lightColor));

        int lightPosLoc = glGetUniformLocation(nossoShader.ID, "lightPos");
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(sceneState.lightPos));

        int viewPosLoc = glGetUniformLocation(nossoShader.ID, "viewPos");
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera.Position));

        newModel.draw();

        uiManager.endFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}