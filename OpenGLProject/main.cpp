#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <iostream>
#include <gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <string>
#define M_PI 3.14
// Vertex shader for the geometry pass
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

vec3 calculateFog(vec3 objectColor)
{
    float fogDistance = length(FragPos);
	float fogFactor = exp(- fogDensity  * fogDistance);
    vec3 fogColor = vec3(0.6, 0.6, 0.6);
    return fogFactor * objectColor + (1 - fogFactor) * fogColor;
}

void main()
{
    gPosition = FragPos;
    gNormal = normalize(Normal);
    gAlbedo = objColor;
    if (isFog)
		gAlbedo = calculateFog(gAlbedo);

}
)";

// Vertex shader for the lighting pass
const char* lightingVS = R"(
#version 330 core
layout(location = 0) in vec2 aPos;

out vec2 TexCoords;

void main()
{
    TexCoords = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

// Fragment shader for the lighting pass
const char* lightingFS = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

struct Light {
    vec3 position;
    vec3 color;
    vec3 direction;
    int type;
    // type 0 = point light
    // type 1 = directional light
    // type 2 = spot light
};


uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform Light lights[4];

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform bool isDayLight;


float CalculateDistance(vec3 lightPos, vec3 FragPos)
{
	return length(lightPos - FragPos);
}
    float attenuation(float distance)
    {
        float constant = 1.0;
		float linear = 0.09;
		float quadratic = 0.032;
		return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
	}

vec3 CalculateDiffuse(vec3 Albedo,vec3 Normal, Light light, vec3 lightDir)
{

    float diff = max(dot(Normal, lightDir), 0.0);
    return diff * light.color * Albedo;
}
vec3 CalculateSpecular(vec3 FragPos, vec3 Normal, Light light, vec3 lightDir)
{
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, Normal);
   
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    return spec  * light.color;
}
vec3 CalculateAmbient(vec3 Albedo)
{
    if (isDayLight)
	    return 0.2f * Albedo;
    else
	    return 0.08f * Albedo;
}

 vec3 calculateLight(Light light, vec3 Albedo, vec3 Normal, vec3 FragPos)
 {
        vec3 lightDir = normalize(light.position - FragPos);

        float dist = length(light.position - FragPos);
		float att = attenuation(dist);
        
        vec3 diffuse = CalculateDiffuse(Albedo, Normal, light, lightDir) * att;
   
        vec3 specular = CalculateSpecular(FragPos, Normal, light, lightDir) * att;

        

        return (diffuse + specular) ;
 }
 vec3 calculateDirectionalLight(Light light, vec3 Albedo, vec3 Normal, vec3 FragPos) {

		vec3 diffuse = CalculateDiffuse(Albedo, Normal, light, light.direction);
   
        vec3 specular = CalculateSpecular(FragPos, Normal, light, light.direction);

        return (diffuse + specular) ;
       
	}
vec3 calculateSpotLight(Light light, vec3 Albedo, vec3 Normal, vec3 FragPos) {
        vec3 lightDir = normalize(light.position - FragPos);
        float dist = length(light.position - FragPos);
		float att = attenuation(dist);
		float theta = dot(lightDir, normalize(-light.direction));
        float cutOff = 0.91;
		float outerCutOff = 0.82;
		float epsilon   = cutOff - outerCutOff;
        float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
        
        vec3 diffuse = CalculateDiffuse(Albedo, Normal, light, lightDir) * intensity;
   
        vec3 specular = CalculateSpecular(FragPos, Normal, light, lightDir) * intensity;

        vec3 result = vec3(0.0);
		if (theta > cutOff)
			result = result + diffuse + specular;

        return result;
	}

void main()
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;

    // Ambient
    
    vec3 ambient = CalculateAmbient(Albedo);

    vec3 lightsColors; 
    for (int i = 0; i < 4; i++)
	{
		if (lights[i].type == 0)
			lightsColors = lightsColors + calculateLight(lights[i], Albedo, Normal, FragPos);
		else if (lights[i].type == 1 && isDayLight)
			lightsColors = lightsColors + calculateDirectionalLight(lights[i], Albedo, Normal, FragPos);
		else if (lights[i].type == 2)
			lightsColors = lightsColors + calculateSpotLight(lights[i], Albedo, Normal, FragPos);
	}
    

    FragColor = vec4(ambient + lightsColors, 1.0);

}
)";
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
struct Weather
{
    bool isDayLight;
    bool isFog;
    float fogDensity;
};
struct Light {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 direction;
    int type;
    // type 0 = point light
    // type 1 = directional light
    // type 2 = spot light
};
struct Object
{
    glm::vec3 color;
    glm::vec3 position;
    glm::vec3 rotation;
	float scale;
};
// Quad vertices for the lighting pass
float quadVertices[] = {
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f
};
struct Camera
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
};
unsigned int quadIndices[] = {
    0, 1, 2,
    0, 2, 3
};

