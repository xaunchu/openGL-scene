#version 330 core
out vec4 FragColor;

// ��Դ��ɫ��Ϊ uniform ����
uniform vec3 lightColor;

void main()
{
    // ʹ�ô���Ĺ�Դ��ɫ
    FragColor = vec4(lightColor, 1.0); // RGB ʹ�� lightColor��Alpha �̶�Ϊ 1.0
}
