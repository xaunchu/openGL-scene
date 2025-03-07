#version 330 core

layout (location = 0) in vec3 aPos;

uniform float uTime;       // 时间
uniform float waveAmplitude;  // 波浪幅度
uniform float waveLength;     // 波长
uniform float waveSpeed;      // 波速

void main() {
    // 基于 x 和 z 计算波浪的 y 值
    float k = 2.0 * 3.14159 / waveLength; // 波数
    float omega = waveSpeed * k;          // 角速度
    float y = waveAmplitude * sin(k * aPos.x + k * aPos.z - omega * uTime);

    // 更新顶点位置
    vec3 newPos = vec3(aPos.x, y, aPos.z);
    gl_Position = vec4(newPos, 1.0);
}