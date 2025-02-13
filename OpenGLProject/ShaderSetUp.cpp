#include "ShaderSetUp.hpp"

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

    glUniform3fv(glGetUniformLocation(shaderProgram, "objColor"), 1, glm::value_ptr(cube.color));

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

    glBindVertexArray(0);
}


void LightingPassCube(VAOStruct buffers, GLuint shaderProgram, Gbuffer gBuffer, Light* lights, Weather weather, Camera camera, float specPower, bool isBlinn)
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

    glm::mat4 view = glm::lookAt(camera.position, camera.direction, camera.up);

    glUniform1i(glGetUniformLocation(shaderProgram, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "gAlbedo"), 2);
    std::string uniform;
    for (int i = 0; i < 6; i++)
    {
        uniform = "lights[" + std::to_string(i) + "].position";
        glUniform3fv(glGetUniformLocation(shaderProgram, uniform.c_str()), 1, glm::value_ptr(glm::vec3(view * glm::vec4(lights[i].position, 1.0))));
        uniform = "lights[" + std::to_string(i) + "].direction";
        glUniform3fv(glGetUniformLocation(shaderProgram, uniform.c_str()), 1, glm::value_ptr(glm::vec3(view * glm::vec4(lights[i].direction, 1.0))));
        uniform = "lights[" + std::to_string(i) + "].color";
        glUniform3fv(glGetUniformLocation(shaderProgram, uniform.c_str()), 1, glm::value_ptr(lights[i].color));
        uniform = "lights[" + std::to_string(i) + "].type";
        glUniform1i(glGetUniformLocation(shaderProgram, uniform.c_str()), lights[i].type);
    }
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glUniform1f(glGetUniformLocation(shaderProgram, "specPower"), specPower);

    glUniform1i(glGetUniformLocation(shaderProgram, "isDayLight"), weather.isDayLight);
    glUniform1i(glGetUniformLocation(shaderProgram, "isFog"), weather.isFog);
    glUniform1f(glGetUniformLocation(shaderProgram, "fogDensity"), weather.fogDensity);

    glUniform1i(glGetUniformLocation(shaderProgram, "isBlinn"), isBlinn);


    glBindVertexArray(buffers.VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
