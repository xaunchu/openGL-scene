#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h" // 你需要一个Shader类来加载和使用着色器

// 三角形顶点和法线
glm::vec3 V1(0.0f, 0.0f, 0.0f);
glm::vec3 V2(1.0f, 0.0f, 0.0f);
glm::vec3 V3(0.5f, 1.0f, 0.0f);

glm::vec3 N1(0.0f, 0.0f, 1.0f);
glm::vec3 N2(0.0f, 0.0f, 1.0f);
glm::vec3 N3(0.0f, 0.0f, 1.0f);

GLuint VAO, VBO, EBO;

void setupBuffers()
{
    GLfloat vertices[] = {
        // 顶点位置            // 法向量
        0.0f, 0.0f, 0.0f,     0.0f, 0.0f, 1.0f,  // V1
        1.0f, 0.0f, 0.0f,     0.0f, 0.0f, 1.0f,  // V2
        0.5f, 1.0f, 0.0f,     0.0f, 0.0f, 1.0f   // V3
    };

    GLuint indices[] = {
        0, 1, 2
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void renderLoop(GLFWwindow* window, Shader& shader)
{
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 3.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    shader.use();
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

int main()
{
    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW初始化失败" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "N-Patches Bezier Surface", nullptr, nullptr);
    if (!window) {
        std::cerr << "窗口创建失败" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glewInit();

    Shader shader("vertex_shader.glsl", "fragment_shader.glsl");
    setupBuffers();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        renderLoop(window, shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
