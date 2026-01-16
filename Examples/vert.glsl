#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant) uniform PushConsts
{
    uint storageBufferIndex;
    uint uniformBufferIndex;
} pushConsts;

layout(set = 3, binding = 0) uniform GlobalUniformParam
{
    float globalTime;
    float globalScale;
    uint frameCount;
    uint padding;
} globalParams[];

layout(set = 1, binding = 0) readonly buffer StorageBufferObject
{
    uint textureIndex;
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 viewPos;
    vec3 lightColor;
    vec3 lightPos;
} storageBufferObjects[];

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
    mat4 scaledModel = mat4(vec4(storageBufferObjects[pushConsts.storageBufferIndex].model[0].xyz * globalParams[pushConsts.uniformBufferIndex].globalScale, storageBufferObjects[pushConsts.storageBufferIndex].model[0].w),
                            vec4(storageBufferObjects[pushConsts.storageBufferIndex].model[1].xyz * globalParams[pushConsts.uniformBufferIndex].globalScale, storageBufferObjects[pushConsts.storageBufferIndex].model[1].w),
                            vec4(storageBufferObjects[pushConsts.storageBufferIndex].model[2].xyz * globalParams[pushConsts.uniformBufferIndex].globalScale, storageBufferObjects[pushConsts.storageBufferIndex].model[2].w),
                            storageBufferObjects[pushConsts.storageBufferIndex].model[3]);   

    gl_Position = storageBufferObjects[pushConsts.storageBufferIndex].proj * 
                  storageBufferObjects[pushConsts.storageBufferIndex].view * 
                  scaledModel * 
                  vec4(inPosition, 1.0);           

    fragPos = vec3(scaledModel * vec4(inPosition, 1.0));
    fragNormal = normalize(mat3(transpose(inverse(scaledModel))) * inNormal);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}