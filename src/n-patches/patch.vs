#version 330 core
layout(location = 0) in vec3 inPosition; // 三角形顶点
layout(location = 1) in vec3 inNormal;   // 顶点法向量

out vec3 vPosition;
out vec3 vNormal;

void main() {
    vPosition = inPosition;
    vNormal = inNormal;
    gl_Position = vec4(inPosition, 1.0);
}
