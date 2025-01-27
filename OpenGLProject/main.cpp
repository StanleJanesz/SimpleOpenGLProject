#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>



struct Light {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 direction;
    int type;
    // type 0 = point light
    // type 1 = directional light
    // type 2 = spot light
};


// Vertex shader
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    
    out vec3 FragPos;
    out vec3 Normal;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    void main() {
        FragPos = vec3(view * model * vec4(aPos, 1.0));
        Normal =  mat3(transpose(inverse(view * model))) * aNormal;
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

// Fragment shader
const char* fragmentShaderSource = R"(
    #version 330 core

    out vec4 FragColor;
    
    in vec3 FragPos;
    in vec3 Normal;
    

struct Light {
    vec3 position;
    vec3 color;
    vec3 direction;
    int type;
    // type 0 = point light
    // type 1 = directional light
    // type 2 = spot light
};

    uniform vec3 viewPos;
    uniform bool isDayLight;
    uniform Light lights[4];
	uniform bool isFog;
    uniform float fogDensity;
    uniform mat4 model;
    vec3 calculateAmbient(vec3 lightColor)
    {
        float ambientStrength = 0.3;
        return ambientStrength * lightColor;
    }
    vec3 calculateDiffuse(vec3 lightColor,vec3 lightDir)
    {
        vec3 norm = normalize(Normal);
        
        float diff = max(dot(norm, lightDir), 0.0);
        return diff * lightColor;
    }
    vec3 calculateSpecular(vec3 lightColor,vec3 lightDir)
    {   
        float specularStrength = 0.7;
        vec3 viewDir = normalize(- FragPos);
        vec3 norm = normalize(Normal);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
        return specularStrength * spec * lightColor;
    }
    float distance(vec3 lightPos)
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
      vec3 calculateFog(vec3 objectColor)
	{
		float fogDistance = length(FragPos);
		float fogFactor = exp(- fogDensity  * fogDistance);
        vec3 fogColor = vec3(0.6, 0.6, 0.6);
        return fogFactor * objectColor + (1 - fogFactor) * fogColor;
	}
    vec3 calculateLight(vec3 lightPos, vec3 lightColor) {

       vec3 lightDir = normalize(lightPos - FragPos);
        float dist = distance(lightPos);
		float att = attenuation(dist);
        vec3 ambient = calculateAmbient(lightColor) * att;
        
        vec3 diffuse = calculateDiffuse(lightColor, lightDir) * att;
   
        vec3 specular = calculateSpecular(lightColor, lightDir) * att;

        

        return (ambient + diffuse + specular) ;
    }
   

 vec3 calculateDirectionalLight(vec3 lightDir, vec3 lightColor) {

		vec3 ambient = calculateAmbient(lightColor);
        
        vec3 diffuse = calculateDiffuse(lightColor, -lightDir);
   
        vec3 specular = calculateSpecular(lightColor, -lightDir);
        
		return (ambient + diffuse + specular);
	}
vec3 calculateSpotLight(vec3 lightPos, vec3 lightColor, vec3 spotLightDirection) {
        vec3 lightDir = normalize(lightPos - FragPos);
        float dist = distance(lightPos);
		float att = attenuation(dist);
		float theta = dot(lightDir, normalize(-spotLightDirection));
        float cutOff = 0.91;
		float outerCutOff = 0.82;
		float epsilon   = cutOff - outerCutOff;
        float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
        
        vec3 ambient = calculateAmbient(lightColor) ;
        
        vec3 diffuse = calculateDiffuse(lightColor, lightDir)  * intensity;
   
        vec3 specular = calculateSpecular(lightColor, lightDir)  * intensity;
        vec3 result = ambient;
		if (theta > cutOff)
			result = result + diffuse + specular;

        return result;
	}
    void main() {
        
        vec3 result1 = calculateLight(lights[1].position, lights[1].color);
        vec3 result2 = calculateLight(lights[0].position, lights[0].color);
		vec3 result3 = calculateDirectionalLight(lights[2].direction, lights[2].color);
        vec3 result4 = calculateSpotLight(lights[3].position, lights[3].color, lights[3].direction);

        vec3 objectColor = vec3(0.2, 0.5, 0.8);  // Blue-ish color
        if (isFog)
            objectColor = calculateFog(objectColor);
        vec3 result = result1 + result2 + result4;
        if (isDayLight)
            result = result + result3;
        result  = result * objectColor;
        FragColor = vec4(result, 1.0);
    }
)";

// Light source vertex shader
const char* lightVertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

// Light source fragment shader
const char* lightFragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    
    void main() {
        FragColor = vec4(1.0); // White color for light source
    }
)";

