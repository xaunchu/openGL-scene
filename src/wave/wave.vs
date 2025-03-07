#version 330 core

layout (location = 0) in vec3 aPos;

uniform float uTime;       // ʱ��
uniform float waveAmplitude;  // ���˷���
uniform float waveLength;     // ����
uniform float waveSpeed;      // ����

void main() {
    // ���� x �� z ���㲨�˵� y ֵ
    float k = 2.0 * 3.14159 / waveLength; // ����
    float omega = waveSpeed * k;          // ���ٶ�
    float y = waveAmplitude * sin(k * aPos.x + k * aPos.z - omega * uTime);

    // ���¶���λ��
    vec3 newPos = vec3(aPos.x, y, aPos.z);
    gl_Position = vec4(newPos, 1.0);
}