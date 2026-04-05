#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(push_constant) uniform TexturePush
{
    uvec2 textureHandle;
} pc;

layout(set = 0, binding = 0) uniform sampler2D combinedTextureSamplerHandles[];

layout(location = 0) in vec3 ourColor;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = texture(combinedTextureSamplerHandles[nonuniformEXT(pc.textureHandle.x)], TexCoord) * vec4(ourColor, 1.0);
}
