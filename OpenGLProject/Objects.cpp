#include "Objects.hpp"





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

        unsigned int k3 = k1;
        unsigned int k4 = k2;
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
        if (i != 0) {
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k3);
        }
        if (i != (stacks - 1)) {
            indices.push_back(k3);
            indices.push_back(k2);
            indices.push_back(k4);
        }
    }
}
Light* CreateLights()
{
    Light* lights = new Light[6];

    lights[0].position = glm::vec3(0.0f, 0.0f, 0.3f);
    lights[0].color = glm::vec3(0.0f, 0.0f, 1.0f);
    lights[0].type = 0;

    lights[1].position = glm::vec3(0.0f, 2.0f, 0.2f);
    lights[1].color = glm::vec3(1.0f, 0.0f, 0.0f);
    lights[1].type = 0;

    lights[2].direction = glm::vec3(0.0f, 0.0f, 1.0f);
    lights[2].color = glm::vec3(1.0f, 1.0f, 1.0f);
    lights[2].type = 1;

    lights[3].position = glm::vec3(0.0f, 0.0f, 0.7f);
    lights[3].direction = glm::vec3(0.0f, 0.0f, -1.0f);
    lights[3].color = glm::vec3(1.0f, 1.0f, 1.0f);
    lights[3].type = 2;

    lights[4].position = glm::vec3(0.0f, 0.0f, 0.0f);
    lights[4].color = glm::vec3(1.0f, 0.0f, 0.0f);
    lights[4].type = 0;

    lights[5].position = glm::vec3(1.0f, -1.0f, 0.2f);
    lights[5].color = glm::vec3(0.0f, 0.0f, 1.0f);
    lights[5].type = 0;
    return lights;

}

Object* CubesGenerator()
{
    srand(static_cast <unsigned> (time(0)));
    Object* cubes = new Object[100];
    for (int i = 0; i < 100; i++)
    {
        Object cube;
        cube.color = glm::vec3(1.0f, 1.0f, 1.0f);
        float LO = -1.0f;
        float HI = 0.9f;

        float r3 = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
        float r1 = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
        float r2 = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
        cube.position = glm::vec3(r1, r2, r3);
        cube.rotation = glm::vec3(r1, r2, r3);
        cube.scale = 0.1f;
        cubes[i] = cube;
    }
    return cubes;
}

Object* CreateCubes()
{
    Object* cubes = new Object[12];

    cubes[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
    cubes[1].color = glm::vec3(1.0f, 1.0f, 1.0f);
    cubes[2].color = glm::vec3(1.0f, 1.0f, 1.0f);
    cubes[3].color = glm::vec3(1.0f, 1.0f, 1.0f);
    cubes[4].color = glm::vec3(1.0f, 1.0f, 1.0f);
    cubes[5].color = glm::vec3(1.0f, 1.0f, 1.0f);
    cubes[6].color = glm::vec3(1.0f, 1.0f, 1.0f);
    cubes[7].color = glm::vec3(1.0f, 1.0f, 1.0f);
    cubes[8].color = glm::vec3(1.0f, 1.0f, 1.0f);
    cubes[9].color = glm::vec3(1.0f, 1.0f, 1.0f);
    cubes[10].color = glm::vec3(1.0f, 1.0f, 1.0f);
    cubes[11].color = glm::vec3(1.0f, 1.0f, 1.0f);

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

    cameras[0].up = glm::vec3(0.0f, 1.0f, 0.0f);
    cameras[1].up = glm::vec3(0.0f, 1.0f, 0.0f);
    cameras[2].up = glm::vec3(0.0f, 1.0f, 0.0f);
    cameras[3].up = glm::vec3(0.0f, 1.0f, 0.0f);

    return cameras;
}

float CalculateFogDensity(float time)
{
    return 0.5 * sin(time * 0.3) + 0.5;
}