#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "graphics/Shader.h"
#include "graphics/Primitives.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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
    //Cylinder firstCylinder(0.5f, 0.5f, 1.0f, 36);
    Sphere firstSphere(0.5f, 10, 5);

    // Posição estática da câmera (lembra que movemos o mundo -3 no eixo Z? 
    // Logo, a nossa câmera está em +3 no eixo Z do mundo real!)
    glm::vec3 viewPos(0.0f, 0.0f, 3.0f);

    // Cor da luz (Branca pura)
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    // Posição inicial da luz no mundo 3D
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

    // Loop de Renderização
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 4. Ativa o shader
        nossoShader.use();

        // --- INÍCIO DA ILUMINAÇÃO ---
        
        // (Opcional) Animando a luz para ela orbitar o seu objeto!
        lightPos.x = -5.0f * sin(glfwGetTime());
        lightPos.z = -5.0f * cos(glfwGetTime());

        // Envia a Posição da Luz
        int lightPosLoc = glGetUniformLocation(nossoShader.ID, "lightPos");
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

        // Envia a Posição da Câmera (necessária para o reflexo Especular)
        int viewPosLoc = glGetUniformLocation(nossoShader.ID, "viewPos");
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

        // Envia a Cor da Luz
        int lightColorLoc = glGetUniformLocation(nossoShader.ID, "lightColor");
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        
        // --- FIM DA ILUMINAÇÃO ---

        // --- INÍCIO DA MATEMÁTICA MVP ---
        
        // 1. MODEL: Usamos glfwGetTime() para pegar o tempo em segundos desde que a janela abriu.
        // Multiplicamos por um ângulo para a velocidade da rotação e escolhemos eixos X e Y.
        glm::mat4 model = glm::mat4(1.0f);
        float timeValue = glfwGetTime();
        model = glm::rotate(model, timeValue * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));

        // 2. VIEW: O OpenGL fica na coordenada (0,0,0) olhando para o eixo Z negativo.
        // Para vermos o objeto, precisamos mover o mundo "para trás" (Z negativo).
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

        // 3. PROJECTION: Cria a perspectiva. FOV de 45 graus, proporção de tela 800/600,
        // e limites de visão entre 0.1 e 100.0 unidades de distância.
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        
        // --- FIM DA MATEMÁTICA MVP ---

        // Envia as matrizes para o Shader
        int modelLoc = glGetUniformLocation(nossoShader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        
        int viewLoc = glGetUniformLocation(nossoShader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        
        int projLoc = glGetUniformLocation(nossoShader.ID, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Desenhamos o objeto chamando o método da classe!
        //firstBox.draw();
        //firstCylinder.draw();
        firstSphere.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}