const float M_PI = 3.14159265359f;
// Cube vertices with normals
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
float vertices[] = {
    // positions          // normals
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
void drawCube(float time, GLuint shaderProgram, GLuint VAO, Light lights[4], glm::vec3 viewPos, glm::mat4 view, glm::mat4 projection,glm::vec3 position, bool isDayLight,bool isFog, float fogDensity)
{
    // Draw cube
    glUseProgram(shaderProgram);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, time * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glm::vec3 pos1 = glm::vec3(view * glm::vec4(lights[1].position, 1.0));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[1].position"), 1, glm::value_ptr(pos1));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[1].color"), 1, glm::value_ptr(lights[1].color));
    glUniform1i(glGetUniformLocation(shaderProgram, "lights[1].type"), lights[1].type);
    glm::vec3 pos0 = glm::vec3(view * glm::vec4(lights[0].position, 1.0));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[0].position"), 1, glm::value_ptr(pos0));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[0].color"), 1, glm::value_ptr(lights[0].color));
    glUniform1i(glGetUniformLocation(shaderProgram, "lights[0].type"), lights[0].type);

    glUniform1i(glGetUniformLocation(shaderProgram, "isDayLight"), isDayLight);
    glm::vec3 dir2 = glm::vec3(view * glm::vec4(lights[2].position, 1.0));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[2].direction"), 1, glm::value_ptr(dir2));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[2].color"), 1, glm::value_ptr(lights[2].color));
    glUniform1i(glGetUniformLocation(shaderProgram, "lights[2].type"), lights[2].type);

    glm::vec3 pos3 = glm::vec3(view * glm::vec4(lights[3].position, 1.0));
    glm::vec3 dir3 = glm::vec3(view * glm::vec4(lights[3].position, 1.0));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[3].position"), 1, glm::value_ptr(pos3));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[3].direction"), 1, glm::value_ptr(dir3));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[3].color"), 1, glm::value_ptr(lights[3].color));
    glUniform1i(glGetUniformLocation(shaderProgram, "lights[3].type"), lights[3].type);

    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));


    glUniform1i(glGetUniformLocation(shaderProgram, "isFog"), isFog);
    glUniform1f(glGetUniformLocation(shaderProgram, "fogDensity"), fogDensity);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

}
void drawSphere(float time, GLuint shaderProgram, GLuint VAO, Light lights[4], glm::vec3 viewPos, glm::mat4 view, glm::mat4 projection, glm::vec3 position, std::vector<unsigned int> indices,  bool isDayLight, bool isFog, float fogDensity)
{
    // Draw cube
    glUseProgram(shaderProgram);
    glm::mat4 model = glm::mat4(1.0f);
    position.z = sin(time) * 2.0f;
    model = glm::translate(model, position);
    model = glm::rotate(model, time * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glm::vec3 pos1 = view  * glm::vec4(lights[1].position, 1.0);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[1].position"), 1, glm::value_ptr(pos1));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[1].color"), 1, glm::value_ptr(lights[1].color));
    glUniform1i(glGetUniformLocation(shaderProgram, "lights[1].type"), lights[1].type);
	glm::vec3 pos0 = view  * glm::vec4(lights[0].position, 1.0);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[0].position"), 1, glm::value_ptr(pos0));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[0].color"), 1, glm::value_ptr(lights[0].color));
    glUniform1i(glGetUniformLocation(shaderProgram, "lights[0].type"), lights[0].type);

    glUniform1i(glGetUniformLocation(shaderProgram, "isDayLight"), isDayLight);
    glm::vec3 dir2 = view  * glm::vec4(lights[2].direction, 1.0);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[2].direction"), 1, glm::value_ptr(dir2));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[2].color"), 1, glm::value_ptr(lights[2].color));
    glUniform1i(glGetUniformLocation(shaderProgram, "lights[2].type"), lights[2].type);

    glm::vec3 pos3 = view  * glm::vec4(lights[3].position, 1.0);
    glm::vec3 dir3 = view  * glm::vec4(lights[3].direction, 1.0);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[3].position"), 1, glm::value_ptr(pos3));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[3].direction"), 1, glm::value_ptr(dir3));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lights[3].color"), 1, glm::value_ptr(lights[3].color));
    glUniform1i(glGetUniformLocation(shaderProgram, "lights[3].type"), lights[3].type);

    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));


    glUniform1i(glGetUniformLocation(shaderProgram, "isFog"), isFog);
    glUniform1f(glGetUniformLocation(shaderProgram, "fogDensity"), fogDensity);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

}
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Cube with Moving Light", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);

    // Create and compile shaders
    // Cube shader program
    GLuint cubeShaderProgram = glCreateProgram();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glAttachShader(cubeShaderProgram, vertexShader);
    glAttachShader(cubeShaderProgram, fragmentShader);
    glLinkProgram(cubeShaderProgram);

    // Light shader program
    GLuint lightShaderProgram = glCreateProgram();
    GLuint lightVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(lightVertexShader, 1, &lightVertexShaderSource, NULL);
    glCompileShader(lightVertexShader);

    GLuint lightFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(lightFragmentShader, 1, &lightFragmentShaderSource, NULL);
    glCompileShader(lightFragmentShader);

    glAttachShader(lightShaderProgram, lightVertexShader);
    glAttachShader(lightShaderProgram, lightFragmentShader);
    glLinkProgram(lightShaderProgram);

    std::vector<float> verticesS;
    std::vector<unsigned int> indicesS;
    createSphere(verticesS, indicesS, 0.33f, 32, 16);

    

    // Create and bind VAOs and VBOs
    GLuint VAOs, VBOs, EBOs;
    glGenVertexArrays(1, &VAOs);
    glGenBuffers(1, &VBOs);
    glGenBuffers(1, &EBOs);

    glBindVertexArray(VAOs);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs);
    glBufferData(GL_ARRAY_BUFFER, verticesS.size() * sizeof(float), verticesS.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesS.size() * sizeof(unsigned int), indicesS.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);












    // Cube VAO
    GLuint cubeVAO, VBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);



    // Light VAO
    GLuint lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Light VAO1
    GLuint lightVAO1;
    glGenVertexArrays(1, &lightVAO1);
    glBindVertexArray(lightVAO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int cameraNumber = 0;
	Light lights[4];
	glm::vec3 po1 = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 po2 = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 po3 = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 po4 = glm::vec3(0.0f, 0.0f, 0.0f);
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
	bool isDaytLight = false;
	float time = glfwGetTime();
    float radius = 2.0f;
	float fogDensity = 0.2;
	bool isFog = false;
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate light position
        time = glfwGetTime();
		fogDensity = (sin(time/5)+1.0f)/2.0f;
		lights[0].position = glm::vec3(
			sin(time) * radius,
			0.5f,
			cos(time) * radius
		);
        lights[3].direction = glm::vec3(0.0f, sin(time) * radius, -1.0f);

		lights[1].position = glm::vec3(
			0.0f,
			1.0f,
			0.0f
		);
        glm::vec3 viewPos;
        glm::mat4 view;
        glm::mat4 projection;
        // Camera position and matrices
        if (cameraNumber == 0)
        {
            viewPos = glm::vec3(0.0f, 0.0f, 5.0f);
            view = glm::lookAt(viewPos, glm::vec3(0.6f), glm::vec3(0.0f, 0.0f, 1.0f));
            projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        }
        else if (cameraNumber == 1)
        {
            viewPos = glm::vec3(0.0f, 5.0f, 0.0f);
            view = glm::lookAt(lights[0].position * 1.5f, lights[0].position, glm::vec3(0.0f, 0.0f, 1.0f));
            projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        }
        else
        {
            viewPos = glm::vec3(5.0f, 0.0f, 0.0f);
            view = glm::lookAt(viewPos, lights[0].position, glm::vec3(0.0f, 0.0f, 1.0f));
            projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        }
        glm::vec3 position = glm::vec3(1.0f, -1.0f, 1.0f);
        drawSphere(time, cubeShaderProgram, VAOs, lights, viewPos, view, projection, position,indicesS,isDaytLight, isFog, fogDensity); 

        position = glm::vec3(0.0f, 1.0f, 0.0f);
        drawSphere(time, cubeShaderProgram, VAOs, lights, viewPos, view, projection, position,indicesS, isDaytLight, isFog, fogDensity);

        position = glm::vec3(-1.0f, 1.0f, 0.0f);
        drawCube(time, cubeShaderProgram, cubeVAO, lights, viewPos, view, projection, position, isDaytLight, isFog, fogDensity);
        position = glm::vec3(0.0f, 1.0f, 1.0f);
        drawCube(time, cubeShaderProgram, cubeVAO, lights, viewPos, view, projection, position, isDaytLight, isFog, fogDensity);
        position = glm::vec3(1.0f, 1.0f, -0.5f);
        drawCube(time, cubeShaderProgram, cubeVAO, lights, viewPos, view, projection, position, isDaytLight, isFog, fogDensity);
        glUseProgram(lightShaderProgram);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, lights[0].position);
        model = glm::scale(model, glm::vec3(0.01f));

        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // Draw light source 1
        glUseProgram(lightShaderProgram);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lights[1].position);
        model = glm::scale(model, glm::vec3(0.01f));

        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(lightVAO1);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Handle ESC key to close window
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
            cameraNumber = 0;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            cameraNumber = 1;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            cameraNumber = 2;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			isDaytLight = true;
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
            isDaytLight = false;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            isFog = true;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            isFog = false;
    }

    // Cleanup
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(cubeShaderProgram);
    glDeleteProgram(lightShaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(lightVertexShader);
    glDeleteShader(lightFragmentShader);

    glfwTerminate();
    return 0;
}