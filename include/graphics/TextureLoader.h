#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <glad/glad.h>
#include <stb_image.h>
#include <vector>
#include <string>
#include <iostream>

unsigned int loadCubemap(std::vector<std::string> faces);

#endif