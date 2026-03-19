#include "Model.h"
#include <tiny_obj_loader.h>
#include <iostream>
#include <map>
#include <vector>

// Uma estrutura simples para nos ajudar a identificar vértices únicos
struct VertexKey {
    int v, vt, vn;
    
    // O std::map precisa saber como comparar as chaves para organizá-las na memória
    bool operator<(const VertexKey& other) const {
        if (v != other.v) return v < other.v;
        if (vt != other.vt) return vt < other.vt;
        return vn < other.vn;
    }
};

Model::Model(const std::string& path) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // A biblioteca faz o trabalho sujo de ler o arquivo de texto
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        std::cerr << "Erro ao carregar OBJ: " << warn << err << std::endl;
        return;
    }

    std::map<VertexKey, unsigned int> uniqueVertices;
    unsigned int uniqueVertexCount = 0;

    // O arquivo .obj pode ter várias "formas" agrupadas dentro dele
    for (const auto& shape : shapes) {
        // Itera sobre todos os vértices que formam os triângulos
        for (const auto& index : shape.mesh.indices) {
            
            // Cria uma chave para essa combinação exata de Posição, UV e Normal
            VertexKey key = {index.vertex_index, index.texcoord_index, index.normal_index};

            // Se nós ainda não vimos essa combinação...
            if (uniqueVertices.count(key) == 0) {
                // Registra que este será o índice 'uniqueVertexCount'
                uniqueVertices[key] = uniqueVertexCount;
                
                // 1. Extrai a Posição (Obrigatório)
                float vx = attrib.vertices[3 * index.vertex_index + 0];
                float vy = attrib.vertices[3 * index.vertex_index + 1];
                float vz = attrib.vertices[3 * index.vertex_index + 2];

                // 2. Extrai a Normal (Pode não existir no arquivo)
                float nx = 0.0f, ny = 1.0f, nz = 0.0f; 
                if (index.normal_index >= 0) {
                    nx = attrib.normals[3 * index.normal_index + 0];
                    ny = attrib.normals[3 * index.normal_index + 1];
                    nz = attrib.normals[3 * index.normal_index + 2];
                }

                // 3. Extrai o UV (Pode não existir no arquivo)
                float tx = 0.0f, ty = 0.0f;
                if (index.texcoord_index >= 0) {
                    tx = attrib.texcoords[2 * index.texcoord_index + 0];
                    // Atenção: O eixo Y das texturas no OpenGL é invertido em relação ao formato .obj!
                    ty = 1.0f - attrib.texcoords[2 * index.texcoord_index + 1]; 
                }

                // Nossa função auxiliar brilhante cuidando do trabalho de inserir na lista!
                addVertex(vertices, vx, vy, vz, nx, ny, nz, tx, ty);
                
                uniqueVertexCount++;
            }

            // Adiciona o índice (seja ele novo que acabamos de criar, ou um antigo reaproveitado) no EBO
            indices.push_back(uniqueVertices[key]);
        }
    }

    // Passa os arrays finalizados para a classe mãe configurar a Placa de Vídeo!
    setupMesh(vertices, indices);
}