#version 330 core
out vec4 FragColor;

in vec2 TexCoord; // Variável vinda do Vertex Shader

// sampler2D é o tipo de variável GLSL para acessar texturas
uniform sampler2D ourTexture; 

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}