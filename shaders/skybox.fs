#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

// Mudou de samplerCube para sampler2D!
uniform sampler2D skybox; 

// Constantes mágicas da matemática (1/(2*PI) e 1/PI)
const vec2 invAtan = vec2(0.1591, 0.3183);

// Converte o Vetor 3D num UV 2D de um mapa panorâmico
vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    // Normalizamos as coordenadas do cubo para virarem um raio esférico perfeito
    vec2 uv = SampleSphericalMap(normalize(TexCoords));
    
    vec3 color = texture(skybox, uv).rgb;
    FragColor = vec4(color, 1.0);
}