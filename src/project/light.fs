#version 330 core
out vec4 FragColor;

// 光源颜色作为 uniform 输入
uniform vec3 lightColor;

void main()
{
    // 使用传入的光源颜色
    FragColor = vec4(lightColor, 1.0); // RGB 使用 lightColor，Alpha 固定为 1.0
}
