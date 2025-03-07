#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class Sphere {
public:
    Sphere(float radius, unsigned int rings, unsigned int sectors);
    void draw();

private:
    unsigned int VAO, VBO;
    std::vector<float> vertices;
    void setupSphere(float radius, unsigned int rings, unsigned int sectors);
};

Sphere::Sphere(float radius, unsigned int rings, unsigned int sectors) {
    setupSphere(radius, rings, sectors);
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Sphere::setupSphere(float radius, unsigned int rings, unsigned int sectors) {
    float const R = 1.0f / (float)(rings - 1);
    float const S = 1.0f / (float)(sectors - 1);
    vertices.reserve(rings * sectors * 5);

    for (unsigned int r = 0; r < rings; r++) {
        for (unsigned int s = 0; s < sectors; s++) {
            float y = sin(-M_PI_2 + M_PI * r * R);
            float x = cos(2 * M_PI * s * S) * cos(M_PI * r * R);
            float z = sin(2 * M_PI * s * S) * cos(M_PI * r * R);

            vertices.push_back(x * radius);
            vertices.push_back(y * radius);
            vertices.push_back(z * radius);
            vertices.push_back(s * S);
            vertices.push_back(r * R);
        }
    }
}

void Sphere::draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size() / 5);
    glBindVertexArray(0);
}