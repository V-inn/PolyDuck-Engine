#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#pragma once

class TextureLoader {
public:
    // Função estática para podermos chamá-la de qualquer lugar sem instanciar a classe
    static unsigned int loadTexture(char const * path);
};

#endif