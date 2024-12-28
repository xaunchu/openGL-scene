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
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int setupMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& attributes);
std::vector<float> generateSphereVertices(float radius, unsigned int sectorCount, unsigned int stackCount);
unsigned int loadTexture(const char *path);
void renderScene(Shader &shader);
void renderModel(Model &model, Shader &shader);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
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
float rotationSpeed = 0.2f;

// mesh VAO
unsigned int planeVAO;
unsigned int lightCubeVAO;

// sphere vertices
std::vector<float> sphereVertices;



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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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
    // Shader shader("src/project/shadow.vs", "src/project/shadow.fs"); // the whole scene
    Shader model("src/project/model.vs", "src/project/model.fs"); // the whole scene


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float plane[] = {
        // positions          // normals        // texture coords  
        1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
        1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
        -1.0f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f,  0.0f, 1.0f
    };

    sphereVertices = generateSphereVertices(0.5f, 36, 18);
    planeVAO = setupMesh(std::vector<float>(plane, plane + 32), { 3, 3, 2 });
    lightCubeVAO = setupMesh(sphereVertices, { 3, 3 });

    // load textures (we now use a utility function to keep the code more organized)
    unsigned int grassMap = loadTexture("resources/textures/grass.png");


    // depth map FBO
    // -------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
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
        float radius = 3.0f; // 光源旋转半径

        // 更新光源位置，绕 (0, 0, 0) 旋转
        lightPos = glm::vec3(cos(angle) * radius, 1.0f, sin(angle) * radius);

        // 如果希望光源上下浮动，可以使用正弦函数
        lightPos.y = -1.0f + sin(angle * 0.5f) * 2.0f; // 上下浮动


         // render 
        // -------------------------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*// render depth of scene to texture (from light's perspective)
        // --------------------------------------------------------------
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 7.5f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;

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
        grassShader.setVec3("lightColor", lightColor);
        grassShader.setVec3("lightPos", lightPos);
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
        backPackShader.setVec3("lightPos", lightPos);
        backPackShader.setVec3("viewPos", camera.Position);

        // light properties
        backPackShader.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);  // 环境光
        backPackShader.setVec3("light.diffuse", 0.2f, 0.2f, 0.2f);  // 漫反射光
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

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render scene as normal using the generated depth/shadow map
        // ----------------------------------------------------------
        shader.use();
        projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // set light uniforms
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("viewPos", camera.Position);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderScene(shader);*/


        // 草坪
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
        glDrawArrays(GL_TRIANGLES, 0, sphereVertices.size() / 6);


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

