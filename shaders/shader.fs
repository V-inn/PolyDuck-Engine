#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// Variáveis de Material
uniform sampler2D texture1;
uniform vec4 uBaseColor;
uniform bool uHasTexture; 
uniform bool uAffectedByLight; 

// Sistema de Múltiplas Luzes
#define MAX_LIGHTS 10
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];
uniform int numLights; 

uniform vec3 viewPos;

void main()
{
    // Lê a textura garantida (vai ser a imagem ou o pixel branco 1x1)
    vec4 texColor = texture(texture1, TexCoord);
    
    // Mistura a textura com a Cor Base do Inspetor
    vec4 baseResult = texColor * uBaseColor;

    // Se ignorar luz (Billboards), devolve a cor pura
    if (!uAffectedByLight) {
        FragColor = baseResult;
        return;
    }

    // --- MATEMÁTICA DE ILUMINAÇÃO (O coração do Shader!) ---
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Luz ambiente fixa (fraquinha, para não ficar 100% preto nas sombras)
    vec3 ambient = 0.1 * vec3(1.0);
    vec3 finalLighting = vec3(0.0);

    // Soma o poder de cada luz que existe na cena
    for(int i = 0; i < numLights; i++) {
        // A. Luz Difusa (Onde a luz bate de frente)
        vec3 lightDir = normalize(lightPos[i] - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor[i];

        // B. Luz Especular (O brilho refletido)
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // 32.0 é o Shininess padrão
        vec3 specular = spec * lightColor[i];

        // C. Atenuação (A luz enfraquece com a distância)
        float distance = length(lightPos[i] - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

        finalLighting += (diffuse + specular) * attenuation;
    }

    // 4. Aplica a iluminação final ao objeto!
    FragColor = vec4(ambient + finalLighting, 1.0) * baseResult;
}