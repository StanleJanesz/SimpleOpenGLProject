#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cmath>




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
        FragPos = vec3(model * vec4(aPos, 1.0));
        Normal = mat3(transpose(inverse(model))) * aNormal;
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

// Fragment shader
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    
    in vec3 FragPos;
    in vec3 Normal;
    
    uniform vec3 lightPos;
    uniform vec3 lightPos1;
    uniform vec3 lightColor;
    uniform vec3 lightColor1;
    uniform vec3 viewPos;
    
    vec3 calculateAmbient(vec3 lightColor)
    {
        float ambientStrength = 0.3;
        return ambientStrength * lightColor;
    }
    vec3 calculateDiffuse(vec3 lightPos, vec3 lightColor)
    {
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        return diff * lightColor;
    }
    vec3 calculateSpecular(vec3 lightPos, vec3 lightColor)
    {   
        float specularStrength = 0.7;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 norm = normalize(Normal);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
        return specularStrength * spec * lightColor;
    }
    
        
    vec3 calculateLight(vec3 lightPos, vec3 lightColor) {
       
        vec3 ambient = calculateAmbient(lightColor);
        
        vec3 diffuse = calculateDiffuse(lightPos, lightColor);
        
   
        vec3 specular = calculateSpecular(lightPos, lightColor);
        
        return (ambient + diffuse + specular);
    }

    void main() {
        
        vec3 result1 = calculateLight(lightPos1, lightColor1);
        vec3 result2 = calculateLight(lightPos, lightColor);

        vec3 objectColor = vec3(0.2, 0.5, 0.8);  // Blue-ish color
        vec3 result = (result1 + result2) * objectColor;
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
void drawCube(float time, GLuint shaderProgram, GLuint VAO, glm::vec3 lightPos, glm::vec3 lightColor, glm::vec3 lightPos1, glm::vec3 lightColor1,    glm::vec3 viewPos, glm::mat4 view, glm::mat4 projection,glm::vec3 position)
{
    // Draw cube
    glUseProgram(shaderProgram);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, time * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view)); 
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection)); 
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos1"), 1, glm::value_ptr(lightPos1));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor1"), 1, glm::value_ptr(lightColor1));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

}
void drawSphere(float time, GLuint shaderProgram, GLuint VAO, glm::vec3 lightPos, glm::vec3 lightColor, glm::vec3 lightPos1, glm::vec3 lightColor1, glm::vec3 viewPos, glm::mat4 view, glm::mat4 projection, glm::vec3 position, std::vector<unsigned int> indices)
{
    // Draw cube
    glUseProgram(shaderProgram);
    glm::mat4 model = glm::mat4(1.0f);
    position.z = sin(time) * 2.0f;
    model = glm::translate(model, position);
    model = glm::rotate(model, time * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos1"), 1, glm::value_ptr(lightPos1));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor1"), 1, glm::value_ptr(lightColor1));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));

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
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate light position
        float time = glfwGetTime();
        float radius = 2.0f;
        glm::vec3 lightPos(
            sin(time) * radius,
            0.5f,
            cos(time) * radius
        );
        glm::vec3 lightColor(1.0f, 0.0f, 0.0f);

        glm::vec3 lightPos1(
            0.0f,
            1.0f,
            0.0f
        );


        glm::vec3 lightColor1(0.0f, 0.0f, 1.0f);

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
            view = glm::lookAt(lightPos * 1.5f, lightPos, glm::vec3(0.0f, 0.0f, 1.0f));
            projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        }
        else
        {
            viewPos = glm::vec3(5.0f, 0.0f, 0.0f);
            view = glm::lookAt(viewPos, lightPos, glm::vec3(0.0f, 0.0f, 1.0f));
            projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        }
        glm::vec3 position = glm::vec3(1.0f, -1.0f, 1.0f);
        drawSphere(time, cubeShaderProgram, VAOs, lightPos, lightColor, lightPos1, lightColor1, viewPos, view, projection, position,indicesS);

        position = glm::vec3(0.0f, 1.0f, 0.0f);
        drawSphere(time, cubeShaderProgram, VAOs, lightPos, lightColor, lightPos1, lightColor1, viewPos, view, projection, position,indicesS);

        position = glm::vec3(-1.0f, 1.0f, 0.0f);
        drawCube(time, cubeShaderProgram, cubeVAO, lightPos, lightColor, lightPos1, lightColor1, viewPos, view, projection, position);
        position = glm::vec3(0.0f, 1.0f, 1.0f);
        drawCube(time, cubeShaderProgram, cubeVAO, lightPos, lightColor, lightPos1, lightColor1, viewPos, view, projection, position);
        position = glm::vec3(1.0f, 1.0f, -0.5f);
        drawCube(time, cubeShaderProgram, cubeVAO, lightPos, lightColor, lightPos1, lightColor1, viewPos, view, projection, position);
        glUseProgram(lightShaderProgram);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.01f));

        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // Draw light source 1
        glUseProgram(lightShaderProgram);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos1);
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