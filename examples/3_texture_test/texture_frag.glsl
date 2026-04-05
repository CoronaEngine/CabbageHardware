#version 450

layout(location = 0) in vec3 ourColor;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D texture1;

void main()
{
    FragColor = texture(texture1, TexCoord);
    // 或者：FragColor = texture(texture1, TexCoord) * vec4(ourColor, 1.0);
}
