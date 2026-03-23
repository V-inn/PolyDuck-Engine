#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;   // PORTA 3
layout (location = 4) in vec3 aBitangent; // PORTA 4

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out mat3 TBN; // Matriz mágica que alinha a textura com o mundo 3D

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    TexCoord = aTexCoords;
    
    // Normal padrão (para objetos sem Normal Map ou .obj simples)
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // --- CONSTRUÇÃO DA MATRIZ TBN ---
    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    
    // Gram-Schmidt (Garante que os vetores fiquem a exatos 90 graus de diferença)
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T); 
    
    TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}