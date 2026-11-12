#ifndef OURPAINT_RENDERER_SHADERS_UTILS_H_
#define OURPAINT_RENDERER_SHADERS_UTILS_H_

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class ShaderUtils {
public:
    static std::string readFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open shader file: " << filepath << std::endl;
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

#endif // ! OURPAINT_RENDERER_SHADERS_UTILS_H_