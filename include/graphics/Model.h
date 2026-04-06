#ifndef MODEL_H
#define MODEL_H

#include "Primitives.h" // Ajuste o caminho se a sua classe Primitive estiver em outra pasta
#include <string>

class Model : public Primitive {
public:
    std::string filePath;

    // O construtor recebe o caminho do arquivo .obj no seu disco
    Model(const std::string& path);
};

#endif