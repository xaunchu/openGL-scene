#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>



float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 相机参数
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f); // 增大初始距离
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// 鼠标控制参数
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float fov = 45.0f; // 视野角度

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// OpenGL 错误回调函数
void GLAPIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    std::cerr << "OpenGL Debug Message: " << message << std::endl;
}

// GLFW 错误回调函数
void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
}

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

std::vector<Vertex> generateBezierPatch(const glm::vec3& P0, const glm::vec3& P1, const glm::vec3& P2,
                                        const glm::vec3& N0, const glm::vec3& N1, const glm::vec3& N2, int subdivisions) {
    // 计算控制点
    glm::vec3 P01 = 2.0f / 3.0f * P0 + 1.0f / 3.0f * P1;
    glm::vec3 P12 = 2.0f / 3.0f * P1 + 1.0f / 3.0f * P2;
    glm::vec3 P20 = 2.0f / 3.0f * P2 + 1.0f / 3.0f * P0;

    glm::vec3 innerControl = (P0 + P1 + P2) / 3.0f;  // 简单的插值内部点

    std::vector<Vertex> vertices;

    // Bezier 细分
    for (int i = 0; i <= subdivisions; i++) {
        for (int j = 0; j <= subdivisions - i; j++) {
            float u = i / static_cast<float>(subdivisions);
            float v = j / static_cast<float>(subdivisions);
            float w = 1.0f - u - v;

            // Bezier 公式
            glm::vec3 position = u * u * P0 + v * v * P1 + w * w * P2 +
                                 2.0f * u * v * P01 + 2.0f * v * w * P12 + 2.0f * w * u * P20;
            
            glm::vec3 normal = glm::normalize(N0 * u + N1 * v + N2 * w);  // 简单法线插值
            
            vertices.push_back({position, normal});
        }
    }

    return vertices;
}

void renderBezierPatch(const std::vector<Vertex>& vertices, int subdivisions) {
    std::vector<GLuint> indices;
    for (int i = 0; i < subdivisions; i++) {
        for (int j = 0; j < subdivisions - i; j++) {
            int index0 = i * (subdivisions + 1) + j;
            int index1 = index0 + 1;
            int index2 = index0 + (subdivisions + 1);

            indices.push_back(index0);
            indices.push_back(index1);
            indices.push_back(index2);

            if (j + 1 < subdivisions - i) {
                indices.push_back(index1);
                indices.push_back(index2 + 1);
                indices.push_back(index2);
            }
        }
    }

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

int main() {
    // 初始化 GLFW
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Bezier N-Patches", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // 初始化控制点和法线
    glm::vec3 P0(-0.5f, -0.5f, 0.0f), P1(0.5f, -0.5f, 0.0f), P2(0.0f, 0.5f, 0.0f);
    glm::vec3 N0(0.0f, 0.0f, 1.0f), N1(0.0f, 0.0f, 1.0f), N2(0.0f, 0.0f, 1.0f);

    int subdivisions = 10;
    std::vector<Vertex> bezierVertices = generateBezierPatch(P0, P1, P2, N0, N1, N2, subdivisions);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderBezierPatch(bezierVertices, subdivisions);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