// Cube vertices
float cubeVertices[] = {
    // Positions          // Normals
   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

unsigned int cubeIndices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20
};
void createSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, float radius, unsigned int sectors, unsigned int stacks)
{
    vertices.clear();
    indices.clear();

    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;

    for (unsigned int i = 0; i <= stacks; ++i) {
        float stackAngle = M_PI / 2 - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            // Vertex position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal vector (normalized)
            float length = sqrt(x * x + y * y + z * z);
            vertices.push_back(x / length);
            vertices.push_back(y / length);
            vertices.push_back(z / length);
        }
    }

    for (unsigned int i = 0; i < stacks; ++i) {
        unsigned int k1 = i * (sectors + 1);
        unsigned int k2 = k1 + sectors + 1;

        for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

// Function to compile shaders
unsigned int compileShader(const char* source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }

    return shader;
}

// Function to link shaders into a program
unsigned int createShaderProgram(const char* vsSource, const char* fsSource) {
    unsigned int vs = compileShader(vsSource, GL_VERTEX_SHADER);
    unsigned int fs = compileShader(fsSource, GL_FRAGMENT_SHADER);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}


Gbuffer SetUpGbuffer()
{
    Gbuffer gBuffer;
  //  unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer.buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.buffer);

    unsigned int gPosition, gNormal, gAlbedo; 
    glGenTextures(1, &gBuffer.gPosition);  
    glGenTextures(1, &gBuffer.gNormal); 
    glGenTextures(1, &gBuffer.gAlbedo); 

    glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 800, 600, 0, GL_RGB, GL_FLOAT, nullptr); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.gPosition, 0); 

    glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 800, 600, 0, GL_RGB, GL_FLOAT, nullptr); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.gNormal, 0); 

    glBindTexture(GL_TEXTURE_2D, gBuffer.gAlbedo); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.gAlbedo, 0); 

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 }; 
    glDrawBuffers(3, attachments); 

    unsigned int rboDepth; 
    glGenRenderbuffers(1, &rboDepth); 
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 800, 600);  
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth); 
    return gBuffer;
}


