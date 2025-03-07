#顶点着色器代码
# 该文件负责处理顶点数据并将其传递给片段着色器

#version 330 core
layout (location = 0) in vec3 aPos; // 位置属性
layout (location = 1) in vec3 aNormal; // 法线属性

out vec3 FragPos; // 传递给片段着色器的片段位置
out vec3 Normal; // 传递给片段着色器的法线

uniform mat4 model; // 模型矩阵
uniform mat4 view; // 视图矩阵
uniform mat4 projection; // 投影矩阵

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // 计算片段位置
    Normal = mat3(transpose(inverse(model))) * aNormal; // 计算法线
    gl_Position = projection * view * vec4(FragPos, 1.0); // 计算最终位置
}