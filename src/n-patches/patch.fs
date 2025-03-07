#version 330 core
in vec3 fragPosition;
in vec3 fragNormal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec3 lightDir = normalize(lightPos - fragPosition);
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 reflectDir = reflect(-lightDir, fragNormal);

    // 漫反射
    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 0.5, 0.3);

    // 高光
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * vec3(1.0);

    // 环境光
    vec3 ambient = 0.1 * vec3(1.0);

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
