#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <stb_image.h>
#define STB_IMAGE_IMPLEMENTATION


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// #include <learnopengl/shader_m.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int setupMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& attributes);
unsigned int setupMeshWithIndices(const std::vector<float>& vertices, const std::vector<unsigned int>& indices, const std::vector<unsigned int>& attributes);
std::vector<float> generateSphereVertices(float radius, unsigned int sectorCount, unsigned int stackCount);
std::vector<float> generateDonutVertices(float R, float r, unsigned int sectorCount, unsigned int stackCount);
std::vector<float> generateCylinderVertices(float radius, float height, unsigned int sectorCount, std::vector<unsigned int>& indices);
std::vector<float> generateHollowCylinderVertices(float R, float r, float height, unsigned int sectorCount, std::vector<unsigned int>& indices);
unsigned int loadTexture(const char *path);
void renderScene(Shader &shader, Model &backPack);
void renderVehicle(Shader &shader);
unsigned int loadCubemap(vector<std::string> faces);
// void renderModel(Model &model, Shader &shader);
// void renderDonut(Shader &shader);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 创建摄像机
Camera camera(glm::vec3(-2.0f, 1.0f, 4.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightColor(1.0f, 0.98f, 0.75f);
//glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

// rotation speed (can adjust this value)
float rotationSpeed = 0.5f;

// camera speed (can adjust this value)
float cameraSpeed = 0.0005f;

// 头部的旋转矩阵
float headRotationAngle = 0.0f;

// mesh VAO
unsigned int planeVAO;
unsigned int lightCubeVAO;
unsigned int donutVAO;
unsigned int cubeVAO;
unsigned int skyboxVAO;

// sphere vertices
std::vector<float> sphereVertices;

// donut vertices
std::vector<float> donutVertices;
// 索引
std::vector<unsigned int> donutIndices;

// vehicle
unsigned int wheelVAO, bodyVAO, headVAO, weaponVAO;
std::vector<float> wheel, weapon;
std::vector<unsigned int> wheelIndices, weaponIndices;
// 车辆的整体模型矩阵
glm::mat4 vehicleModel = glm::mat4(1.0f);




int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL-Scene", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader geometryShader("src/project/geometry.vs", "src/project/geometry.fs");
    Shader lightShader("src/project/light.vs", "src/project/light.fs");
    // Shader lightShader("src/project/shadow.vs", "src/project/shadow.fs");
    Shader grassShader("src/project/model.vs", "src/project/model.fs"); // plain
    // Shader grassShader("src/project/shadow.vs", "src/project/shadow.fs"); // plain
    Shader backPackShader("src/project/model.vs", "src/project/model.fs"); // model
    // Shader backPackShader("src/project/shadow.vs", "src/project/shadow.fs"); // model
    Shader shadow("src/project/shadow.vs", "src/project/shadow.fs"); // the whole scene
    //Shader model("src/project/model.vs", "src/project/model.fs"); // the whole scene
    Shader depthShader("src/project/shadow_depth.vs", "src/project/shadow_depth.fs", "src/project/shadow_depth.gs"); // depth map
    Shader skyboxShader("src/6.1.cubemaps_skybox/6.1.skybox.vs", "src/6.1.cubemaps_skybox/6.1.skybox.fs");


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float plane[] = {
        // positions          // normals        // texture coords  
        1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        -1.0f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f,  0.0f, 1.0f
    };

    // 正方体
    float vertices[] = {
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

    // 长方体
    float body[] = {
        // positions          // normals
        -1.0f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         1.0f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         1.0f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         1.0f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -1.0f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -1.0f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -1.0f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         1.0f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         1.0f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         1.0f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -1.0f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -1.0f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -1.0f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -1.0f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -1.0f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -1.0f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -1.0f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -1.0f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         1.0f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         1.0f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         1.0f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         1.0f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         1.0f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         1.0f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -1.0f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         1.0f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         1.0f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         1.0f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -1.0f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -1.0f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -1.0f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         1.0f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         1.0f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         1.0f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -1.0f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -1.0f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };


    sphereVertices = generateSphereVertices(0.5f, 36, 18);
    donutVertices = generateDonutVertices(1.0f, 0.5f, 36, 18);
    wheel = generateCylinderVertices(0.5f, 0.5f, 36, wheelIndices);
    weapon = generateHollowCylinderVertices(0.1f, 0.05f, 1.0f, 36, weaponIndices);

    planeVAO = setupMesh(std::vector<float>(plane, plane + 32), { 3, 3, 2 });
    lightCubeVAO = setupMesh(sphereVertices, { 3, 3 });
    //donutVAO = setupMesh(donutVertices, { 3, 3 });
    // cubeVAO = setupMesh(std::vector<float>(vertices, vertices + 180), { 3, 3 });
    cubeVAO = setupMesh(std::vector<float>(vertices, vertices + 216), { 3, 3 });
    donutVAO = setupMeshWithIndices(donutVertices, donutIndices, { 3, 3 });
    bodyVAO = setupMesh(std::vector<float>(body, body + 216), { 3, 3 });
    wheelVAO = setupMeshWithIndices(wheel, wheelIndices, { 3, 3 });
    weaponVAO = setupMeshWithIndices(weapon, weaponIndices, { 3, 3 });
    skyboxVAO = setupMesh(std::vector<float>(skyboxVertices, skyboxVertices + 108), { 3 });
    // Vehicle.generateWheel();
    // donut VAO
    /*unsigned int VBO, EBO;
    glGenVertexArrays(1, &donutVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(donutVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, donutVertices.size() * sizeof(float), &donutVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, donutIndices.size() * sizeof(unsigned int), &donutIndices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);*/





    // load textures (we now use a utility function to keep the code more organized)
    unsigned int grassMap = loadTexture("resources/textures/grass.png");

    vector<std::string> faces
    {
        "resources/textures/skybox/right.jpg",
        "resources/textures/skybox/left.jpg",
        "resources/textures/skybox/top.jpg",
        "resources/textures/skybox/bottom.jpg",
        "resources/textures/skybox/front.jpg",
        "resources/textures/skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);


    // depth map FBO
    // -------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

     // shader configuration
    // --------------------
    grassShader.use();
    grassShader.setInt("material.diffuse", 0);
    //grassShader.setInt("material.specular", 1);

    //grassShader.use();
    //grassShader.setInt("texture_diffuse", 0);
    //grassShader.setInt("diffuseTexture", 0);
    //grassShader.setInt("shadowMap", 1);
    shadow.use();
    shadow.setInt("diffuseTexture", 0);
    shadow.setInt("shadowMap", 1);
    shadow.setInt("skybox", 2);

    // load models
    // -----------
    Model backPack("resources/objects/backpack/backpack.obj");

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) 
    {

        //cout << 1 << endl;
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // 计算光源的新位置
        float angle = glfwGetTime() * rotationSpeed; // 旋转速度
        float radius = 2.0f; // 光源旋转半径

        // 更新光源位置，绕 (0, 0, 0) 旋转
        lightPos = glm::vec3(cos(angle) * radius, 2.0f, sin(angle) * radius);

        // 如果希望光源上下浮动，可以使用正弦函数
        lightPos.y = 0.5f + sin(angle * 0.5f) * 2.0f; // 上下浮动


         // render 
        // -------------------------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        // 1. render scene to depth cubemap
        // --------------------------------------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LEQUAL); 
        depthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        depthShader.setFloat("far_plane", far_plane);
        depthShader.setVec3("lightPos", lightPos);
        renderScene(depthShader, backPack);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal
        // --------------------------------------------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shadow.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shadow.setMat4("projection", projection);
        shadow.setMat4("view", view);
        // set lighting uniforms
        shadow.setVec3("lightPos", lightPos);
        shadow.setVec3("viewPos", camera.Position);
        shadow.setInt("shadows", 1); // enable/disable shadows by pressing 'SPACE'
        shadow.setFloat("far_plane", far_plane);
        shadow.setVec3("lightColor", lightColor);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(shadow, backPack);

        /*// render skybox
        glDepthFunc(GL_LEQUAL);
        shadow.use();
        shadow.setBool("isSkybox", true);   
        shadow.setBool("isLightSource", false);
        shadow.setMat4("projection", projection);
        shadow.setMat4("view", view);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);*/

        
        /*
        // render scene from light's point of view
        // ---------------------------------------
        lightShader.use();
        lightShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        grassShader.use();
        grassShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        backPackShader.use();
        backPackShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, grassMap);
        
        // renderScene
        grassShader.use();
        //modelShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        //grassShader.setVec3("lightColor", lightColor);
        grassShader.setVec3("light.position", lightPos);
        grassShader.setVec3("viewPos", camera.Position);
        

        // light properties
        grassShader.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);  // 环境光
        grassShader.setVec3("light.diffuse", 0.2f, 0.2f, 0.2f);  // 漫反射光
        grassShader.setVec3("light.specular", 0.0f, 0.0f, 0.0f);  // 镜面反射光
        grassShader.setFloat("material.shininess", 0.0f);         // 光泽度
        

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        grassShader.setMat4("projection", projection);
        grassShader.setMat4("view", view);
        
        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        grassShader.setMat4("model", model);


        // bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassMap);

        // render plain
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // 背包
        backPackShader.use();
        backPackShader.setVec3("lightColor", lightColor);
        backPackShader.setVec3("light.position", lightPos);
        backPackShader.setVec3("viewPos", camera.Position);

        // light properties
        backPackShader.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);  // 环境光
        backPackShader.setVec3("light.diffuse", 0.4f, 0.4f, 0.4f);  // 漫反射光
        backPackShader.setVec3("light.specular", 0.0f, 0.0f, 0.0f);  // 镜面反射光
        backPackShader.setFloat("material.shininess", 0.0f);   

        // view/projection transformations
        backPackShader.setMat4("projection", projection);
        backPackShader.setMat4("view", view);

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.13f, -0.6f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));	// it's a bit too big for our scene, so scale it down
        backPackShader.setMat4("model", model);
        backPack.Draw(backPackShader);

        // render light sphere
        lightShader.use();
        lightShader.setVec3("lightColor", lightColor);
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.6f)); // a smaller cube
        lightShader.setMat4("model", model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, sphereVertices.size() / 6);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        */

        /*// reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render scene as normal using the generated depth/shadow map
        // ----------------------------------------------------------
        shadow.use();
        projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        shadow.setMat4("projection", projection);
        shadow.setMat4("view", view);
        // set light uniforms
        shadow.setVec3("lightPos", lightPos);
        shadow.setVec3("viewPos", camera.Position);
        shadow.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        
        shadow.setMat4("model", model);
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        shadow.setMat4("model", model);
        backPack.Draw(shadow);*/
        
        



        // 草坪
        /*grassShader.use();
        //modelShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        //grassShader.setVec3("lightColor", lightColor);
        grassShader.setVec3("light.position", lightPos);
        grassShader.setVec3("viewPos", camera.Position);
        

        // light properties
        grassShader.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);  // 环境光
        grassShader.setVec3("light.diffuse", 0.2f, 0.2f, 0.2f);  // 漫反射光
        grassShader.setVec3("light.specular", 0.0f, 0.0f, 0.0f);  // 镜面反射光
        grassShader.setFloat("material.shininess", 0.0f);         // 光泽度
        

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        grassShader.setMat4("projection", projection);
        grassShader.setMat4("view", view);
        
        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        grassShader.setMat4("model", model);


        // bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassMap);

        // render plane
        glBindVertexArray(planeVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // 背包
        backPackShader.use();
        // backPackShader.setVec3("lightColor", lightColor);
        backPackShader.setVec3("light.position", lightPos);
        backPackShader.setVec3("viewPos", camera.Position);

        // light properties
        backPackShader.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);  // 环境光
        backPackShader.setVec3("light.diffuse", 0.4f, 0.4f, 0.4f);  // 漫反射光
        backPackShader.setVec3("light.specular", 0.0f, 0.0f, 0.0f);  // 镜面反射光
        backPackShader.setFloat("material.shininess", 0.0f);   

        // view/projection transformations
        backPackShader.setMat4("projection", projection);
        backPackShader.setMat4("view", view);

        // render the loaded model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.13f, -0.6f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));	// it's a bit too big for our scene, so scale it down
        backPackShader.setMat4("model", model);
        backPack.Draw(backPackShader); 


        // render light sphere
        lightShader.use();
        lightShader.setVec3("lightColor", lightColor);
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.6f)); // a smaller cube
        lightShader.setMat4("model", model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, sphereVertices.size() / 6);*/


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &planeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
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

     // 移动整体车辆（按键控制）
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        vehicleModel = glm::translate(vehicleModel, glm::vec3(0.0f, 0.0f, -cameraSpeed));
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        vehicleModel = glm::translate(vehicleModel, glm::vec3(0.0f, 0.0f, cameraSpeed));
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        vehicleModel = glm::translate(vehicleModel, glm::vec3(-cameraSpeed, 0.0f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        vehicleModel = glm::translate(vehicleModel, glm::vec3(cameraSpeed, 0.0f, 0.0f));
    }

        // 控制头部旋转
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        headRotationAngle -= rotationSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        headRotationAngle += rotationSpeed;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 定义 setupMesh 函数
unsigned int setupMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& attributes)
{
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针
    size_t stride = 0;
    for (auto attr : attributes) stride += attr; // 计算步长
    stride *= sizeof(float);

    size_t offset = 0;
    for (unsigned int i = 0; i < attributes.size(); ++i)
    {
        glVertexAttribPointer(i, attributes[i], GL_FLOAT, GL_FALSE, stride, (void*)(offset * sizeof(float)));
        glEnableVertexAttribArray(i);
        offset += attributes[i];
    }

    glBindVertexArray(0); // 解绑 VAO
    return VAO;
}

// 生成球体顶点数据
std::vector<float> generateSphereVertices(float radius, unsigned int sectorCount, unsigned int stackCount)
{
    std::vector<float> vertices;
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (unsigned int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        for (unsigned int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    std::vector<unsigned int> indices;
    unsigned int k1, k2;
    for (unsigned int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    std::vector<float> sphereVertices;
    for (unsigned int i = 0; i < indices.size(); ++i)
    {
        unsigned int index = indices[i];
        sphereVertices.push_back(vertices[index * 6]);
        sphereVertices.push_back(vertices[index * 6 + 1]);
        sphereVertices.push_back(vertices[index * 6 + 2]);
        sphereVertices.push_back(vertices[index * 6 + 3]);
        sphereVertices.push_back(vertices[index * 6 + 4]);
        sphereVertices.push_back(vertices[index * 6 + 5]);
    }

    return sphereVertices;
}

// utility function for loading a 2D texture from file  
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// renders the 3D scene
// --------------------
void renderScene(Shader &shader, Model &backPack)
{

    // render plane
    glm::mat4 model;
    model = glm::mat4(1.0f);
    shader.setBool("isLightSource", false);
    shader.setBool("isModel", true);
    shader.setBool("isSkybox", false);
    shader.setMat4("model", model);
    shader.setVec3("lightColor", lightColor);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);



    // render donut
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.5f, 0.13f, 0.0f));
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
    // 旋转圆环：每一帧旋转一定角度
    float angle = glfwGetTime() * 50.0f; // 用时间控制旋转角度，50.0f 表示每秒旋转 50 度
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); // 绕 y 轴旋转
    shader.setBool("isLightSource", false);
    shader.setBool("isModel", false);
    shader.setBool("isSkybox", false);
    shader.setMat4("model", model);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("objectColor", 1.0f, 0.647f, 0.0f);
    glBindVertexArray(donutVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, donutVertices.size() / 6);

    /*// render cube
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.13f, 0.0f));
    model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));
    shader.setBool("isLightSource", false);
    shader.setBool("isModel", false);
    shader.setMat4("model", model);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    */

    // render the loaded model
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.13f, -0.6f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));	// it's a bit too big for our scene, so scale it down
     shader.setBool("isLightSource", false);
     shader.setBool("isModel", true);
     shader.setBool("isSkybox", false);
    shader.setMat4("model", model);
    shader.setVec3("lightColor", lightColor);
    backPack.Draw(shader);

   
    // render the vehicle
    model = vehicleModel;
    // 1. render the body
    model = glm::translate(model, glm::vec3(0.0f, 0.10f, 0.6f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(0.09f, 0.12f, 0.12f));	// it's a bit too big for our scene, so scale it down
    shader.setBool("isLightSource", false);
    shader.setBool("isModel", false);
    shader.setBool("isSkybox", false);
    shader.setMat4("model", model);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("objectColor", 0.0f, 0.0f, 1.0f);
    glBindVertexArray(bodyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // 2. render the head
    model = vehicleModel;
    model = glm::translate(model, glm::vec3(0.0f, 0.20f, 0.6f)); // translate it down so it's at the center of the scene
    model = glm::rotate(model, glm::radians(headRotationAngle), glm::vec3(0.0f, 0.19f, 0.0f)); // 旋转头部
    model = glm::scale(model, glm::vec3(0.09f, 0.09f, 0.09f));	// it's a bit too big for our scene, so scale it down
    shader.setBool("isLightSource", false);
    shader.setBool("isModel", false);
    shader.setBool("isSkybox", false);
    shader.setMat4("model", model);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("objectColor", 1.0f, 0.5f, 1.0f);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);   

    /* // weapon
    glm::mat4 cylinderModel = model; // 使用头部的旋转矩阵
    //model = vehicleModel;
    //model = glm::translate(model, glm::vec3(0.0f, 0.19f, 0.53f)); // translate it down so it's at the center of the scene
    cylinderModel = glm::translate(cylinderModel, glm::vec3(0.0f, 0.0f, -1.0f)); // translate it down so it's at the center of the scene
    //model = glm::rotate(model, glm::radians(headRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // 旋转头部
    cylinderModel = glm::rotate(cylinderModel, glm::radians(headRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // 旋转头部
    //cylinderModel = glm::scale(cylinderModel, glm::vec3(0.3f, 0.3f, 0.08f));	// it's a bit too big for our scene, so scale it down
    shader.setBool("isLightSource", false);
    shader.setBool("isModel", false);
    shader.setMat4("model", cylinderModel);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("objectColor", 0.0f, 1.0f, 0.0f);
    glBindVertexArray(weaponVAO);
    // glDrawArrays(GL_TRIANGLE_FAN, 0, weapon.size() / 6); 
    glDrawElements(GL_TRIANGLES, weaponIndices.size(), GL_UNSIGNED_INT, 0);  
    */

    // 3. render the wheel
    //glm::mat4 model;
    model = vehicleModel;
    model = glm::translate(model, glm::vec3(-0.042f, 0.03f, 0.53f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(0.06f, 0.06f, 0.06f));	// it's a bit too big for our scene, so scale it down
    shader.setBool("isLightSource", false);
    shader.setBool("isModel", false);
    shader.setBool("isSkybox", false);
    shader.setMat4("model", model);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("objectColor", 1.0f, 1.0f, 0.0f);
    glBindVertexArray(wheelVAO);
    glDrawElements(GL_TRIANGLES, wheelIndices.size(), GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, wheel.size() / 6);

    model = vehicleModel;
    model = glm::translate(model, glm::vec3(0.042f, 0.03f, 0.53f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(0.06f, 0.06f, 0.06f));	// it's a bit too big for our scene, so scale it down
    shader.setBool("isLightSource", false);
    shader.setBool("isModel", false);
    shader.setBool("isSkybox", false);
    shader.setMat4("model", model);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("objectColor", 1.0f, 1.0f, 0.0f);
    glBindVertexArray(wheelVAO);
    glDrawElements(GL_TRIANGLES, wheelIndices.size(), GL_UNSIGNED_INT, 0);

    model = vehicleModel;
    model = glm::translate(model, glm::vec3(0.042f, 0.03f, 0.67f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(0.06f, 0.06f, 0.06f));	// it's a bit too big for our scene, so scale it down
    shader.setBool("isLightSource", false);
    shader.setBool("isModel", false);
    shader.setBool("isSkybox", false);
    shader.setMat4("model", model);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("objectColor", 1.0f, 1.0f, 0.0f);
    glBindVertexArray(wheelVAO);
    glDrawElements(GL_TRIANGLES, wheelIndices.size(), GL_UNSIGNED_INT, 0);

    model = vehicleModel;
    model = glm::translate(model, glm::vec3(-0.042f, 0.03f, 0.67f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(0.06f, 0.06f, 0.06f));	// it's a bit too big for our scene, so scale it down
    shader.setBool("isLightSource", false);
    shader.setBool("isModel", false);
    shader.setBool("isSkybox", false);
    shader.setMat4("model", model);
    shader.setVec3("lightColor", lightColor);
    shader.setVec3("objectColor", 1.0f, 1.0f, 0.0f);
    glBindVertexArray(wheelVAO);
    glDrawElements(GL_TRIANGLES, wheelIndices.size(), GL_UNSIGNED_INT, 0);

    


    // render light cube
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.6f)); // a smaller cube
    shader.setBool("isLightSource", true);
    shader.setBool("isModel", true);
    shader.setBool("isSkybox", false);
    shader.setMat4("model", model);
    shader.setVec3("lightColor", lightColor);
    glBindVertexArray(lightCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, sphereVertices.size() / 6);
    

    //glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
    //shader.setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.

    // render skybox
    /*glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content]
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(5.0f));
    shader.setMat4("model", model);
    shader.setBool("isLightSource", false);
    shader.setBool("isModel", true);
    shader.setVec3("lightColor", lightColor);
    shader.setBool("isSkybox", true);
    glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
    //shader.setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //shader.setInt("reverse_normals", 0); // and of course disable it
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS); // set depth function back to default*/

}

// 生成甜甜圈顶点数据
std::vector<float> generateDonutVertices(float R, float r, unsigned int sectorCount, unsigned int stackCount)
{
    std::vector<float> vertices;
    //std::vector<unsigned int> indices;
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f;            // vertex normal
    float sectorStep = 2 * M_PI / sectorCount;      // Angle between each sector
    float stackStep = 2 * M_PI / stackCount;        // Angle between each stack
    float sectorAngle, stackAngle;

    for (unsigned int i = 0; i <= stackCount; ++i)
    {
        stackAngle = i * stackStep;                 // Starting from 0 to 2pi
        xy = R + r * cosf(stackAngle);              // r * cos(u) + R
        z = r * sinf(stackAngle);                   // r * sin(u)

        // Loop through each sector
        for (unsigned int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // Starting from 0 to 2pi

            // Vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // Normal calculation
            nx = cosf(stackAngle) * cosf(sectorAngle);  // Normal in x direction
            ny = cosf(stackAngle) * sinf(sectorAngle);  // Normal in y direction
            nz = sinf(stackAngle);                      // Normal in z direction
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    // Generate indices
    for (unsigned int i = 0; i < stackCount; ++i)
    {
        for (unsigned int j = 0; j < sectorCount; ++j)
        {
            unsigned int first = (i * (sectorCount + 1)) + j;
            unsigned int second = first + sectorCount + 1;

            donutIndices.push_back(first);
            donutIndices.push_back(second);
            donutIndices.push_back(first + 1);

            donutIndices.push_back(second);
            donutIndices.push_back(second + 1);
            donutIndices.push_back(first + 1);
        }
    }

    std::vector<float> donutVertices;
    for (unsigned int i = 0; i < donutIndices.size(); ++i)
    {
        unsigned int index = donutIndices[i];
        donutVertices.push_back(vertices[index * 6]);
        donutVertices.push_back(vertices[index * 6 + 1]);
        donutVertices.push_back(vertices[index * 6 + 2]);
        donutVertices.push_back(vertices[index * 6 + 3]);
        donutVertices.push_back(vertices[index * 6 + 4]);
        donutVertices.push_back(vertices[index * 6 + 5]);
    }

    return donutVertices;
}

std::vector<float> generateCylinderVertices(float radius, float height, unsigned int sectorCount, std::vector<unsigned int>& indices)
{
    std::vector<float> vertices;
    float x, y, z, nx, ny, nz;
    float sectorStep = 2 * M_PI / sectorCount;
    float sectorAngle;

    // 1. Top cap
    for (unsigned int i = 0; i <= sectorCount; ++i)
    {
        sectorAngle = i * sectorStep;
        x = radius * cosf(sectorAngle);
        y = radius * sinf(sectorAngle);
        z = height / 2;
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
    }

    // 2. Bottom cap
    for (unsigned int i = 0; i <= sectorCount; ++i)
    {
        sectorAngle = i * sectorStep;
        x = radius * cosf(sectorAngle);
        y = radius * sinf(sectorAngle);
        z = -height / 2;
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
    }

    // 3. Side
    for (unsigned int i = 0; i <= sectorCount; ++i)
    {
        sectorAngle = i * sectorStep;
        x = radius * cosf(sectorAngle);
        y = radius * sinf(sectorAngle);
        z = height / 2;
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(cosf(sectorAngle));
        vertices.push_back(sinf(sectorAngle));
        vertices.push_back(0.0f);

        x = radius * cosf(sectorAngle);
        y = radius * sinf(sectorAngle);
        z = -height / 2;
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(cosf(sectorAngle));
        vertices.push_back(sinf(sectorAngle));
        vertices.push_back(0.0f);
    }

    // Generate indices for top cap
    for (unsigned int i = 0; i < sectorCount; ++i)
    {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }

    // Generate indices for bottom cap
    unsigned int baseIndex = sectorCount + 1;
    for (unsigned int i = 0; i < sectorCount; ++i)
    {
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + i + 1);
        indices.push_back(baseIndex + i + 2);
    }

    // Generate indices for side
    baseIndex = (sectorCount + 1) * 2;
    for (unsigned int i = 0; i < sectorCount; ++i)
    {
        unsigned int k1 = baseIndex + i * 2;
        unsigned int k2 = k1 + 1;
        unsigned int k3 = k1 + 2;
        unsigned int k4 = k3 + 1;

        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k3);

        indices.push_back(k2);
        indices.push_back(k4);
        indices.push_back(k3);
    }

    return vertices;
}

std::vector<float> generateHollowCylinderVertices(float R, float r, float height, unsigned int sectorCount, std::vector<unsigned int>& indices)
{
    // generate a hollow cylinder
    std::vector<float> vertices;
    float x, y, z, nx, ny, nz;
    float sectorStep = 2 * M_PI / sectorCount;
    float sectorAngle;

    // 1. Top cap
    for (unsigned int i = 0; i <= sectorCount; ++i)
    {
        sectorAngle = i * sectorStep;
        x = R * cosf(sectorAngle);
        y = R * sinf(sectorAngle);
        z = height / 2;
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
    }

    // 2. Bottom cap
    for (unsigned int i = 0; i <= sectorCount; ++i)
    {
        sectorAngle = i * sectorStep;
        x = r * cosf(sectorAngle);
        y = r * sinf(sectorAngle);
        z = -height / 2;
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
    }

    // 3. Side
    for (unsigned int i = 0; i <= sectorCount; ++i)
    {
        sectorAngle = i * sectorStep;
        x = R * cosf(sectorAngle);
        y = R * sinf(sectorAngle);
        z = height / 2;
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(cosf(sectorAngle));
        vertices.push_back(sinf(sectorAngle));
        vertices.push_back(0.0f);

        x = r * cosf(sectorAngle);
        y = r * sinf(sectorAngle);
        z = -height / 2;
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
        vertices.push_back(cosf(sectorAngle));
        vertices.push_back(sinf(sectorAngle));
        vertices.push_back(0.0f);
    }

    // Generate indices for top cap
    for (unsigned int i = 0; i < sectorCount; ++i)
    {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
    }

    // Generate indices for bottom cap
    unsigned int baseIndex = sectorCount + 1;   
    for (unsigned int i = 0; i < sectorCount; ++i)
    {
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + i + 1);
        indices.push_back(baseIndex + i + 2);
    }

    // Generate indices for side
    baseIndex = (sectorCount + 1) * 2;
    for (unsigned int i = 0; i < sectorCount; ++i)
    {
        unsigned int k1 = baseIndex + i * 2;
        unsigned int k2 = k1 + 1;
        unsigned int k3 = k1 + 2;
        unsigned int k4 = k3 + 1;

        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k3);

        indices.push_back(k2);
        indices.push_back(k4);
        indices.push_back(k3);
    }

    return vertices;
}

unsigned int setupMeshWithIndices(const std::vector<float>& vertices, const std::vector<unsigned int>& indices, const std::vector<unsigned int>& attributes)
{
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // 设置顶点属性指针
    size_t stride = 0;
    for (auto attr : attributes) stride += attr; // 计算步长
    stride *= sizeof(float);

    size_t offset = 0;
    for (unsigned int i = 0; i < attributes.size(); ++i)
    {
        glVertexAttribPointer(i, attributes[i], GL_FLOAT, GL_FALSE, stride, (void*)(offset * sizeof(float)));
        glEnableVertexAttribArray(i);
        offset += attributes[i];
    }

    glBindVertexArray(0); // 解绑 VAO
    return VAO;
} 

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


// render the entire scene
/*void renderScene(Shader &shader)
{
    // render plane
    shader.use();
    shader.setVec3("lightPos", lightPos);
    shader.setVec3("viewPos", camera.Position);

    // light properties
    shader.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);  // 环境光
    shader.setVec3("light.diffuse", 0.2f, 0.2f, 0.2f);  // 漫反射光
    shader.setVec3("light.specular", 0.0f, 0.0f, 0.0f);  // 镜面反射光
    shader.setFloat("material.shininess", 0.0f);         // 光泽度

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    // world transformation
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    
}*/

// render the loaded model
/*void renderModel(Model &model, Shader &shader)
{
    shader.use();
    model.Draw(shader);
}*/


