#include "Primitives.h"

// --- IMPLEMENTAÇÃO DA ESFERA ---
Sphere::Sphere(float radius, int sectorCount, int stackCount) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    const float PI = 3.14159265359f;
    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    // 1. GERAÇÃO DOS VÉRTICES (Avaliando as Equações Paramétricas)
    for(int i = 0; i <= stackCount; ++i) {
        stackAngle = PI / 2 - i * stackStep;        // phi: de pi/2 a -pi/2
        float xy = radius * cosf(stackAngle);       // R * cos(phi)
        float z = radius * sinf(stackAngle);        // R * sin(phi)

        for(int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;           // theta: de 0 a 2*pi

            // Coordenadas (x, y, z)
            float x = xy * cosf(sectorAngle);       
            float y = xy * sinf(sectorAngle);       
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Coordenadas de Textura (u, v)
            float s = (float)j / sectorCount;
            float t = (float)i / stackCount;
            vertices.push_back(s);
            vertices.push_back(t);
        }
    }

    // 2. GERAÇÃO DOS ÍNDICES (Ligando os pontos para formar triângulos)
    for(int i = 0; i < stackCount; ++i) {
        unsigned int k1 = i * (sectorCount + 1);     // Início da linha atual
        unsigned int k2 = k1 + sectorCount + 1;      // Início da próxima linha

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // Dois triângulos por setor (exceto no primeiro e último stack)
            if(i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if(i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
    
    indexCount = indices.size();

    // 3. CONFIGURAÇÃO DO OPENGL (VAO, VBO e EBO)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO (Vértices)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // EBO (Índices)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Atributo de Posição (3 floats)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atributo de Textura UV (2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Desvincula para segurança
}
void Sphere::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// --- IMPLEMENTAÇÃO DO CILINDRO ---
Cylinder::Cylinder(float baseRadius, float topRadius, float height, int sectorCount) {
    // ... equações paramétricas do cilindro virão aqui ...
}
void Cylinder::draw() const {
    // ...
}

Plane::Plane(float width, float depth, int resX, int resZ) {
    // ... equações paramétricas do plano virão aqui ...
}
void Plane::draw() const {
    // ...
}