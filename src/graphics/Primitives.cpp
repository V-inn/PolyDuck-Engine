#include "Primitives.h"

const float PI = 3.14159265359f;

void Primitive::setupMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    indexCount = indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Dica de C++ moderno: std::vector::data() retorna o ponteiro direto do array
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    int stride = 8 * sizeof(float); // Agora cada vértice tem 8 floats no total!

    // Layout 0: Posição (3 floats, começa no byte 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Layout 1: Normal (3 floats, começa após os 3 de posição)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Layout 2: Textura UV (2 floats, começa após os 6 anteriores)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Primitive::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Primitive::addVertex(std::vector<float>& vertices, float x, float y, float z, float nx, float ny, float nz, float u, float v) {
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(nx);
    vertices.push_back(ny);
    vertices.push_back(nz);
    vertices.push_back(u);
    vertices.push_back(v);
}

Primitive::~Primitive() {
    // Apaga os buffers da Memória de Vídeo (VRAM)
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

// --- IMPLEMENTAÇÃO DA ESFERA ---
Sphere::Sphere(float radius, int sectorCount, int stackCount) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

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

            // Coordenadas da Normal (NX, NY, NZ) -> É a posição normalizada!
            float invLen = 1.0f / radius;

            // Coordenadas de Textura (u, v)
            float s = (float)j / sectorCount;
            float t = (float)i / stackCount;

            addVertex(vertices, x, y, z, x * invLen, y * invLen, z * invLen, s, t);
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
    
    setupMesh(vertices, indices);
}

Cylinder::Cylinder(float baseRadius, float topRadius, float height, int sectorCount) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float sectorStep = 2 * PI / sectorCount;
    float sectorAngle;

    addVertex(vertices, 0.0f, -height/2.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f); // Centro da base
    addVertex(vertices, 0.0f, height/2.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f); // Centro do topo

    for(int i = 0; i <= sectorCount; ++i){
        sectorAngle = i * sectorStep;
        float x = cosf(sectorAngle);
        float z = sinf(sectorAngle); 

        // A direção normal da parede 
        float nx = x;
        float ny = 0.0f;
        float nz = z;
        float u = (float)i / sectorCount;

        // Borda da Base (1 única linha!)
        addVertex(vertices, baseRadius * x, -height/2.0f, baseRadius * z, nx, ny, nz, u, 0.0f);

        // Borda do Topo (1 única linha!)
        addVertex(vertices, topRadius * x, height/2.0f, topRadius * z, nx, ny, nz, u, 1.0f); 
    }

    for(int i = 0; i < sectorCount; ++i) {
        int k1 = 2 + i * 2;     // Vértice da base
        int k2 = k1 + 1;        // Vértice do topo

        // Base
        indices.push_back(0);   // Centro da base
        indices.push_back(k1);
        indices.push_back(k1 + 2);

        // Topo
        indices.push_back(1);   // Centro do topo
        indices.push_back(k2);
        indices.push_back(k2 + 2);

        // Lateral
        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 2);

        indices.push_back(k1 + 2);
        indices.push_back(k2);
        indices.push_back(k2 + 2);
    }

    setupMesh(vertices, indices);
}

Plane::Plane(float width, float depth, int resX, int resZ, float uvScale) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;
    float stepX = width / resX;
    float stepZ = depth / resZ;

    for(int i = 0; i <= resZ; ++i) {
        for(int j = 0; j <= resX; ++j) {
            float x = -halfWidth + j * stepX;
            float z = -halfDepth + i * stepZ;

            addVertex(vertices, x, 0.0f, z, 0.0f, 1.0f, 0.0f, ((float)j / resX) * uvScale, ((float)i / resZ) * uvScale);
        }
    }

    for(int i = 0; i < resZ; ++i) {
        for(int j = 0; j < resX; ++j) {
            int k1 = i * (resX + 1) + j;
            int k2 = k1 + resX + 1;

            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1);

            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);
        }
    }
    setupMesh(vertices, indices);
}