VAOStruct SetUpCubeVAO()
{
    VAOStruct vStruct;
  
    glGenVertexArrays(1, &vStruct.VAO);
    glGenBuffers(1, &vStruct.VBO);
    glGenBuffers(1, &vStruct.EBO);

    glBindVertexArray(vStruct.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vStruct.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    return vStruct;
}

VAOStruct SetUpSphereVAO(std::vector<float> verticesS, std::vector<unsigned int> indicesS)
{
    // Create and bind VAOs and 
    VAOStruct vStruct;
    glGenVertexArrays(1, &vStruct.VAO);
    glGenBuffers(1, &vStruct.VBO);
    glGenBuffers(1, &vStruct.EBO);

    glBindVertexArray(vStruct.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, vStruct.VBO);
    glBufferData(GL_ARRAY_BUFFER, verticesS.size() * sizeof(float), verticesS.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vStruct.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesS.size() * sizeof(unsigned int), indicesS.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    return vStruct;
}
VAOStruct SetUpQuad()
{
    VAOStruct vStruct;

    glGenVertexArrays(1, &vStruct.VAO); 
    glGenBuffers(1, &vStruct.VBO); 
    glGenBuffers(1, &vStruct.EBO); 
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(vStruct.VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, vStruct.VBO); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vStruct.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);

    return vStruct;
}
void GeometryPassCube(VAOStruct buffers, GLuint shaderProgram, Object cube, Weather weather, Gbuffer gBuffer, float time, Camera camera)
{
    
    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);
    
    model = glm::translate(model, cube.position);
    if (glm::length(cube.rotation) > 0.0f)
        model = glm::scale(model, glm::vec3(cube.scale));
    model = glm::rotate(model, time * 0.5f, cube.rotation);

    glm::mat4 view = glm::lookAt(camera.position, camera.direction, camera.up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUniform1i(glGetUniformLocation(shaderProgram, "isFog"), weather.isFog);
    glUniform1f(glGetUniformLocation(shaderProgram, "fogDensity"), weather.fogDensity);
	glUniform1i(glGetUniformLocation(shaderProgram, "isDayLight"), weather.isDayLight);

    glBindVertexArray(buffers.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void GeometryPassSphere(VAOStruct buffers, GLuint shaderProgram, Object sphere, Weather weather, Gbuffer gBuffer, float time, std::vector<unsigned int>& indices, Camera camera)
{

    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, sphere.position);
    model = glm::scale(model, glm::vec3(sphere.scale));
	if (glm::length(sphere.rotation) > 0.0f)
        model = glm::rotate(model, time * 0.5f, sphere.rotation);

    glm::mat4 view = glm::lookAt(camera.position, camera.direction, camera.up); 
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUniform1i(glGetUniformLocation(shaderProgram, "isFog"), weather.isFog);
    glUniform1f(glGetUniformLocation(shaderProgram, "fogDensity"), weather.fogDensity);
    glUniform1i(glGetUniformLocation(shaderProgram, "isDayLight"), weather.isDayLight);

    glUniform3fv(glGetUniformLocation(shaderProgram, "objColor"), 1, glm::value_ptr(sphere.color));

    glBindVertexArray(buffers.VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}

void LightingPassCube(VAOStruct buffers, GLuint shaderProgram, Gbuffer gBuffer, Light* lights, Weather weather) 
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer.gAlbedo);

    glUniform1i(glGetUniformLocation(shaderProgram, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "gAlbedo"), 2);
    std::string uniform;
	for (int i = 0; i < 4; i++)
    {
		uniform = "lights[" + std::to_string(i) + "].position";
        glUniform3fv(glGetUniformLocation(shaderProgram, uniform.c_str()), 1, glm::value_ptr(lights[i].position));
		uniform = "lights[" + std::to_string(i) + "].direction";
		glUniform3fv(glGetUniformLocation(shaderProgram, uniform.c_str()), 1, glm::value_ptr(lights[i].direction));
		uniform = "lights[" + std::to_string(i) + "].color";
		glUniform3fv(glGetUniformLocation(shaderProgram, uniform.c_str()), 1, glm::value_ptr(lights[i].color));
		uniform = "lights[" + std::to_string(i) + "].type";
		glUniform1i(glGetUniformLocation(shaderProgram, uniform.c_str()), lights[i].type);
	}   

    glUniform1i(glGetUniformLocation(shaderProgram, "isDayLight"), weather.isDayLight);

    glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 2.0f, 2.0f, 2.0f);

    glBindVertexArray(buffers.VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
Light* CreateLights()
{
    Light* lights = new Light[4];

    lights[0].position = glm::vec3(0.0f, 0.0f, 1.0f);
    lights[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
    lights[0].type = 0;

    lights[1].position = glm::vec3(0.0f, 1.0f, 0.0f);
    lights[1].color = glm::vec3(1.0f, 1.0f, 1.0f);
    lights[1].type = 0;

    lights[2].direction = glm::vec3(0.0f, 0.0f, 1.0f);
    lights[2].color = glm::vec3(1.0f, 1.0f, 1.0f);
    lights[2].type = 1;

    lights[3].position = glm::vec3(0.0f, 0.0f, 0.7f);
    lights[3].direction = glm::vec3(0.0f, 0.0f, -1.0f);
    lights[3].color = glm::vec3(1.0f, 1.0f, 1.0f);
    lights[3].type = 2;

    return lights;

}
Object* CreateCubes()
{
    Object* cubes = new Object[12];

    cubes[0].color = glm::vec3(0.0f, 1.0f, 0.0f);
    cubes[1].color = glm::vec3(0.0f, 1.0f, 0.0f);
    cubes[2].color = glm::vec3(0.0f, 1.0f, 0.0f);
    cubes[3].color = glm::vec3(0.0f, 1.0f, 0.0f);
    cubes[4].color = glm::vec3(0.0f, 1.0f, 0.0f);
    cubes[5].color = glm::vec3(0.0f, 1.0f, 0.0f);
    cubes[6].color = glm::vec3(0.0f, 1.0f, 0.0f);
    cubes[7].color = glm::vec3(0.0f, 1.0f, 0.0f);
    cubes[8].color = glm::vec3(0.0f, 1.0f, 0.0f);
    cubes[9].color = glm::vec3(0.0f, 1.0f, 0.0f);
    cubes[10].color = glm::vec3(0.0f, 1.0f, 0.0f);
    cubes[11].color = glm::vec3(0.0f, 1.0f, 0.0f);
    
    cubes[0].position = glm::vec3(0.3f, 0.0f, 0.0f);
    cubes[1].position = glm::vec3(1.0, 0.3f, -1.0f);
    cubes[2].position = glm::vec3(0.7, -0.3f, -0.1f);
    cubes[3].position = glm::vec3(0.7, 0.3f, 0.6f);
    cubes[4].position = glm::vec3(-0.5, 0.3f, -0.2f);
    cubes[5].position = glm::vec3(0.0, 0.3f, 0.2f);
    cubes[6].position = glm::vec3(0.2, 0.3f, 0.0f);
    cubes[7].position = glm::vec3(1.0, 0.3f, 0.0f);
    cubes[8].position = glm::vec3(-1.0, 0.3f, 0.3f);
    cubes[9].position = glm::vec3(0.0, 0.3f, 0.2f);
    cubes[10].position = glm::vec3(0.0, -1.3f, 0.5f);
    cubes[11].position = glm::vec3(0.0, 2.3f, 0.7f);

    cubes[1].rotation = glm::vec3(0.1f, 0.1f, 0.1f);
    cubes[0].rotation = glm::vec3(0.1f, 0.1f, 0.1f);
    cubes[2].rotation = glm::vec3(0.1f, 0.1f, 0.1f);
    cubes[3].rotation = glm::vec3(0.1f, 0.1f, 0.1f);
    cubes[4].rotation = glm::vec3(0.1f, 0.1f, 0.1f);
    cubes[5].rotation = glm::vec3(0.1f, 0.1f, 0.1f);
    cubes[6].rotation = glm::vec3(0.1f, 0.1f, 0.1f);
    cubes[7].rotation = glm::vec3(0.1f, 0.1f, 0.1f);
    cubes[8].rotation = glm::vec3(0.1f, 0.1f, 0.1f);
    cubes[9].rotation = glm::vec3(0.1f, 0.1f, 0.1f);
    cubes[10].rotation = glm::vec3(0.1f, 0.1f, 0.1f);
    cubes[11].rotation = glm::vec3(0.1f, 0.1f, 0.1f);

	for (int i = 0; i < 12; i++)
		cubes[i].scale = 0.1f;

	return cubes;
}
Object* CreateSpheres()
{
    Object* spheres = new Object[3];

    spheres[0].color = glm::vec3(0.0f, 1.0f, 0.0f);

    spheres[0].position = glm::vec3(-0.1f, -0.1f, 0.0f);

    spheres[0].rotation = glm::vec3(0.1f, 0.1f, 0.1f);

	spheres[0].scale = 0.4f;

    spheres[1].color = glm::vec3(0.0f, 1.0f, 0.0f);

    spheres[1].position = glm::vec3(-0.1f, -0.1f, 0.0f);

    spheres[1].rotation = glm::vec3(0.1f, 0.1f, 0.1f);

    spheres[1].scale = 0.07f;

    spheres[2].color = glm::vec3(0.0f, 0.0f, 1.0f);

    spheres[2].position = glm::vec3(0.0f, 0.0f, 0.0f);

    spheres[2].rotation = glm::vec3(0.0f, 0.0f, 0.0f);

    spheres[2].scale = 6.0f;

    return spheres;
}
Camera* CreateCameras()
{
    Camera* cameras = new Camera[4];

    cameras[0].position = glm::vec3(0.0f, 0.0f, 1.0f);
    cameras[1].position = glm::vec3(0.0f, 1.0f, 0.0f);
    cameras[2].position = glm::vec3(1.0f, 0.0f, 0.0f);
    cameras[3].position = glm::vec3(0.0f, 0.0f, 2.0f);

    cameras[0].direction = glm::vec3(0.1f);
    cameras[1].direction = glm::vec3(0.0f);
    cameras[2].direction = glm::vec3(0.0f);
    cameras[3].direction = glm::vec3(0.0f);

    cameras[0].up = glm::vec3(0.0f, 0.0f, 1.0f);
    cameras[1].up = glm::vec3(0.0f, 0.0f, 1.0f);
    cameras[2].up = glm::vec3(0.0f, 0.0f, 1.0f);
    cameras[2].up = glm::vec3(0.0f, 1.0f, 0.0f);

    return cameras;
}
float CalculateFogDensity(float time)
{
	return 0.5 * sin(time * 0.5) + 0.5;
}
int main() {
    // Initialize GLFW and create a window
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Deferred Shading", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    Gbuffer gBuffer;
    gBuffer = SetUpGbuffer();


    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
        return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::vector<float> verticesS;
    std::vector<unsigned int> indicesS;
    createSphere(verticesS, indicesS, 0.33f, 32, 16);

    // Set up shaders
    unsigned int geometryShader = createShaderProgram(geometryVS, geometryFS);
    unsigned int lightingShader = createShaderProgram(lightingVS, lightingFS);

    // Set up cube VAO
    VAOStruct cubeVAOs = SetUpCubeVAO();

    // Set up Sphere VAO
    VAOStruct SphereVAO = SetUpSphereVAO(verticesS, indicesS);
    // Set up quad VAO
    VAOStruct quadVAOs = SetUpQuad();

    // set up other 
    Weather weather;
	weather.fogDensity = 0.8f;
	weather.isFog = false;
    weather.isDayLight = false;
    unsigned int currentCamera = 0;
    Light* lights = CreateLights(); 
    Object* cubes = CreateCubes();
    Object* spheres = CreateSpheres();
    Camera* cameras = CreateCameras();
    Object cube;
    Object cube1;
    cube.position = glm::vec3(0, 0.0f, -0.2f);
    cube1.position = glm::vec3(0.0, 0.3f, -0.0f);
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.buffer); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

       float time = glfwGetTime();
	   float radius = 0.5f;
	   weather.fogDensity = CalculateFogDensity(time);

	   glm::vec3 prevPos = spheres[1].position;
	   spheres[1].position = glm::vec3(sin(time) * radius,cos(time) * radius, 0.3f);
	   glm::vec3 diff = spheres[1].position - prevPos;
	   cameras[2].position = 3.0f * spheres[1].position;
	   cameras[1].direction = spheres[1].position;
	   lights[3].position = spheres[1].position;
        // Geometry pass
	   for (int i = 0; i < 12; i++)
		   GeometryPassCube(cubeVAOs, geometryShader, cubes[i], weather, gBuffer, time, cameras[currentCamera]);
	   for (int i = 0; i < 3; i++)
           GeometryPassSphere(SphereVAO, geometryShader, spheres[i], weather, gBuffer, time, indicesS, cameras[currentCamera]);

        // Lighting pass
        LightingPassCube(quadVAOs, lightingShader, gBuffer, lights, weather);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            currentCamera = 0;

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            currentCamera = 1;

        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            currentCamera = 2;

        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
            currentCamera = 3;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            weather.isDayLight = true;
		if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
			weather.isDayLight = false;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
			weather.isFog = true;
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
			weather.isFog = false;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			lights[3].direction.y += 0.01f;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			lights[3].direction.y -= 0.01f;
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			lights[3].direction.x -= 0.01f;
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			lights[3].direction.x += 0.01f;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			lights[3].direction.z += 0.01f;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			lights[3].direction.z -= 0.01f;
    }

    // Clean up
    glDeleteVertexArrays(1, &cubeVAOs.VAO);
    glDeleteBuffers(1, &cubeVAOs.VBO);
    glDeleteBuffers(1, &cubeVAOs.EBO);

    glDeleteVertexArrays(1, &quadVAOs.VAO);
    glDeleteBuffers(1, &quadVAOs.VBO); 
    glDeleteBuffers(1, &quadVAOs.EBO); 

    glDeleteFramebuffers(1, &gBuffer.buffer);
    glDeleteTextures(1, &gBuffer.gPosition);
    glDeleteTextures(1, &gBuffer.gNormal);
    glDeleteTextures(1, &gBuffer.gAlbedo);

    glDeleteProgram(geometryShader);
    glDeleteProgram(lightingShader);

    glfwTerminate();
    return 0;
}