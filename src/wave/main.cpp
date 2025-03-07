#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>
#define STB_IMAGE_IMPLEMENTATION

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>

// �ص���������
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// ��Ļ����
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ���������
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// ʱ�����
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
    // ��ʼ�� GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ��������
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Wave Simulation", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // ��׽���
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ��ʼ�� GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // �����������ɫ������
    Shader ourShader("src/wave/wave.vs", "src/wave/wave.fs");

    // ���ö������ݺͻ���
    const int gridSize = 100;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    for (int i = 0; i <= gridSize; ++i) {
        for (int j = 0; j <= gridSize; ++j) {
            vertices.push_back((float)i / gridSize - 0.5f);
            vertices.push_back(0.0f);
            vertices.push_back((float)j / gridSize - 0.5f);
        }
    }
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            indices.push_back(i * (gridSize + 1) + j);
            indices.push_back((i + 1) * (gridSize + 1) + j);
            indices.push_back(i * (gridSize + 1) + j + 1);

            indices.push_back(i * (gridSize + 1) + j + 1);
            indices.push_back((i + 1) * (gridSize + 1) + j);
            indices.push_back((i + 1) * (gridSize + 1) + j + 1);
        }
    }

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ��Ⱦѭ��
    while (!glfwWindowShouldClose(window)) {
        // ÿ֡ʱ���߼�
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // ���봦��
        processInput(window);

        // ��Ⱦָ��
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // ������ɫ��
        ourShader.use();

        // �����������ͼ��ͶӰ����
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);

        // ���� uniform ����
        ourShader.setFloat("uTime", currentFrame);
        ourShader.setFloat("waveAmplitude", 0.1f);
        ourShader.setFloat("waveLength", 1.0f);
        ourShader.setFloat("waveSpeed", 1.0f);

        // ����
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        // �������岢��ѯ IO �¼�
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // �ͷ���Դ
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

// �����������룺��ѯ GLFW �Ƿ���/�ͷ���ؼ�
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// �����ڴ�С�ı�ʱ���ص��˺����Ե����ӿڴ�С
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// ����ƶ��ص�
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // ע���������෴�ģ���Ϊ y �����Ǵӵײ������������
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// ���ֻص�
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}