Box::Box(float width, float height, float length) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Calculamos a metade do tamanho para centralizar a caixa no ponto (0,0,0)
    float w = width / 2.0f;
    float h = height / 2.0f;
    float l = length / 2.0f;

    // Array bruto com as 6 faces. Ordem: X, Y, Z, U, V
    float v[] = {
        // Posições (3)       // Normais (3)      // Texturas (2)
        
        // Face da Frente (+Z) -> Normal aponta para frente (0, 0, 1)
        -w, -h,  l,           0.0f,  0.0f,  1.0f,    0.0f, 0.0f,
         w, -h,  l,           0.0f,  0.0f,  1.0f,    1.0f, 0.0f,
         w,  h,  l,           0.0f,  0.0f,  1.0f,    1.0f, 1.0f,
        -w,  h,  l,           0.0f,  0.0f,  1.0f,    0.0f, 1.0f,

        // Face de Trás (-Z) -> Normal aponta para trás (0, 0, -1)
         w, -h, -l,           0.0f,  0.0f, -1.0f,    0.0f, 0.0f,
        -w, -h, -l,           0.0f,  0.0f, -1.0f,    1.0f, 0.0f,
        -w,  h, -l,           0.0f,  0.0f, -1.0f,    1.0f, 1.0f,
         w,  h, -l,           0.0f,  0.0f, -1.0f,    0.0f, 1.0f,

        // Face Esquerda (-X) -> Normal aponta para esquerda (-1, 0, 0)
        -w, -h, -l,          -1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
        -w, -h,  l,          -1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
        -w,  h,  l,          -1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
        -w,  h, -l,          -1.0f,  0.0f,  0.0f,    0.0f, 1.0f,

        // Face Direita (+X) -> Normal aponta para direita (1, 0, 0)
         w, -h,  l,           1.0f,  0.0f,  0.0f,    0.0f, 0.0f,
         w, -h, -l,           1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
         w,  h, -l,           1.0f,  0.0f,  0.0f,    1.0f, 1.0f,
         w,  h,  l,           1.0f,  0.0f,  0.0f,    0.0f, 1.0f,

        // Face do Topo (+Y) -> Normal aponta para cima (0, 1, 0)
        -w,  h,  l,           0.0f,  1.0f,  0.0f,    0.0f, 0.0f,
         w,  h,  l,           0.0f,  1.0f,  0.0f,    1.0f, 0.0f,
         w,  h, -l,           0.0f,  1.0f,  0.0f,    1.0f, 1.0f,
        -w,  h, -l,           0.0f,  1.0f,  0.0f,    0.0f, 1.0f,

        // Face da Base (-Y) -> Normal aponta para baixo (0, -1, 0)
        -w, -h, -l,           0.0f, -1.0f,  0.0f,    0.0f, 0.0f,
         w, -h, -l,           0.0f, -1.0f,  0.0f,    1.0f, 0.0f,
         w, -h,  l,           0.0f, -1.0f,  0.0f,    1.0f, 1.0f,
        -w, -h,  l,           0.0f, -1.0f,  0.0f,    0.0f, 1.0f
    };

    int numElementos = sizeof(v) / sizeof(float);
    
    for (int i = 0; i < numElementos; ++i) { 
        vertices.push_back(v[i]);
    }

    // Índices desenhando no sentido anti-horário (Counter-Clockwise)
    unsigned int ind[] = {
         0,  1,  2,  2,  3,  0, // Frente
         4,  5,  6,  6,  7,  4, // Trás
         8,  9, 10, 10, 11,  8, // Esquerda
        12, 13, 14, 14, 15, 12, // Direita
        16, 17, 18, 18, 19, 16, // Topo
        20, 21, 22, 22, 23, 20  // Base
    };

    for (int i = 0; i < 36; ++i) {
        indices.push_back(ind[i]);
    }

    // A Mágica da Herança: passa os vetores para a classe mãe configurar a GPU!
    setupMesh(vertices, indices);
}