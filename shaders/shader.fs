#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;
uniform bool uHasSpecularMap;

uniform vec4 uBaseColor;
uniform bool uAffectedByLight;

uniform float uSpecularStrength;
uniform float uShininess;

uniform sampler2D normalMap;
uniform bool uHasNormalMap;
in mat3 TBN;

// Sistema de Múltiplas Luzes
#define MAX_LIGHTS 10
uniform vec3 lightPos[MAX_LIGHTS];
uniform vec3 lightColor[MAX_LIGHTS];
uniform int numLights; 

uniform vec3 viewPos;

uniform samplerCube skybox;       // A textura do nosso céu
uniform float uReflectivity;

void main()
{
    vec4 texColor = texture(diffuseMap, TexCoord);
    vec4 baseResult = texColor * uBaseColor;

    if (!uAffectedByLight) {
        FragColor = baseResult;
        return;
    }

    vec3 norm = normalize(Normal); // Padrão
    if (uHasNormalMap) {
        // 1. Lê a cor RGB do mapa (tons de azul/roxo)
        norm = texture(normalMap, TexCoord).rgb;
        // 2. Transforma a cor (0.0 a 1.0) em vetor de direção (-1.0 a 1.0)
        norm = normalize(norm * 2.0 - 1.0);
        // 3. Usa a Matriz TBN para girar esse relevo e alinhá-lo com a face do objeto 3D
        norm = normalize(TBN * norm);
    }

    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 ambient = 0.1 * vec3(1.0);
    
    // Separamos as luzes!
    vec3 totalDiffuse = vec3(0.0);
    vec3 totalSpecular = vec3(0.0);

    for(int i = 0; i < numLights; i++) {
        vec3 lightDir = normalize(lightPos[i] - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor[i];

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess); 
        
        float mapSpecular = 1.0; 
        if (uHasSpecularMap) {
            mapSpecular = texture(specularMap, TexCoord).r;
        }
        
        vec3 specular = uSpecularStrength * mapSpecular * spec * lightColor[i];

        float distance = length(lightPos[i] - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

        totalDiffuse += diffuse * attenuation;
        totalSpecular += specular * attenuation;
    }

    vec3 objectIllumination = (ambient + totalDiffuse) * baseResult.rgb;
    
    // 2. O reflexo especular é adicionado POR CIMA (additive blending)
    vec3 finalColor = objectIllumination + totalSpecular;

    // --- 3. A NOVA MÁGICA DOS REFLEXOS (Environment Mapping) ---
    // O vetor de incidência (I) deve apontar DA câmera PARA o pixel. 
    // Como o seu viewDir aponta DO pixel PARA a câmera, nós apenas invertemos o sinal dele!
    vec3 I = -viewDir;
    
    // Rebatemos o raio usando a normal FINAL (norm), garantindo que 
    // o reflexo entorte se houver um Normal Map ativado!
    vec3 R = reflect(I, norm);
    
    // Lemos a cor do céu na direção em que o raio rebateu
    vec3 reflectionColor = texture(skybox, R).rgb;

    // Misturamos a cor original do objeto com a cor do céu (baseado no % de refletividade)
    finalColor = mix(finalColor, reflectionColor, uReflectivity);

    // Aplica o alpha final
    FragColor = vec4(finalColor, baseResult.a);
}