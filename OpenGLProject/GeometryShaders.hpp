#ifndef GeometryShaders_hpp
#define GeometryShaders_hpp
const char* geometryVS = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

// Fragment shader for the geometry pass
const char* geometryFS = R"(
#version 330 core
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;

in vec3 FragPos;
in vec3 Normal;

uniform bool isFog;
uniform bool isDayLight;
uniform float fogDensity;
uniform vec3 objColor;
uniform mat4 view;



void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);
    gAlbedo = objColor;

}
)";


#endif