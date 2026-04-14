#ifndef JSON_HELPERS_H
#define JSON_HELPERS_H

#include "../json.hpp"
#include <glm/glm.hpp>

// Isso diz à biblioteca JSON como converter vetores do OpenGL em texto e vice-versa
namespace glm {
    inline void to_json(nlohmann::json& j, const vec3& v) {
        j = nlohmann::json{{"x", v.x}, {"y", v.y}, {"z", v.z}};
    }

    inline void from_json(const nlohmann::json& j, vec3& v) {
        j.at("x").get_to(v.x);
        j.at("y").get_to(v.y);
        j.at("z").get_to(v.z);
    }

    inline void to_json(nlohmann::json& j, const vec4& v) {
        j = nlohmann::json{{"r", v.r}, {"g", v.g}, {"b", v.b}, {"a", v.a}};
    }

    inline void from_json(const nlohmann::json& j, vec4& v) {
        j.at("r").get_to(v.r);
        j.at("g").get_to(v.g);
        j.at("b").get_to(v.b);
        j.at("a").get_to(v.a);
    }
}

#endif