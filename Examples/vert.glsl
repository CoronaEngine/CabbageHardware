#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant) uniform PushConsts
{
    uint textureIndex;
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 viewPos;
    vec3 lightColor;
    vec3 lightPos;
} pushConsts;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inColor;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragColor;

void main()
{
    gl_Position = pushConsts.proj * pushConsts.view * pushConsts.model * vec4(inPosition, 1.0);

    fragPos = vec3(pushConsts.model * vec4(inPosition, 1.0));

    fragNormal = normalize(mat3(transpose(inverse(pushConsts.model))) * inNormal);

    fragColor = inColor;

    fragTexCoord = inTexCoord;
}