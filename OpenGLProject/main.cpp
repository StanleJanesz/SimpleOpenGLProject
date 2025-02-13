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
#include "ShaderSetUp.hpp"
#include "GeometryShaders.hpp"
#include "LightingShaders.hpp"
// Vertex shader for the geometry pass




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
    Object* cubes = CubesGenerator();
    Object* spheres = CreateSpheres();
    Camera* cameras = CreateCameras();
	float specPower = 32.0f;

	bool isBlinn = false;
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.buffer); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

       float time = glfwGetTime();
	   float radius = 0.5f;
	   weather.fogDensity = CalculateFogDensity(time);

	   spheres[1].position = glm::vec3(sin(time) * radius,cos(time) * radius, 0.3f);
	   cameras[2].position = 3.0f * spheres[1].position;
	   cameras[1].direction = spheres[1].position;
	   lights[3].position = spheres[1].position;
        // Geometry pass
	   for (int i = 0; i < 100; i++)
		   GeometryPassCube(cubeVAOs, geometryShader, cubes[i], weather, gBuffer, time, cameras[currentCamera]);
	   for (int i = 0; i < 3; i++)
           GeometryPassSphere(SphereVAO, geometryShader, spheres[i], weather, gBuffer, time, indicesS, cameras[currentCamera]);

        // Lighting pass
        LightingPassCube(quadVAOs, lightingShader, gBuffer, lights, weather, cameras[currentCamera],specPower, isBlinn);

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
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
			specPower = std::max(specPower - 1.0f, 1.0f);
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            specPower = std::min(specPower + 1.0f, 64.0f);
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
			isBlinn = false;
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
			isBlinn = true;
    }

    // Clean up
    glDeleteVertexArrays(1, &cubeVAOs.VAO);
    glDeleteBuffers(1, &cubeVAOs.VBO);
    glDeleteBuffers(1, &cubeVAOs.EBO);

    glDeleteVertexArrays(1, &SphereVAO.VAO); 
    glDeleteBuffers(1, &SphereVAO.VBO); 
    glDeleteBuffers(1, &SphereVAO.EBO); 

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