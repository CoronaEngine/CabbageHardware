#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 1, binding = 0) readonly buffer StorageBufferObject
{
    uint textureIndex;
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 viewPos;
    vec3 lightColor;
    vec3 lightPos;
} storageBufferObject;

layout(set = 0, binding = 0) uniform sampler2D textures[];

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 inColor;

layout(location = 0) out vec4 outColor;

// 简单的 Blinn-Phong 光照计算
vec3 calculateSimpleLighting(vec3 WorldPos, vec3 Normal, vec3 albedo)
{
    // 1. 环境光 (Ambient)
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * albedo;

    // 2. 漫反射 (Diffuse)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(storageBufferObject.lightPos - WorldPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * storageBufferObject.lightColor * albedo;

    // 3. 镜面高光 (Specular)
    float specularStrength = 0.5;
    vec3 viewDir = normalize(storageBufferObject.viewPos - WorldPos);
    vec3 halfDir = normalize(lightDir + viewDir);  // Blinn-Phong 使用半程向量
    float spec = pow(max(dot(norm, halfDir), 0.0), 32.0); // 32.0 为高光反光度 (shininess)
    vec3 specular = specularStrength * spec * storageBufferObject.lightColor;

    // 合并结果
    return ambient + diffuse + specular;
}

void main()
{
    // 获取纹理颜色
    vec4 color = textureLod(textures[storageBufferObject.textureIndex], fragTexCoord, 0.0);
    
    // 如果纹理 alpha 极小，则退化使用顶点颜色，否则使用纹理颜色作为漫反射基础色 (albedo)
    vec3 albedo = color.w > 0.01 ? color.xyz : inColor;
    
    // 计算光照
    vec3 finalColor = calculateSimpleLighting(inPosition, inNormal, albedo);
    
    outColor = vec4(finalColor, 1.0);
}