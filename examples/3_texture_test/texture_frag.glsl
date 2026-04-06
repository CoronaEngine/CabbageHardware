#version 450

layout(location = 0) in vec2 TexCoord;
layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 0) uniform sampler2D texture1;

void main()
{
    vec2 uv = clamp(TexCoord, vec2(0.0), vec2(1.0));
    FragColor = texture(texture1, uv);
}
