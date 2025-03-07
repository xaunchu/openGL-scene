# 一、项目介绍
本项目100%基于**OpenGL和cpp**，构建了一个**草地的3D场景**。其中**导入了模型**，涉及了**复杂几何体**、**几何体组合**的渲染，并使用了一个**球体的移动光源**围绕整个场景旋转，模拟真实世界中的太阳，整个场景的光照基于Blinn-Phong光照模型，具有较强的真实感。进一步涉及了各个物体之间的**阴影的实时渲染**，基于点光源阴影技术。最后，本项目还允许用户在空间进行随意的**漫游**，**操纵移动**场景上的组合几何体，并还可**旋转**组合几何体中的一部分，具有可玩性。

整个场景如下：
![](https://github.com/xaunchu/openGL-scene/blob/master/images/Pasted%20image%2020250104195042.png)

---
# 二、工作内容
## 1. 光照模型

**Blinn-Phong** 光照模型包含了环境光、漫反射光和镜面光的计算：

- **环境光**：为场景提供基本的光照，确保物体即使在没有直接光源照射的情况下也能显现。
- **漫反射光**：根据光源到物体表面的角度来计算，模拟光照与物体表面相互作用的效果。
- **镜面光**：计算反射光与观察方向的角度，从而模拟高光效果，反射光强度与视角的关系紧密。

## 2. 阴影映射

使用点光源阴影技术，在光源与物体之间计算深度信息：

- **深度贴图**：从光源视角渲染深度信息，用于之后判断场景中哪些区域是处于阴影中的。
- **阴影计算**：通过深度比较判断每个片段是否被阴影遮挡，从而影响光照的最终结果。

## 3. 草地与纹理映射、模型导入

- 草地是通过绑定PNG格式的纹理来渲染的，通过适当的纹理映射和光照模型使其显得更加真实。
- 并导入了一个背包的模型(.obj)，增加场景的丰富性。

## 4. 圆环几何体

圆环几何体通过**极坐标系**来生成顶点：

- **纬度方向切割**（堆栈方向）：控制圆环的厚度。
- **经度方向切割**：控制圆环的分割。
- **法向量计算**：每个顶点的法向量是通过圆环的表面法线计算得出的，确保正确的光照计算。

## 5. 着色器实现

项目中的着色器分为多个模块：

- **深度着色器**：生成深度信息以供阴影映射使用。
- **基础着色器(片段、顶点)**：处理光照计算和场景渲染，区分不同的物体类型（光源、模型、简单几何体）。

## 6. 实现的功能和交互

- **漫游和操作**：用户可以自由漫游场景，操控场景中的物体，并旋转几何体，增强了场景的互动性。
- **光源动画**：使用球体移动光源模拟真实世界中的太阳，动态改变光照效果。
- **旋转**：使圆环自动旋转。

---
# 三、具体实现

## 1. 着色器
本项目中使用了五个着色器来绘制整体场景。其中三个主要是用来计算和生成**深度贴图**，从而用于**阴影映射**。剩下的两个用来起到整个场景的基础绘制作用。

对于**深度着色器**，其片段着色器和顶点着色器如下：

`shadow_depth.fs`
```cpp
#version 330 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}
```
`shadow_depth.vs`
```cpp
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;

void main()
{
    gl_Position = model * vec4(aPos, 1.0);
}
```

在此以外，还有一个几何着色器。此项目模拟整个场景处于一个立方体内。此处的几何着色器是负责将所有世界空间的顶点变换到6个不同的光空间的着色器。几何着色器的作用是从一个三角形生成多个新三角形（6个面，每面3个顶点），并将每个顶点转换到光空间，以生成立方体贴图的深度信息。这些深度信息用于阴影映射，帮助计算场景中物体的阴影。其代码如下：

`shadow_depth.gs`
```cpp
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
} 
```

对于其他几何体、模型、光源等的绘制均只需要片段着色器和顶点着色器。此项目中，为了使代码更加整合，我们均用相同的着色器来绘制它们。

但对于一些不同的物体，其所需的计算和值不同。比如光源的着色器只需要简单的根据光的颜色来附着，而不需要计算基于Blinn-Phong模型的光照计算。因此引入了**bool类型的全局统一变量**(Uniform)，来区分我们要绘制的是光源、添加材质的模型、还是一个需要传入颜色的简单几何体，等等。

这样使得代码的复用性更好，可以使项目的代码结构更加清晰。

**顶点着色器** `shadow.vs`:
```cpp
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform bool reverse_normals;
uniform bool isSkybox;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    if(reverse_normals) // a slight hack to make sure the outer large cube displays lighting from the 'inside' instead of the default 'outside'.
        vs_out.Normal = transpose(inverse(mat3(model))) * (-1.0 * aNormal);
    else
        vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;

    vs_out.TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```
**片段着色器** `shadow.fs` （伪代码）
```cpp
function ShadowCalculation(fragPos):
    fragToLight = fragPos - lightPos  # 从片段到光源的向量
    currentDepth = length(fragToLight)  # 计算片段到光源的深度
    shadow = 0.0  # 初始化阴影值
    bias = 0.05  # 用于避免阴影接触（阴影裂纹）的偏移量
    samples = 40  # 阴影计算的样本数
    viewDistance = length(viewPos - fragPos)  # 计算片段到摄像机的距离
    diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0  # 根据视距调整采样圆盘的半径
    
    for i = 0 to samples - 1:  # 遍历所有样本点
        closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r
        closestDepth *= far_plane  # 恢复深度映射
        if (currentDepth - bias > closestDepth):
            shadow += 1.0  # 如果当前深度大于采样到的深度，则增加阴影值

    shadow /= float(samples)  # 将阴影值平均化

    return shadow

function main():
    if isLightSource:  # 如果当前片段是光源
        FragColor = lightColor  # 输出光源颜色
        return
    
    color = texture(diffuseTexture, fs_in.TexCoords).rgb  # 从纹理中获取漫反射颜色
    normal = normalize(fs_in.Normal)  # 将法线向量归一化
    lightColor = (1.0, 1.0, 1.0)  # 设置光源颜色为白色（也可以自定义颜色）
    
    # 环境光照
    ambient = 0.5 * lightColor
    
    # 漫反射光照
    lightDir = normalize(lightPos - fs_in.FragPos)  # 从片段到光源的方向
    diff = max(dot(lightDir, normal), 0.0)  # 计算漫反射因子
    diffuse = diff * lightColor  # 应用漫反射光照
    
    # 镜面反射光照
    viewDir = normalize(viewPos - fs_in.FragPos)  # 从片段到摄像机的方向
    reflectDir = reflect(-lightDir, normal)  # 计算反射方向
    halfwayDir = normalize(lightDir + viewDir)  # 计算半程向量用于镜面反射
    spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0)  # 计算镜面反射因子
    specular = 0.2 * spec * lightColor  # 应用镜面反射光照
    
    # 如果需要阴影，计算阴影
    shadow = 0.0
    if shadows:
        shadow = ShadowCalculation(fs_in.FragPos)  # 调用阴影计算函数
    
    # 根据阴影因子计算最终光照
    lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color
    
    if not isModel:
        specular = 0.4 * spec * lightColor  # 对非模型物体调整镜面反射光照
        lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * objectColor  # 应用物体颜色
        FragColor = lighting * objectColor * lightColor  # 对非模型物体计算最终颜色
        return
    
    # 对于模型物体，应用最终的光照与光源颜色
    FragColor = lighting * lightColor

```
在这个片段着色器中，涉及到Blinn-Phong光照模型的计算和阴影的计算。
### 1. Blinn-Phong

需要考虑三个分量：环境(Ambient)、漫反射(Diffuse)和镜面(Specular)光照。

对于**环境光照**，我们使用一个很小的常量（光照）颜色，添加到物体片段的最终颜色中，这样子的话即便场景中没有直接的光源也能看起来存在有一些发散的光。用光的颜色乘以一个很小的常量环境因子，再乘以物体的颜色，将最终结果作为片段的颜色。

对于**漫反射光照**，我们需要法向量和定向的光线。法向量是一个垂直于定点表面的单位向量，但由于顶点本身没有表面，我们利用它周围的顶点来计算出这个顶点的表面。同时，所有光照的计算都是在片段着色器里进行，所以我们需要将法向量由顶点着色器传递到片段着色器。由于光源的位置是一个静态变量，我们可以简单地在片段着色器中把它声明为uniform(uniform vec3 lightPos;)。我们使用在前面声明的lightPos向量作为光源位置。最后，我们还需要片段的位置。我们可以通过把顶点位置属性乘以模型矩阵，来把它变换到世界空间坐标。

然后，我们需要计算光源和片段位置之间的方向向量。光的方向向量是光源位置向量与片段位置向量之间的向量差。我们同样希望确保所有相关向量最后都转换为单位向量，所以我们把法线和最终的方向向量都进行标准化。然后，我们需要对这两个向量进行点乘，计算光源对当前片段实际的漫反射影响。结果值再乘以光的颜色，得到漫反射分量。两个向量之间的角度越大，漫反射分量就会越小。

对于**镜面光照**，它决定于光的方向向量和物体的法向量。同时，它也决定于观察方向。我们通过根据法向量翻折入射光的方向来计算反射向量。然后我们计算反射向量与观察方向的角度差，它们之间夹角越小，镜面光的作用就越大。由此产生的效果就是，我们看向在入射光在表面的反射方向时，会看到一点高光。观察向量是我们计算镜面光照时需要的一个额外变量，我们可以使用观察者的世界空间位置和片段的位置来计算它。之后我们计算出镜面光照强度，用它乘以光源的颜色，并将它与环境光照和漫反射光照部分加和。要得到观察者的世界空间坐标，我们直接使用摄像机的位置向量即可。然后计算镜面分量。

最后把所有分量加起来，乘以物体的颜色，就可以模拟一个模型的光照情况。
### 2. 阴影计算

在片段着色器的最后，计算出阴影元素，当fragment在阴影中时它是1.0，不在阴影中时是0.0。我们使用计算出来的阴影元素去影响光照的diffuse和specular元素。

对于阴影的计算，我们首先计算从片段到光源的方向(片段位置-光源位置) `fragToLight` 。通过将`fragToLight` 作为坐标采样深度贴图 `depthMap`。深度贴图是光源视角下的深度信息，因此 `fragToLight` 被用来从深度贴图中采样并获得该点的深度值。

然后用`closestDepth` 存储从光源视角看该片段的深度（距离光源的距离）。在深度贴图中，深度值通常是归一化的（在 `[0, 1]` 范围内），表示从光源到物体的距离。为了得到实际的深度值，必须将其乘以 `far_plane`，即深度贴图的远裁剪面。这样 `closestDepth` 就转换回了实际的深度（线性深度）。

再计算当前片段到光源的实际距离，即 `currentDepth`。使用 `length(fragToLight)` 计算`fragToLight` 向量的长度，表示片段在世界空间中离光源的实际距离。然后进行阴影检测。为了为了避免“阴影幽灵”效应（Shadow Acne），添加了一个小的 **偏移量** `bias`，这是一个常见的做法，目的是减少深度值比较时因浮动精度导致的错误。再进行**阴影判断**，如果当前片段到光源的实际深度（`currentDepth`）减去偏移量后，仍然大于从深度贴图中获取的深度（`closestDepth`），则表示该片段被阴影遮挡，光源无法照亮该片段。此时，`shadow` 的值为 `1.0`，表示完全阴影；否则，`shadow` 的值为 `0.0`，表示没有阴影。

最后返回阴影值`Shadow`，这是一个布尔值，表示当前片段是否在阴影中。

---
## 2. 草地
本项目中，先创建了一个正方形的图形。然后绑定一张png格式的草地图片进行渲染。

![[Pasted image 20241229215822.png]]
---

## 3. 背包
背包是用开源项目 `LearnOpenGL` 中的obj文件导入的，如下：

![[Pasted image 20241230154240.png]]

---
## 4. 圆环几何体
圆环通过**极坐标系**生成顶点，逻辑如下：

**（1）堆栈方向（纬度方向）：**
- 每个 `stack` 代表圆环在垂直方向的一个切割（纬度方向）。
- 对于每一个堆栈，`stackAngle` 从 `0` 到 `2π` （360度）变化。我们将其视为圆环的“纬度角”。
- 对于每个堆栈，我们计算出该堆栈处的半径 `xy = R + r * cos(stackAngle)`。这是因为：
    - 当 `stackAngle = 0` 或 `stackAngle = π`（上和下极），圆环的半径是最大值 `R + r`。
    - 当 `stackAngle = π/2` 或 `stackAngle = 3π/2`（圆环的中部），圆环的半径是最小值 `R`。
    - 通过这种方式，`xy` 定义了该堆栈切割处的 **圆环半径**。

**（2）经度方向（纵向方向）：**
- 每个 `sector` 表示圆环的一个切割（经度方向）。`sectorAngle` 从 `0` 到 `2π` 变化。
- 对于每个 `sector`，我们通过以下公式计算出顶点的 `x` 和 `y` 坐标：
    - `x = xy * cos(sectorAngle)`：这是通过沿着当前堆栈（纬度）圆形边缘的方向计算的。
    - `y = xy * sin(sectorAngle)`：这是与 `x` 配合，形成一个完整的圆形。

**（3）法向量：**
- 法向量的计算反映了每个顶点的表面法线。由于圆环是一个圆形曲面，其法向量自然是沿着与表面垂直的方向。
- 法向量 `nx, ny, nz` 通过以下方式计算：
    - `nx = cos(stackAngle) * cos(sectorAngle)`
    - `ny = cos(stackAngle) * sin(sectorAngle)`
    - `nz = sin(stackAngle)`
- 这些法向量垂直于曲面并指向外部，因此使得每个顶点的法向量在几何体表面上均匀分布。

这一圆环的顶点生成函数如下。通过传入圆环的外半径、内半径、经度切割数（即切分成多少个扇形）纬度切割数（即切分成多少个堆栈）来进行计算。

```cpp
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
```

在绘制圆环的时候还需要注意，我们还需要绑定**EBO（元素缓冲区对象）**，并通过生成其对应的索引数组来创建VAO，这样才能平滑地绘制出来一个圆环。项目后面的小车轮子也是如此。

**Element Buffer Object（EBO）** 是 OpenGL 中用于存储 **索引数据** 的缓冲区对象，它与 **顶点缓冲区对象（VBO）** 搭配使用。EBO 的主要作用是 **通过索引重用顶点数据**，减少顶点数据的冗余，从而提高渲染性能，特别是当网格包含大量重复顶点时。

同时，本项目中还添加了一个让其**绕y轴**以**固定速度旋转**的功能，代码如下：
```cpp
    float angle = glfwGetTime() * 50.0f; // 用时间控制旋转角度，50.0f 表示每秒旋转 50 度
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); // 绕 y 轴旋转
```

![[Pasted image 20241230154446.png]]

---
## 5. 光源
在此项目中，光源是一个球体。

球体的生成逻辑如下：
**（1）计算每个纬度（堆栈）方向的圆周：**
- 在每个纬度（即 `stackAngle`）上，计算该纬度上的圆的半径 `xy = radius * cos(stackAngle)`，并计算该纬度的高度 `z = radius * sin(stackAngle)`。
    - 当 `stackAngle = π/2`（上极）时，`z = radius`，`xy = 0`。
    - 当 `stackAngle = -π/2`（下极）时，`z = -radius`，`xy = 0`。
    - 当 `stackAngle = 0`（赤道）时，`z = 0`，`xy = radius`。

**（2）在每个纬度上的圆周上生成顶点：**
- 对每个 `sectorAngle`，计算出每个顶点的 `x` 和 `y` 坐标：
    - `x = xy * cos(sectorAngle)`：横坐标。
    - `y = xy * sin(sectorAngle)`：纵坐标。
    - `z` 由纬度角 `stackAngle` 决定，`z = radius * sin(stackAngle)`。

**（3）法向量计算：**
- 球体表面的法向量总是指向球心，因此每个顶点的法向量是从球心到该顶点的向量，经过归一化。
    - 法向量 `nx = x / radius`，`ny = y / radius`，`nz = z / radius`。
    - 这样，法向量的长度为 1，表示该顶点处球面上的法线。

在此项目中，我们让光源绕着整个场景**旋转**，来模拟太阳。通过获取程序运行的时间，将其乘以 `rotationSpeed` 得到角度`angle`，这个角度随着时间不断增大，从而使光源沿着某个路径旋转。然后定义一个半径`radius`。再计算计算光源在 **x** 和 **z** 轴上的位置，使得光源在 **xy平面** 上做**圆形轨迹旋转**。最后再让光源使光源的 `y` 坐标根据时间发生**上下浮动**。

代码如下：
```cpp

// 计算光源的新位置
float angle = glfwGetTime() * rotationSpeed; // 旋转速度
float radius = 2.0f; // 光源旋转半径
// 更新光源位置，绕 (0, 0, 0) 旋转
lightPos = glm::vec3(cos(angle) * radius, 2.0f, sin(angle) * radius);
// 使光源上下浮动
lightPos.y = 0.5f + sin(angle * 0.5f) * 2.0f; // 上下浮动
```

![[Pasted image 20241230154600.png]]

---
## 6. 组合几何体（模拟汽车）
此组合体由一个正方体、长方体，和四个圆柱体组成。模拟了一个小车的形态。

由于正方体、长方体的绘制较为简单，此处省略其介绍。主要介绍轮子（圆柱体）的生成和绘制。

**圆柱体**由三个主要部分组成：顶部圆盖（top cap）、底部圆盖（bottom cap）和侧面（side）。在此项目中，我们通过逐个计算顶点位置，并计算出每个顶点的法线，最后生成索引数据来连接这些顶点。

**顶部圆盖（top cap）**
- **顶点位置**：顶部圆盖位于圆柱体的上半部分，`z = height / 2` 是圆柱体的顶端位置。每个顶点的位置通过极坐标转化为笛卡尔坐标（`x = radius * cosf(sectorAngle)`，`y = radius * sinf(sectorAngle)`）。
- **法线**：顶部的法线是垂直向上的，在 `z` 轴方向上为 `(0.0f, 0.0f, 1.0f)`。
- **`sectorCount`**：确定了圆盖上细分的顶点数量，`sectorStep` 是两个顶点之间的角度差。

**底部圆盖（bottom cap）**
- **顶点位置**：底部圆盖位于圆柱体的下半部分，`z = -height / 2` 是圆柱体的底端位置。与顶部圆盖相同，`x` 和 `y` 也是通过极坐标转化为笛卡尔坐标。
- **法线**：底部的法线是垂直向下的，在 `z` 轴方向上为 `(0.0f, 0.0f, -1.0f)`。
- 其他与顶部圆盖相同，每个顶点的法线方向指向圆柱体的外部。

**侧面（side）**
- **顶点位置**：对于侧面，顶点在圆柱体的上下两部分之间沿 `z` 轴插值，每个 `sectorAngle` 对应一个圆周上的点，`z` 值分别为 `height / 2`（顶部）和 `-height / 2`（底部）。因此，每个切面上有两个顶点，一个在顶部，一个在底部，形成了圆柱体的侧面。
- **法线**：侧面法线与顶点的 `x` 和 `y` 方向相关，因此，法线是指向圆柱体外侧的切线方向。在这里，`x` 和 `y` 的法线是通过 `cosf(sectorAngle)` 和 `sinf(sectorAngle)` 计算的，而 `z` 分量始终为 0，因为侧面是平行于 `xy` 平面的。

**生成索引**
- **顶部圆盖索引**
	- **顶点索引**：顶部圆盖的索引生成方式是通过将中心顶点（顶点 0）与周围的顶点连接，形成三角形。每个三角形连接了一个中心点和两个相邻的顶点。
	- 顶部圆盖的顶点从 `1` 到 `sectorCount + 1`，这些顶点围绕圆周排列。
- **底部圆盖索引**
	- **顶点索引**：底部圆盖的索引生成方式与顶部圆盖类似，底部圆盖的中心点是 `sectorCount + 1`，然后与周围的顶点连接，形成三角形。
- **侧面索引**
	-  **顶点索引**：侧面的索引是通过连接相邻的两个顶点来生成的。每两个相邻的顶点（上半部分和下半部分）形成两个三角形，依次填充整个圆柱体的侧面。
	- `baseIndex = (sectorCount + 1) * 2` 是侧面顶点数组的起始索引位置。

**伪代码**如下：
```cpp
function generateCylinderVertices(radius, height, sectorCount, indices):
    Initialize an empty list vertices
    Set sectorStep = 2 * π / sectorCount  # Calculate the angle step for each sector

    # 1. Top cap vertices
    for i from 0 to sectorCount (inclusive):
        Calculate sectorAngle = i * sectorStep
        Calculate x = radius * cos(sectorAngle)
        Calculate y = radius * sin(sectorAngle)
        Set z = height / 2  # Top of the cylinder
        Add (x, y, z, 0.0, 0.0, 1.0) to vertices  # Position and normal for top cap

    # 2. Bottom cap vertices
    for i from 0 to sectorCount (inclusive):
        Calculate sectorAngle = i * sectorStep
        Calculate x = radius * cos(sectorAngle)
        Calculate y = radius * sin(sectorAngle)
        Set z = -height / 2  # Bottom of the cylinder
        Add (x, y, z, 0.0, 0.0, -1.0) to vertices  # Position and normal for bottom cap

    # 3. Side vertices
    for i from 0 to sectorCount (inclusive):
        Calculate sectorAngle = i * sectorStep
        Calculate x = radius * cos(sectorAngle)
        Calculate y = radius * sin(sectorAngle)
        Set z = height / 2  # Top side of the cylinder
        Add (x, y, z, cos(sectorAngle), sin(sectorAngle), 0.0) to vertices  # Position and normal

        Set z = -height / 2  # Bottom side of the cylinder
        Add (x, y, z, cos(sectorAngle), sin(sectorAngle), 0.0) to vertices  # Position and normal

    # 4. Generate indices for top cap
    for i from 0 to sectorCount - 1:
        Add (0, i+1, i+2) to indices  # Triangle for top cap

    # 5. Generate indices for bottom cap
    Set baseIndex = sectorCount + 1
    for i from 0 to sectorCount - 1:
        Add (baseIndex, baseIndex + i + 1, baseIndex + i + 2) to indices  # Triangle for bottom cap

    # 6. Generate indices for side
    Set baseIndex = (sectorCount + 1) * 2
    for i from 0 to sectorCount - 1:
        Set k1 = baseIndex + i * 2
        Set k2 = k1 + 1
        Set k3 = k1 + 2
        Set k4 = k3 + 1

        Add (k1, k2, k3) to indices  # Triangle for side
        Add (k2, k4, k3) to indices  # Triangle for side

    return vertices

```
对于此小车的生成，对于其各**部分坐标的分配**也是一个需要调试和整理的部分，从而使其成为一个整体，更像现实中的车的形态。

同时，我对这个小车添加了一个可以移动的功能，通过键盘的“上、下、左、右”来控制，并且还给它的头部（粉红色正方体部分）添加了旋转的功能，通过键盘的“1”，“2”来控制向左、向右的旋转。

在代码的实现上，我设了一个车辆的**整体模型矩阵**来控制这一组合体的各个部位同时移动。
```cpp
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
```
并为头部添加了一个**旋转矩阵**，使其实时根据键盘输入来更新旋转角度。
```cpp
// 控制头部旋转
if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
	headRotationAngle -= rotationSpeed;
}
if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
	headRotationAngle += rotationSpeed;
}
```
![[Pasted image 20241230154717.png]]

---
# 四、其它主要代码（伪代码）

**整体场景的渲染函数：**
```cpp
function renderScene(shader, backPack):
# Render plane
model = identity matrix
set shader properties: isLightSource = false, isModel = true, isSkybox = false
set shader "model" to model
bind and draw plane

# Render donut
model = identity matrix
translate and scale model for donut
rotate model based on time
set shader properties: isLightSource = false, isModel = false
set shader "model" and "objectColor"
bind and draw donut

# Render loaded model (backPack)
model = identity matrix
translate and scale model for backPack
set shader properties: isLightSource = false, isModel = true
set shader "model"
draw backPack

# Render vehicle body
model = vehicleModel
translate and scale model for body
set shader properties: isLightSource = false, isModel = false
set shader "model" and "objectColor"
bind and draw vehicle body

# Render vehicle head
model = vehicleModel
translate and rotate model for head
scale model for head
set shader properties: isLightSource = false, isModel = false
set shader "model" and "objectColor"
bind and draw vehicle head

# Render vehicle wheels
for each wheel position:
	model = vehicleModel
	translate and scale model for wheel
	set shader properties: isLightSource = false, isModel = false
	set shader "model" and "objectColor"
	bind and draw wheel

# Render light cube
model = identity matrix
translate and scale model for light cube
set shader properties: isLightSource = true, isModel = true
set shader "model" and "lightColor"
bind and draw light cube
```

**主循环函数**
```cpp
function render():
    # Step 1: Clear the screen
    clear color and depth buffer with a dark color

    # Step 2: Create depth cubemap transformation matrices
    near_plane = 1.0
    far_plane = 25.0
    shadowProj = create perspective matrix with 90 degrees FOV
    shadowTransforms = []
    for each direction (6 directions):
        compute view matrix for the light source and store the transformation in shadowTransforms

    # Step 3: Render scene to depth cubemap
    set viewport to shadow map resolution
    bind depth framebuffer
    clear depth buffer
    set depth function to LEQUAL
    use depth shader
    for each shadow matrix in shadowTransforms:
        set corresponding matrix in shader
    set far plane and light position in shader
    render scene to depth cubemap

    # Step 4: Render scene as normal
    set viewport to screen resolution
    clear color and depth buffer
    use shadow shader
    compute projection and view matrices
    set projection and view matrices in shader
    set lighting uniforms in shader
    enable shadows in shader
    set light and view position, far plane, light color
    bind textures (grass map and depth cubemap)
    render the scene with lighting and shadows applied
```
---
# 五、可改进的地方
- **提高性能**：可以通过优化深度贴图的分辨率、阴影采样数量等参数来提高渲染效率。
- **更多物体的引入**：除了草地和背包，还可以引入更多复杂的模型（例如树木、建筑等）来丰富场景。
- **增强光照模型**：可以尝试更复杂的光照模型（如PBR）来进一步提升真实感。
- **细节优化**：如增加物体表面的细节纹理，改善草地的动态效果（例如风吹草动）。
