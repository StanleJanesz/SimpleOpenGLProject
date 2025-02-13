#ifndef ShaderSetUp_hpp
#define ShaderSetUp_hpp
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>
#include <gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <string>
#include "Objects.hpp"


struct Gbuffer
{
    unsigned int buffer;
    unsigned int gPosition;
    unsigned int gNormal;
    unsigned int gAlbedo;
};
struct VAOStruct
{
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
};



// Function to compile shaders
unsigned int compileShader(const char* source, GLenum type);
// Function to link shaders into a program
unsigned int createShaderProgram(const char* vsSource, const char* fsSource);


Gbuffer SetUpGbuffer();


VAOStruct SetUpCubeVAO();

VAOStruct SetUpSphereVAO(std::vector<float> verticesS, std::vector<unsigned int> indicesS);
VAOStruct SetUpQuad();
void GeometryPassCube(VAOStruct buffers, GLuint shaderProgram, Object cube, Weather weather, Gbuffer gBuffer, float time, Camera camera);

void GeometryPassSphere(VAOStruct buffers, GLuint shaderProgram, Object sphere, Weather weather, Gbuffer gBuffer, float time, std::vector<unsigned int>& indices, Camera camera);


void LightingPassCube(VAOStruct buffers, GLuint shaderProgram, Gbuffer gBuffer, Light* lights, Weather weather, Camera camera, float specPower, bool isBlinn);












#endif