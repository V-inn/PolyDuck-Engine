#version 330 core
out vec4 FragColor;

// Entradas recebidas do Vertex Shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// Variáveis Uniformes vindas do C++
uniform sampler2D ourTexture;
uniform vec3 lightPos;   // Onde está a lâmpada?
uniform vec3 viewPos;    // Onde está a câmera?
uniform vec3 lightColor; // Qual a cor da luz? (ex: 1.0, 1.0, 1.0 para branco)

void main()
{
    // Forças da iluminação (você pode transformar isso em uniforms depois!)
    float ambientStrength = 0.1;
    float specularStrength = 0.5;
    float shininess = 32.0; // O quão "focado" é o reflexo (quanto maior, mais metálico/plástico)

    // 1. LUZ AMBIENTE (Ambient)
    // Garante que o objeto não fique 100% preto nas sombras
    vec3 ambient = ambientStrength * lightColor;

    // 2. LUZ DIFUSA (Diffuse)
    // Calcula o impacto direto da luz na superfície
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // O Dot Product retorna 1 se a luz bate de frente, e 0 se bate de lado.
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 3. LUZ ESPECULAR (Specular)
    // Calcula o brilho refletido em direção ao olho do jogador
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;  

    // RESULTADO FINAL
    // Somamos as três luzes e multiplicamos pela cor da textura
    vec4 texColor = texture(ourTexture, TexCoord);
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    
    FragColor = vec4(result, texColor.a);
}