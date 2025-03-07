#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <learnopengl/shader_m.h>
#include <glm/glm.hpp>

class Pyramid {
public:
    Pyramid() {
        setupPyramid();
    }

    void draw(Shader& shader) {
        shader.use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 18); // 6 faces, 3 vertices each
        glBindVertexArray(0);
    }

private:
    unsigned int VAO, VBO;

    void setupPyramid() {
        float vertices[] = {
            // Base
            -0.5f, 0.0f, -0.5f,
             0.5f, 0.0f, -0.5f,
             0.5f, 0.0f,  0.5f,
            -0.5f, 0.0f,  0.5f,

            // Sides
            -0.5f, 0.0f, -0.5f,
             0.5f, 0.0f, -0.5f,
             0.0f, 0.5f, 0.0f,

             0.5f, 0.0f, -0.5f,
             0.5f, 0.0f,  0.5f,
             0.0f, 0.5f, 0.0f,

             0.5f, 0.0f,  0.5f,
            -0.5f, 0.0f,  0.5f,
             0.0f, 0.5f, 0.0f,

            -0.5f, 0.0f,  0.5f,
            -0.5f, 0.0f, -0.5f,
             0.0f, 0.5f, 0.0f,
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};