#version 330 core

// Entradas vindas do C++ (O layout que configuramos no setupMesh)
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// Saídas que serão interpoladas e enviadas para o Fragment Shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

// Matrizes MVP
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 1. Posição final na tela (o padrão que já tínhamos)
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // 2. A Posição do Fragmento no Mundo Real
    // Multiplicamos apenas pela matriz Model (sem View/Projection) 
    // para saber as coordenadas exatas do objeto no cenário 3D.
    FragPos = vec3(model * vec4(aPos, 1.0));

    // 3. A Matemática da Normal (Matriz Inversa Transposta)
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // 4. Repassa o UV
    TexCoord = aTexCoord;
}