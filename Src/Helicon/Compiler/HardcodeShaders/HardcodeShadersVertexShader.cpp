#include"HardcodeShaders.h"
std::unordered_map<std::string, std::variant<EmbeddedShader::ShaderCodeModule::ShaderResources,std::variant<std::vector<uint32_t>,std::string>>> EmbeddedShader::HardcodeShaders::hardcodeShadersVertexShader = {{"SpirV_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_258_column_54", std::vector<uint32_t>{119734787,67072,524299,180,0,131089,1,131089,5302,393227,1,1280527431,1685353262,808793134,0,196622,0,1,1114127,0,4,1852399981,0,16,19,33,119,133,143,155,167,171,172,176,178,196611,2,450,524292,1163873351,1851741272,1853189743,1919903337,1970364269,1718185057,7497065,262149,4,1852399981,0,327685,10,1818321779,1867342949,7103844,458757,13,1919906899,1113941857,1701209717,1784827762,7627621,458758,13,0,1954047348,1231385205,2019910766,0,327686,13,1,1701080941,108,327686,13,2,2003134838,0,327686,13,3,1785688688,0,327686,13,4,2003134838,7565136,393222,13,5,1751607660,1819231092,29295,393222,13,6,1751607660,1936674932,0,524293,16,1919906931,1113941857,1701209717,1784827762,1937007461,0,327685,17,1752397136,1936617283,29556,524294,17,0,1919906931,1113941857,1701209717,1684949362,30821,524294,17,1,1718185589,1114468975,1701209717,1684949362,30821,327685,19,1752397168,1936617283,29556,458757,30,1651469383,1851092065,1919903337,1918980205,28001,393222,30,0,1651469415,1767140449,25965,393222,30,1,1651469415,1666411617,6646881,393222,30,2,1835102822,1970226021,29806,327686,30,3,1684300144,6778473,393221,33,1651469415,1632660577,1936548210,0,393221,117,1348430951,1700164197,2019914866,0,393222,117,0,1348430951,1953067887,7237481,458758,117,1,1348430951,1953393007,1702521171,0,458758,117,2,1130327143,1148217708,1635021673,6644590,458758,117,3,1130327143,1147956341,1635021673,6644590,196613,119,0,327685,133,1867542121,1769236851,28271,262149,143,1734439526,7565136,327685,155,1734439526,1836216142,27745,327685,167,1867411049,1818324338,0,327685,171,1734439526,1869377347,114,262149,172,1866690153,7499628,393221,176,1734439526,1131963732,1685221231,0,327685,178,1700032105,1869562744,25714,196679,13,2,262216,13,0,24,327752,13,0,35,0,262216,13,1,5,327752,13,1,7,16,262216,13,1,24,327752,13,1,35,16,262216,13,2,5,327752,13,2,7,16,262216,13,2,24,327752,13,2,35,80,262216,13,3,5,327752,13,3,7,16,262216,13,3,24,327752,13,3,35,144,262216,13,4,24,327752,13,4,35,208,262216,13,5,24,327752,13,5,35,224,262216,13,6,24,327752,13,6,35,240,196679,16,24,262215,16,33,0,262215,16,34,1,196679,17,2,327752,17,0,35,0,327752,17,1,35,4,196679,30,2,327752,30,0,35,0,327752,30,1,35,4,327752,30,2,35,8,327752,30,3,35,12,262215,33,33,0,262215,33,34,3,196679,117,2,327752,117,0,11,0,327752,117,1,11,1,327752,117,2,11,3,327752,117,3,11,4,262215,133,30,0,262215,143,30,0,262215,155,30,1,262215,167,30,1,262215,171,30,3,262215,172,30,3,262215,176,30,2,262215,178,30,2,131091,2,196641,3,2,196630,6,32,262167,7,6,4,262168,8,7,4,262176,9,7,8,262165,11,32,0,262167,12,6,3,589854,13,11,8,8,8,12,12,12,196637,14,13,262176,15,12,14,262203,15,16,12,262174,17,11,11,262176,18,9,17,262203,18,19,9,262165,20,32,1,262187,20,21,0,262176,22,9,11,262187,20,25,1,262176,26,12,7,393246,30,6,6,11,11,196637,31,30,262176,32,2,31,262203,32,33,2,262176,36,2,6,262187,11,42,3,262176,43,12,6,262187,20,70,2,262187,20,89,3,262187,6,92,1065353216,262187,6,93,0,262187,11,115,1,262172,116,6,115,393246,117,7,6,116,116,262176,118,3,117,262203,118,119,3,262176,122,12,8,262176,132,1,12,262203,132,133,1,262176,140,3,7,262176,142,3,12,262203,142,143,3,262203,142,155,3,262168,159,12,3,262203,132,167,1,262203,142,171,3,262203,132,172,1,262167,174,6,2,262176,175,3,174,262203,175,176,3,262176,177,1,174,262203,177,178,1,327734,2,4,0,3,131320,5,262203,9,10,7,327745,22,23,19,21,262205,11,24,23,458817,26,27,16,24,25,21,262205,7,28,27,524367,12,29,28,28,0,1,2,327745,22,34,19,25,262205,11,35,34,393281,36,37,33,35,25,262205,6,38,37,327822,12,39,29,38,327745,22,40,19,21,262205,11,41,40,524353,43,44,16,41,25,21,42,262205,6,45,44,327761,6,46,39,0,327761,6,47,39,1,327761,6,48,39,2,458832,7,49,46,47,48,45,327745,22,50,19,21,262205,11,51,50,458817,26,52,16,51,25,25,262205,7,53,52,524367,12,54,53,53,0,1,2,327745,22,55,19,25,262205,11,56,55,393281,36,57,33,56,25,262205,6,58,57,327822,12,59,54,58,327745,22,60,19,21,262205,11,61,60,524353,43,62,16,61,25,25,42,262205,6,63,62,327761,6,64,59,0,327761,6,65,59,1,327761,6,66,59,2,458832,7,67,64,65,66,63,327745,22,68,19,21,262205,11,69,68,458817,26,71,16,69,25,70,262205,7,72,71,524367,12,73,72,72,0,1,2,327745,22,74,19,25,262205,11,75,74,393281,36,76,33,75,25,262205,6,77,76,327822,12,78,73,77,327745,22,79,19,21,262205,11,80,79,524353,43,81,16,80,25,70,42,262205,6,82,81,327761,6,83,78,0,327761,6,84,78,1,327761,6,85,78,2,458832,7,86,83,84,85,82,327745,22,87,19,21,262205,11,88,87,458817,26,90,16,88,25,89,262205,7,91,90,327761,6,94,49,0,327761,6,95,49,1,327761,6,96,49,2,327761,6,97,49,3,327761,6,98,67,0,327761,6,99,67,1,327761,6,100,67,2,327761,6,101,67,3,327761,6,102,86,0,327761,6,103,86,1,327761,6,104,86,2,327761,6,105,86,3,327761,6,106,91,0,327761,6,107,91,1,327761,6,108,91,2,327761,6,109,91,3,458832,7,110,94,95,96,97,458832,7,111,98,99,100,101,458832,7,112,102,103,104,105,458832,7,113,106,107,108,109,458832,8,114,110,111,112,113,196670,10,114,327745,22,120,19,21,262205,11,121,120,393281,122,123,16,121,89,262205,8,124,123,327745,22,125,19,21,262205,11,126,125,393281,122,127,16,126,70,262205,8,128,127,327826,8,129,124,128,262205,8,130,10,327826,8,131,129,130,262205,12,134,133,327761,6,135,134,0,327761,6,136,134,1,327761,6,137,134,2,458832,7,138,135,136,137,92,327825,7,139,131,138,327745,140,141,119,21,196670,141,139,262205,8,144,10,262205,12,145,133,327761,6,146,145,0,327761,6,147,145,1,327761,6,148,145,2,458832,7,149,146,147,148,92,327825,7,150,144,149,327761,6,151,150,0,327761,6,152,150,1,327761,6,153,150,2,393296,12,154,151,152,153,196670,143,154,262205,8,156,10,393228,8,157,1,34,156,262228,8,158,157,327761,7,160,158,0,524367,12,161,160,160,0,1,2,327761,7,162,158,1,524367,12,163,162,162,0,1,2,327761,7,164,158,2,524367,12,165,164,164,0,1,2,393296,159,166,161,163,165,262205,12,168,167,327825,12,169,166,168,393228,12,170,1,69,169,196670,155,170,262205,12,173,172,196670,171,173,262205,174,179,178,196670,176,179,65789,65592,}},{"SpirV_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_258_column_54", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"fragPos",{0,0,0,"","fragPos","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragColor",{0,0,3,"","fragColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inTexCoord",{0,0,2,"","inTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragNormal",{0,0,1,"","fragNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},{"pushConsts.uniformBufferIndex",{0,0,0,"","uniformBufferIndex","",0,4,4,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"GLSL_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_258_column_54", R"(#version 460
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 1, binding = 0, std430) readonly buffer StorageBufferObject
{
    uint textureIndex;
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 viewPos;
    vec3 lightColor;
    vec3 lightPos;
} storageBufferObjects[];

layout(set = 3, binding = 0, std140) uniform GlobalUniformParam
{
    float globalTime;
    float globalScale;
    uint frameCount;
    uint padding;
} globalParams[];

layout(push_constant, std430) uniform PushConsts
{
    uint storageBufferIndex;
    uint uniformBufferIndex;
} pushConsts;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 1) in vec3 inNormal;
layout(location = 3) out vec3 fragColor;
layout(location = 3) in vec3 inColor;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 2) in vec2 inTexCoord;

void main()
{
    mat4 scaledModel = mat4(vec4(vec4(storageBufferObjects[pushConsts.storageBufferIndex].model[0].xyz * globalParams[pushConsts.uniformBufferIndex].globalScale, storageBufferObjects[pushConsts.storageBufferIndex].model[0].w)), vec4(vec4(storageBufferObjects[pushConsts.storageBufferIndex].model[1].xyz * globalParams[pushConsts.uniformBufferIndex].globalScale, storageBufferObjects[pushConsts.storageBufferIndex].model[1].w)), vec4(vec4(storageBufferObjects[pushConsts.storageBufferIndex].model[2].xyz * globalParams[pushConsts.uniformBufferIndex].globalScale, storageBufferObjects[pushConsts.storageBufferIndex].model[2].w)), vec4(storageBufferObjects[pushConsts.storageBufferIndex].model[3]));
    gl_Position = ((storageBufferObjects[pushConsts.storageBufferIndex].proj * storageBufferObjects[pushConsts.storageBufferIndex].view) * scaledModel) * vec4(inPosition, 1.0);
    fragPos = vec3((scaledModel * vec4(inPosition, 1.0)).xyz);
    mat4 _158 = transpose(inverse(scaledModel));
    fragNormal = normalize(mat3(_158[0].xyz, _158[1].xyz, _158[2].xyz) * inNormal);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}

)"},{"GLSL_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_258_column_54", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"fragPos",{0,0,0,"","fragPos","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragColor",{0,0,3,"","fragColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inTexCoord",{0,0,2,"","inTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragNormal",{0,0,1,"","fragNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},{"pushConsts.uniformBufferIndex",{0,0,0,"","uniformBufferIndex","",0,4,4,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"HLSL_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_258_column_54", R"(ByteAddressBuffer storageBufferObjects[] : register(t0, space1);
struct GlobalUniformParam_1_1
{
    float globalTime;
    float globalScale;
    uint frameCount;
    uint padding;
};

ConstantBuffer<GlobalUniformParam_1_1> globalParams[] : register(b0, space3);
cbuffer PushConsts
{
    uint pushConsts_storageBufferIndex : packoffset(c0);
    uint pushConsts_uniformBufferIndex : packoffset(c0.y);
};


static float4 gl_Position;
static float3 inPosition;
static float3 fragPos;
static float3 fragNormal;
static float3 inNormal;
static float3 fragColor;
static float3 inColor;
static float2 fragTexCoord;
static float2 inTexCoord;

struct SPIRV_Cross_Input
{
    float3 inPosition : TEXCOORD0;
    float3 inNormal : TEXCOORD1;
    float2 inTexCoord : TEXCOORD2;
    float3 inColor : TEXCOORD3;
};

struct SPIRV_Cross_Output
{
    float3 fragPos : TEXCOORD0;
    float3 fragNormal : TEXCOORD1;
    float2 fragTexCoord : TEXCOORD2;
    float3 fragColor : TEXCOORD3;
    float4 gl_Position : SV_Position;
};

// Returns the determinant of a 2x2 matrix.
float spvDet2x2(float a1, float a2, float b1, float b2)
{
    return a1 * b2 - b1 * a2;
}

// Returns the determinant of a 3x3 matrix.
float spvDet3x3(float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2, float c3)
{
    return a1 * spvDet2x2(b2, b3, c2, c3) - b1 * spvDet2x2(a2, a3, c2, c3) + c1 * spvDet2x2(a2, a3, b2, b3);
}

// Returns the inverse of a matrix, by using the algorithm of calculating the classical
// adjoint and dividing by the determinant. The contents of the matrix are changed.
float4x4 spvInverse(float4x4 m)
{
    float4x4 adj;	// The adjoint matrix (inverse after dividing by determinant)

    // Create the transpose of the cofactors, as the classical adjoint of the matrix.
    adj[0][0] =  spvDet3x3(m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], m[3][3]);
    adj[0][1] = -spvDet3x3(m[0][1], m[0][2], m[0][3], m[2][1], m[2][2], m[2][3], m[3][1], m[3][2], m[3][3]);
    adj[0][2] =  spvDet3x3(m[0][1], m[0][2], m[0][3], m[1][1], m[1][2], m[1][3], m[3][1], m[3][2], m[3][3]);
    adj[0][3] = -spvDet3x3(m[0][1], m[0][2], m[0][3], m[1][1], m[1][2], m[1][3], m[2][1], m[2][2], m[2][3]);

    adj[1][0] = -spvDet3x3(m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], m[3][3]);
    adj[1][1] =  spvDet3x3(m[0][0], m[0][2], m[0][3], m[2][0], m[2][2], m[2][3], m[3][0], m[3][2], m[3][3]);
    adj[1][2] = -spvDet3x3(m[0][0], m[0][2], m[0][3], m[1][0], m[1][2], m[1][3], m[3][0], m[3][2], m[3][3]);
    adj[1][3] =  spvDet3x3(m[0][0], m[0][2], m[0][3], m[1][0], m[1][2], m[1][3], m[2][0], m[2][2], m[2][3]);

    adj[2][0] =  spvDet3x3(m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], m[3][3]);
    adj[2][1] = -spvDet3x3(m[0][0], m[0][1], m[0][3], m[2][0], m[2][1], m[2][3], m[3][0], m[3][1], m[3][3]);
    adj[2][2] =  spvDet3x3(m[0][0], m[0][1], m[0][3], m[1][0], m[1][1], m[1][3], m[3][0], m[3][1], m[3][3]);
    adj[2][3] = -spvDet3x3(m[0][0], m[0][1], m[0][3], m[1][0], m[1][1], m[1][3], m[2][0], m[2][1], m[2][3]);

    adj[3][0] = -spvDet3x3(m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], m[3][2]);
    adj[3][1] =  spvDet3x3(m[0][0], m[0][1], m[0][2], m[2][0], m[2][1], m[2][2], m[3][0], m[3][1], m[3][2]);
    adj[3][2] = -spvDet3x3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[3][0], m[3][1], m[3][2]);
    adj[3][3] =  spvDet3x3(m[0][0], m[0][1], m[0][2], m[1][0], m[1][1], m[1][2], m[2][0], m[2][1], m[2][2]);

    // Calculate the determinant as a combination of the cofactors of the first row.
    float det = (adj[0][0] * m[0][0]) + (adj[0][1] * m[1][0]) + (adj[0][2] * m[2][0]) + (adj[0][3] * m[3][0]);

    // Divide the classical adjoint matrix by the determinant.
    // If determinant is zero, matrix is not invertable, so leave it unchanged.
    return (det != 0.0f) ? (adj * (1.0f / det)) : m;
}

void vert_main()
{
    float4x4 scaledModel = float4x4(float4(float4(storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(16).xyz * globalParams[pushConsts_uniformBufferIndex].globalScale, storageBufferObjects[pushConsts_storageBufferIndex].Load<float>(28))), float4(float4(storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(32).xyz * globalParams[pushConsts_uniformBufferIndex].globalScale, storageBufferObjects[pushConsts_storageBufferIndex].Load<float>(44))), float4(float4(storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(48).xyz * globalParams[pushConsts_uniformBufferIndex].globalScale, storageBufferObjects[pushConsts_storageBufferIndex].Load<float>(60))), float4(storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(64)));
    float4x4 _124 = float4x4(storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(144), storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(160), storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(176), storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(192));
    float4x4 _128 = float4x4(storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(80), storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(96), storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(112), storageBufferObjects[pushConsts_storageBufferIndex].Load<float4>(128));
    gl_Position = mul(float4(inPosition, 1.0f), mul(scaledModel, mul(_128, _124)));
    fragPos = float3(mul(float4(inPosition, 1.0f), scaledModel).xyz);
    float4x4 _158 = transpose(spvInverse(scaledModel));
    fragNormal = normalize(mul(inNormal, float3x3(_158[0].xyz, _158[1].xyz, _158[2].xyz)));
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    inPosition = stage_input.inPosition;
    inNormal = stage_input.inNormal;
    inColor = stage_input.inColor;
    inTexCoord = stage_input.inTexCoord;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.fragPos = fragPos;
    stage_output.fragNormal = fragNormal;
    stage_output.fragColor = fragColor;
    stage_output.fragTexCoord = fragTexCoord;
    return stage_output;
}
)"},{"HLSL_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_258_column_54", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"fragPos",{0,0,0,"","fragPos","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragColor",{0,0,3,"","fragColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inTexCoord",{0,0,2,"","inTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragNormal",{0,0,1,"","fragNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},{"pushConsts.uniformBufferIndex",{0,0,0,"","uniformBufferIndex","",0,4,4,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"SpirV_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", std::vector<uint32_t>{119734787,66816,2621440,169,0,131089,1,196622,0,1,786447,0,2,1852399981,0,55,153,158,161,165,168,15,196611,11,1,524293,15,1970302569,1635135092,774922098,1867542121,1769236851,28271,786437,49,1952533855,1400400242,1634889588,1717527911,1952542572,1597274164,1298952003,1919904353,828666995,12340,327686,49,0,1635017060,0,786437,52,1952533855,1400400242,1634889588,1717527911,1952542572,1597208627,1298952003,1919904353,828666995,12340,327686,52,0,1635017060,0,589829,48,1651469415,1969187937,1935634274,1668641396,1953718132,808726884,0,458758,48,0,1651469415,1985965153,811561569,0,458758,48,1,1651469415,1985965153,828338785,0,458758,48,2,1651469415,1985965153,845116001,0,458758,48,3,1651469415,1985965153,861893217,0,458758,48,4,1651469415,1985965153,878670433,0,458758,48,5,1651469415,1985965153,895447649,0,458758,48,6,1651469415,1985965153,912224865,0,720901,55,1651469415,1885301857,1835102817,1919251557,1869374047,1731095395,1633841004,1651859308,111,720901,158,1920233061,1768902777,1632662638,1601003890,1852399981,1918989870,1714304607,1348952434,29551,786437,161,1920233061,1768902777,1632662638,1601003890,1852399981,1918989870,1714304607,1315398002,1634562671,108,786437,165,1920233061,1768902777,1632662638,1601003890,1852399981,1918989870,1714304607,1416061298,1866692709,6582895,786437,168,1920233061,1768902777,1632662638,1601003890,1852399981,1918989870,1714304607,1130848626,1919904879,0,262149,2,1852399981,0,262215,15,30,0,262215,50,6,16,327752,49,0,35,0,262215,53,6,16,327752,52,0,35,0,196679,48,2,327752,48,0,35,0,327752,48,1,35,64,327752,48,2,35,112,327752,48,3,35,176,327752,48,4,35,240,327752,48,5,35,256,327752,48,6,35,272,262215,55,33,0,262215,55,34,0,262215,153,11,0,262215,158,30,0,262215,161,30,1,262215,165,30,2,262215,168,30,3,131091,1,196641,3,1,196630,6,32,262167,7,6,4,262167,9,6,3,262167,10,6,2,262176,14,1,9,262165,26,32,1,262187,26,27,0,262187,26,34,2,262187,26,38,3,262187,6,44,0,393260,9,43,44,44,44,327724,10,45,44,44,262187,26,51,4,262172,50,7,51,196638,49,50,262172,53,9,38,196638,52,53,589854,48,49,52,49,49,9,9,9,262176,54,2,48,262176,56,2,49,262168,84,7,4,262187,6,145,1065353216,262176,152,3,7,262176,157,3,9,262176,164,3,10,262203,14,15,1,262203,54,55,2,262203,152,153,3,262203,157,158,3,262203,157,161,3,262203,164,165,3,262203,157,168,3,327734,1,2,0,3,131320,4,262205,9,13,15,327746,56,57,55,38,262205,49,58,57,327761,50,59,58,0,327761,7,60,59,0,327761,6,61,60,0,327761,6,62,60,1,327761,6,63,60,2,327761,6,64,60,3,327761,7,65,59,1,327761,6,66,65,0,327761,6,67,65,1,327761,6,68,65,2,327761,6,69,65,3,327761,7,70,59,2,327761,6,71,70,0,327761,6,72,70,1,327761,6,73,70,2,327761,6,74,70,3,327761,7,75,59,3,327761,6,76,75,0,327761,6,77,75,1,327761,6,78,75,2,327761,6,79,75,3,458832,7,80,61,66,71,76,458832,7,81,62,67,72,77,458832,7,82,63,68,73,78,458832,7,83,64,69,74,79,458832,84,85,80,81,82,83,327746,56,86,55,34,262205,49,87,86,327761,50,88,87,0,327761,7,89,88,0,327761,6,90,89,0,327761,6,91,89,1,327761,6,92,89,2,327761,6,93,89,3,327761,7,94,88,1,327761,6,95,94,0,327761,6,96,94,1,327761,6,97,94,2,327761,6,98,94,3,327761,7,99,88,2,327761,6,100,99,0,327761,6,101,99,1,327761,6,102,99,2,327761,6,103,99,3,327761,7,104,88,3,327761,6,105,104,0,327761,6,106,104,1,327761,6,107,104,2,327761,6,108,104,3,458832,7,109,90,95,100,105,458832,7,110,91,96,101,106,458832,7,111,92,97,102,107,458832,7,112,93,98,103,108,458832,84,113,109,110,111,112,327826,84,114,113,85,327746,56,115,55,27,262205,49,116,115,327761,50,117,116,0,327761,7,118,117,0,327761,6,119,118,0,327761,6,120,118,1,327761,6,121,118,2,327761,6,122,118,3,327761,7,123,117,1,327761,6,124,123,0,327761,6,125,123,1,327761,6,126,123,2,327761,6,127,123,3,327761,7,128,117,2,327761,6,129,128,0,327761,6,130,128,1,327761,6,131,128,2,327761,6,132,128,3,327761,7,133,117,3,327761,6,134,133,0,327761,6,135,133,1,327761,6,136,133,2,327761,6,137,133,3,458832,7,138,119,124,129,134,458832,7,139,120,125,130,135,458832,7,140,121,126,131,136,458832,7,141,122,127,132,137,458832,84,142,138,139,140,141,327826,84,143,142,114,327760,7,144,13,145,327824,7,146,144,143,196670,153,146,196670,158,43,196670,161,43,196670,165,45,196670,168,43,65789,65592,}},{"SpirV_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_var_2",{0,0,0,"","global_var_2","matrix",0,64,112,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_ubo",{0,0,0,"","global_ubo","ConstantBuffer",0,288,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"global_var_0",{0,0,0,"","global_var_0","matrix",0,64,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_1",{0,0,0,"","global_var_1","matrix",0,48,64,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_3",{0,0,0,"","global_var_3","matrix",0,64,176,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inColor",{0,0,3,"LOCATION","inColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_4",{0,0,0,"","global_var_4","vector",0,12,240,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_5",{0,0,0,"","global_var_5","vector",0,12,256,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_6",{0,0,0,"","global_var_6","vector",0,12,272,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_7.texture",{0,0,0,"","texture","_Texture",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(5)}},{"global_var_7.sampler",{0,0,0,"","sampler","SamplerState",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(6)}},{"input.var_0.inPosition",{0,0,0,"LOCATION","inPosition","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inNormal",{0,0,1,"LOCATION","inNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inTexCoord",{0,0,2,"LOCATION","inTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"GLSL_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", R"(#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 12144 0
struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    vec4  data_0[4];
};


#line 5 1
struct _MatrixStorage_float3x3_ColMajorstd140_0
{
    vec3  data_1[3];
};


#line 24
struct global_ubo_struct_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_0_0;
    _MatrixStorage_float3x3_ColMajorstd140_0 global_var_1_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_2_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_3_0;
    vec3 global_var_4_0;
    vec3 global_var_5_0;
    vec3 global_var_6_0;
};


#line 3
layout(binding = 0)
layout(std140) uniform block_global_ubo_struct_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_0_0;
    _MatrixStorage_float3x3_ColMajorstd140_0 global_var_1_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_2_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_3_0;
    vec3 global_var_4_0;
    vec3 global_var_5_0;
    vec3 global_var_6_0;
}global_parameter_block_global_ubo_0;

#line 11
struct aggregate_type_2_0
{
    vec3 fragPos_0;
    vec3 fragNormal_0;
    vec2 fragTexCoord_0;
    vec3 fragColor_0;
};


#line 11
aggregate_type_2_0 aggregate_type_2_x24init_0(vec3 fragPos_1, vec3 fragNormal_1, vec2 fragTexCoord_1, vec3 fragColor_1)
{

#line 11
    aggregate_type_2_0 _S1;
    _S1.fragPos_0 = fragPos_1;
    _S1.fragNormal_0 = fragNormal_1;
    _S1.fragTexCoord_0 = fragTexCoord_1;
    _S1.fragColor_0 = fragColor_1;

#line 11
    return _S1;
}


#line 12144 0
layout(location = 0)
out vec3 entryPointParam_main_var_6_fragPos_0;


#line 12144
layout(location = 1)
out vec3 entryPointParam_main_var_6_fragNormal_0;


#line 12144
layout(location = 2)
out vec2 entryPointParam_main_var_6_fragTexCoord_0;


#line 12144
layout(location = 3)
out vec3 entryPointParam_main_var_6_fragColor_0;


#line 12144
layout(location = 0)
in vec3 input_var_0_inPosition_0;


#line 34 1
struct vertex_output_0
{
    vec4 position_output_0;
    aggregate_type_2_0 var_6_0;
};


#line 39
void main()
{

#line 12
    const vec3 _S2 = vec3(0.0, 0.0, 0.0);

#line 41
    aggregate_type_2_0 var_1_0 = aggregate_type_2_x24init_0(_S2, _S2, vec2(0.0, 0.0), _S2);

#line 40
    vertex_output_0 output_0;

    output_0.position_output_0 = (((vec4(input_var_0_inPosition_0, 1.0)) * ((((mat4x4(global_parameter_block_global_ubo_0.global_var_0_0.data_0[0][0], global_parameter_block_global_ubo_0.global_var_0_0.data_0[1][0], global_parameter_block_global_ubo_0.global_var_0_0.data_0[2][0], global_parameter_block_global_ubo_0.global_var_0_0.data_0[3][0], global_parameter_block_global_ubo_0.global_var_0_0.data_0[0][1], global_parameter_block_global_ubo_0.global_var_0_0.data_0[1][1], global_parameter_block_global_ubo_0.global_var_0_0.data_0[2][1], global_parameter_block_global_ubo_0.global_var_0_0.data_0[3][1], global_parameter_block_global_ubo_0.global_var_0_0.data_0[0][2], global_parameter_block_global_ubo_0.global_var_0_0.data_0[1][2], global_parameter_block_global_ubo_0.global_var_0_0.data_0[2][2], global_parameter_block_global_ubo_0.global_var_0_0.data_0[3][2], global_parameter_block_global_ubo_0.global_var_0_0.data_0[0][3], global_parameter_block_global_ubo_0.global_var_0_0.data_0[1][3], global_parameter_block_global_ubo_0.global_var_0_0.data_0[2][3], global_parameter_block_global_ubo_0.global_var_0_0.data_0[3][3])) * ((((mat4x4(global_parameter_block_global_ubo_0.global_var_2_0.data_0[0][0], global_parameter_block_global_ubo_0.global_var_2_0.data_0[1][0], global_parameter_block_global_ubo_0.global_var_2_0.data_0[2][0], global_parameter_block_global_ubo_0.global_var_2_0.data_0[3][0], global_parameter_block_global_ubo_0.global_var_2_0.data_0[0][1], global_parameter_block_global_ubo_0.global_var_2_0.data_0[1][1], global_parameter_block_global_ubo_0.global_var_2_0.data_0[2][1], global_parameter_block_global_ubo_0.global_var_2_0.data_0[3][1], global_parameter_block_global_ubo_0.global_var_2_0.data_0[0][2], global_parameter_block_global_ubo_0.global_var_2_0.data_0[1][2], global_parameter_block_global_ubo_0.global_var_2_0.data_0[2][2], global_parameter_block_global_ubo_0.global_var_2_0.data_0[3][2], global_parameter_block_global_ubo_0.global_var_2_0.data_0[0][3], global_parameter_block_global_ubo_0.global_var_2_0.data_0[1][3], global_parameter_block_global_ubo_0.global_var_2_0.data_0[2][3], global_parameter_block_global_ubo_0.global_var_2_0.data_0[3][3])) * (mat4x4(global_parameter_block_global_ubo_0.global_var_3_0.data_0[0][0], global_parameter_block_global_ubo_0.global_var_3_0.data_0[1][0], global_parameter_block_global_ubo_0.global_var_3_0.data_0[2][0], global_parameter_block_global_ubo_0.global_var_3_0.data_0[3][0], global_parameter_block_global_ubo_0.global_var_3_0.data_0[0][1], global_parameter_block_global_ubo_0.global_var_3_0.data_0[1][1], global_parameter_block_global_ubo_0.global_var_3_0.data_0[2][1], global_parameter_block_global_ubo_0.global_var_3_0.data_0[3][1], global_parameter_block_global_ubo_0.global_var_3_0.data_0[0][2], global_parameter_block_global_ubo_0.global_var_3_0.data_0[1][2], global_parameter_block_global_ubo_0.global_var_3_0.data_0[2][2], global_parameter_block_global_ubo_0.global_var_3_0.data_0[3][2], global_parameter_block_global_ubo_0.global_var_3_0.data_0[0][3], global_parameter_block_global_ubo_0.global_var_3_0.data_0[1][3], global_parameter_block_global_ubo_0.global_var_3_0.data_0[2][3], global_parameter_block_global_ubo_0.global_var_3_0.data_0[3][3]))))))))));

#line 47
    output_0.var_6_0 = var_1_0;
    vertex_output_0 _S3 = output_0;

#line 48
    gl_Position = output_0.position_output_0;

#line 48
    entryPointParam_main_var_6_fragPos_0 = _S3.var_6_0.fragPos_0;

#line 48
    entryPointParam_main_var_6_fragNormal_0 = _S3.var_6_0.fragNormal_0;

#line 48
    entryPointParam_main_var_6_fragTexCoord_0 = _S3.var_6_0.fragTexCoord_0;

#line 48
    entryPointParam_main_var_6_fragColor_0 = _S3.var_6_0.fragColor_0;

#line 48
    return;
}

)"},{"GLSL_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_var_2",{0,0,0,"","global_var_2","matrix",0,64,112,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_ubo",{0,0,0,"","global_ubo","ConstantBuffer",0,284,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"global_var_0",{0,0,0,"","global_var_0","matrix",0,64,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_1",{0,0,0,"","global_var_1","matrix",0,44,64,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_3",{0,0,0,"","global_var_3","matrix",0,64,176,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inColor",{0,0,3,"LOCATION","inColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_4",{0,0,0,"","global_var_4","vector",0,12,240,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_5",{0,0,0,"","global_var_5","vector",0,12,256,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_6",{0,0,0,"","global_var_6","vector",0,12,272,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_7.texture",{0,0,0,"","texture","_Texture",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(5)}},{"global_var_7.sampler",{0,0,0,"","sampler","SamplerState",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(6)}},{"input.var_0.inPosition",{0,0,0,"LOCATION","inPosition","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inNormal",{0,0,1,"LOCATION","inNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inTexCoord",{0,0,2,"LOCATION","inTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"Slang_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", R"(struct aggregate_type_0 {
	Texture2D<float4> texture;
	SamplerState sampler;
}
struct aggregate_type_1 {
	float3 inPosition;
	float3 inNormal;
	float2 inTexCoord;
	float3 inColor;
}
struct aggregate_type_2 {
	float3 fragPos;
	float3 fragNormal;
	float2 fragTexCoord;
	float3 fragColor;
}
struct global_ubo_struct {
	float4x4 global_var_0;
	float3x3 global_var_1;
	float4x4 global_var_2;
	float4x4 global_var_3;
	float3 global_var_4;
	float3 global_var_5;
	float3 global_var_6;
	aggregate_type_0 global_var_7;
}
struct parameter_block_struct {
	ConstantBuffer<global_ubo_struct> global_ubo;
}
ParameterBlock<parameter_block_struct> global_parameter_block;
struct vertex_input {
	aggregate_type_1 var_0 : LOCATION0;
}
struct vertex_output {
	float4 position_output : SV_POSITION;
	aggregate_type_2 var_6 : LOCATION0;
}
[shader("vertex")]
vertex_output main(vertex_input input) {
	vertex_output output;
	aggregate_type_2 var_1 = {};
	output.position_output = mul(mul(mul(global_parameter_block.global_ubo.global_var_3,global_parameter_block.global_ubo.global_var_2),global_parameter_block.global_ubo.global_var_0),float4(input.var_0.inPosition,1.000000));
	float3 var_2 = mul(global_parameter_block.global_ubo.global_var_0,float4(input.var_0.inPosition,1.000000)).xyz;
	float3 var_3 = normalize(mul(global_parameter_block.global_ubo.global_var_1,input.var_0.inNormal));
	float3 var_4 = input.var_0.inColor;
	float2 var_5 = input.var_0.inTexCoord;
	output.var_6 = var_1;
	return output;
}
)"},{"Slang_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_var_2",{0,0,0,"","global_var_2","matrix",0,64,112,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_ubo",{0,0,0,"","global_ubo","ConstantBuffer",0,288,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"global_var_0",{0,0,0,"","global_var_0","matrix",0,64,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_1",{0,0,0,"","global_var_1","matrix",0,48,64,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_3",{0,0,0,"","global_var_3","matrix",0,64,176,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inColor",{0,0,3,"LOCATION","inColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_4",{0,0,0,"","global_var_4","vector",0,12,240,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_5",{0,0,0,"","global_var_5","vector",0,12,256,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_6",{0,0,0,"","global_var_6","vector",0,12,272,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_7.texture",{0,0,0,"","texture","_Texture",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(5)}},{"global_var_7.sampler",{0,0,0,"","sampler","SamplerState",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(6)}},{"input.var_0.inPosition",{0,0,0,"LOCATION","inPosition","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inNormal",{0,0,1,"LOCATION","inNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inTexCoord",{0,0,2,"LOCATION","inTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"DXIL_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", std::vector<uint32_t>{1128421444,3978953624,3474843530,4237568456,3064449704,1,7000,7,60,76,232,432,844,2972,3000,810108499,8,0,0,826757961,148,4,8,0,136,0,0,3,0,1799,0,0,136,1,0,3,1,7,0,0,136,2,0,3,2,3,0,0,136,3,0,3,3,7,0,1094930252,1313818964,0,826757967,192,5,8,0,168,0,1,3,0,15,0,0,180,0,0,3,1,2055,0,0,180,1,0,3,2,2055,0,0,180,2,0,3,3,3075,0,0,180,3,0,3,4,2055,0,1348425299,1953067887,7237481,1094930252,1313818964,0,810963792,404,52,1,0,0,0,0,4294967295,1,67110148,5,0,0,0,73,1,24,2,0,0,0,13,0,80,1129270272,1330205761,1330380878,1230258499,1275088463,1413563215,5132105,1094930252,1313818964,1129270272,1330205761,1330380878,1230258499,1275088463,1413563215,5132105,1094930252,1313818964,1767992576,110,4,0,1,2,3,16,1,0,4390913,3,10,1,4391169,3,19,2,4325889,3,28,3,4391681,3,0,0,54788097,1027,37,0,4391169,515,46,1,4391425,515,55,2,4326145,515,64,3,4391937,515,15,15,15,0,0,0,0,0,0,0,0,0,0,0,0,0,1413567571,2120,65638,530,1279875140,262,16,2096,3737142082,3105,521,2130443,2,19,2435023111,1225050177,959582214,209977746,419956005,1653277726,38082688,1108054594,338825412,1259866168,2288136714,538218568,2777171523,1110579456,2416855268,1355031313,2357285185,2330297313,105263364,6225,8,4292906011,134217727,229114432,4294963332,537133055,4286984301,536870911,11012352,6217,3,1113621011,101207072,0,8329,71,159916594,75850784,2225349395,3810726660,345022852,2358004754,1287947275,146903056,167774528,149291110,9330880,272680489,1368408388,2283831308,2833522786,3962041057,3684639777,639984296,1148041407,1345372940,4184919921,2450585107,2966225439,173549584,1091309729,1644570957,1853490712,3263114779,1475494430,1218998850,2043204128,138569864,1359218953,576751112,773459602,1109312127,3661780978,71329424,482676801,2248822849,2894760096,73498753,3967961186,604070188,696281033,1488924226,1911709724,914442015,957100172,740662720,217654400,3855107376,1275300631,685764094,2794671848,3743814280,1476435032,2121280126,1630046825,1819568634,477214430,744514423,4281073742,1124090883,133448780,3207078795,2952870065,4242560252,37620276,597197363,1280,3228701715,2271245447,2038917174,3228697448,1353584007,248540430,1829654650,813305600,127955463,1829183603,2691763856,119567111,2014220397,3497527200,118548742,1896325238,2423064416,119567118,1913073786,1625884368,127955975,1829191798,1618021984,118520327,3859206262,2691827504,119567111,1980653677,1618609984,114324487,2047312110,2692089616,119567111,1946640506,2148131888,0,0,1674076160,1048832,0,2428502016,1073873159,0,2147483648,67325729,192,0,127812352,32776,0,1015414784,16781330,0,201326592,2108537,2,0,66248,24,412622386,2420904217,1193675148,574817478,1176019018,2694253056,211880456,170172874,62955568,3231734794,10701319,2694056457,2225736252,2301143047,674398225,2980070018,285278464,2684460040,3296724592,2053612209,1950,0,3196059648,25,6265,215,2420900634,1142096454,3966700687,604786157,1170671046,3332769446,67520101,3322293317,650587174,1130704006,2282769424,541098595,2558624795,113840160,3626512577,270036429,2216728706,208672833,3302938830,3770603714,3670205634,3838503114,3738748094,3468613318,3267682008,3303718616,643874526,28476424,818254897,282592774,531252,3255402608,3463710263,3267682008,3270295256,3194011364,3288868448,2552250674,1228194573,2577531923,80613408,540440705,2505122884,3463732577,3267682008,3270295256,3194142436,1426863712,135286053,1682047848,3597489810,3985434437,3985386573,3997970123,1712055883,457457794,2828247568,976139210,1651469415,1985965153,861893217,68366431,276879938,1431380064,2635552792,2973218355,3140466224,2586818864,34183215,135286065,2353136490,3590735169,2635400789,2973218355,3140466224,2595207472,34183215,2285923665,3234900019,2728446650,653706867,1710671382,1710695959,1093862899,135113760,450168481,1410946904,3808825371,2200973630,159604788,3246397442,710443526,3166733155,2160232129,3758997722,2130528,2198700059,2960328497,226730017,1963401411,51396912,3692091441,2964752641,1277310772,1413563215,323899209,547046020,3230063380,3692306584,917676294,3961423404,32775937,1618873375,1814020592,3406661128,3229288352,130064647,2081169283,2752015360,1842430473,3627885656,85474819,1095243838,1130365922,710438224,1040439357,406717744,65183996,167838560,2802471074,3939132864,778955275,3454873229,2018542189,3672245267,2964018208,1632776545,262208527,89153876,1633161091,3253220035,524415006,132170384,3254525699,2211595654,1350819901,264340768,2214084132,1834304013,3627887192,85474819,2705856594,3629646818,2960805040,174095367,3254268068,2961115076,1459972257,2421720440,704843141,1093665484,2205123352,2260915296,2163347010,2690326038,385271393,1094137670,1763061792,1937011566,2736915475,4134967089,1665550021,1810689326,2181262990,195082507,1426833595,356690991,918238774,850539671,1093253687,3473704272,1850519237,1842212142,557072458,196178883,2469114723,1964511899,3864850632,3402154704,3370031844,78043338,1009916232,3115497751,2981706423,2166995378,3473705299,2920230533,3989686316,780938893,1852689901,1963393162,3473704784,3970772677,2322368814,1823378478,1963421770,3395088584,4075088092,3704807072,78046952,210096310,2917950927,1286507982,3494538862,11,6265,76,478152755,1713168836,2285699348,3280222275,125846156,1929869433,3859575192,283971328,243332110,507644979,3458056642,811998369,1133001989,461603896,3359441868,59522371,2356726845,2064085108,1215891208,124809351,1979936890,544245624,298588551,244378638,1846489313,4041412400,240185358,499388467,3625722334,1640111393,2301650462,998489147,3023651792,2210151427,990086204,1981083852,1752893280,2271753991,923232370,2423293824,123760775,1980180598,2021000696,2273343367,1904674911,2557642520,2174253447,250670636,4111393006,820776640,2714264067,480371740,3827081676,1641815201,471976476,3390931396,2429945441,1137195331,960731193,3090760648,1133787331,990087224,3157246868,2197568643,990106683,3289170864,1887176481,2267576835,428310646,4161684433,551814880,115402510,4061008118,2430668480,256962319,244,8305,30,2534228054,3493739759,1275383348,3274967108,918607938,4176723548,85278786,806262033,134317204,3067895704,3854791705,1143967547,554774532,4044033232,565382725,2251587789,3854781445,952078139,255205568,1841690933,1546993819,3539530686,1241793764,4181004109,2166058437,4089338932,1292964856,2120290312,1074191217,13762608,0,1213415752,20,0,2225491384,1819294832,1943259305,249389470,1279875140,3992,65638,998,1279875140,262,16,3968,3737142082,3105,989,2130443,2,19,2435023111,1225050177,959582214,209977746,419956005,1653277726,38082688,1108054594,338825412,1259866168,2288136714,538218568,2777171523,1110579456,2416855268,1355031313,2357285185,2330297313,105263364,6225,8,4292906011,134217727,229114432,4294963332,537133055,4286984301,536870911,11012352,6217,3,1113621011,101207072,0,8329,71,159916594,75850784,2225349395,3810726660,345022852,2358004754,1287947275,146903056,167774528,149291110,9330880,272680489,1368408388,2283831308,2833522786,3962041057,3684639777,639984296,1148041407,1345372940,4184919921,2450585107,2966225439,173549584,1091309729,1644570957,1853490712,3263114779,1475494430,1218998850,2043204128,138569864,1359218953,576751112,773459602,1109312127,3661780978,71329424,482676801,2248822849,2894760096,73498753,3967961186,604070188,696281033,1488924226,1911709724,914442015,957100172,740662720,217654400,3855107376,1275300631,685764094,2794671848,3743814280,1476435032,2121280126,1630046825,1819568634,477214430,744514423,4281073742,1124090883,133448780,3207078795,2952870065,4242560252,37620276,597197363,1280,3228701715,2271245447,2038917174,3228697448,1353584007,248540430,1829654650,813305600,127955463,1829183603,2691763856,119567111,2014220397,3497527200,118548742,1896325238,2423064416,119567118,1913073786,1625884368,127955975,1829191798,1618021984,118520327,3859206262,2691827504,119567111,1980653677,1618609984,114324487,2047312110,2692089616,119567111,1946640506,648240,0,0,1674076160,1048832,0,2428502016,1073873159,0,2147483648,67325729,192,0,127812352,32776,0,1015414784,16781330,0,201326592,2108537,2,0,66248,17,345513522,2420904217,1193675148,574817478,1243127882,1889535074,480299529,171745418,412256897,579020801,186254376,2165313552,1738715588,499322,0,2147483648,6594,6265,110,2420900634,1142096454,3966700687,604786157,1170671046,3332769446,67520101,3322293317,650587174,1130704006,2282769424,541098595,2558624795,113840160,3626512481,270036429,270571650,203490506,68354608,679532898,1126736651,2617797635,1006633997,205587648,876678574,301994049,1852399981,2258540612,2660411522,1158162076,2754101556,1093828877,34186792,2282769601,1839235942,2997163547,840625198,70705893,2825225923,2260454316,159430764,141353218,1350721728,806755152,204188840,3222028718,70648544,2873460442,2920057772,1613681441,1669189891,2791048784,3735076524,3538473702,136764638,541144389,2525105566,3843431174,1124903948,2483880323,741739851,2556257603,1893272600,806755185,332115112,2198020483,1746983819,2873460442,3289131948,3976400992,34177906,3763891457,841921184,2200011536,1356565379,3021668780,3250854849,3826041222,2350946304,1346483585,826499304,3573576529,909514049,882325179,3124179634,2236629289,2932199180,3979243084,1078619597,1942167860,3009612721,312185643,4039669016,3268470364,2598685384,3637168350,424150182,3105307698,985119154,3115495865,2270413097,784715532,2301553837,2351860332,273312942,1942168021,2476962601,1266320331,2610668403,119826,6265,76,478152755,1713168836,2285699348,3280222275,125846156,1929869433,3859575192,283971328,243332110,507644979,3458056642,811998369,1133001989,461603896,3359441868,59522371,2356726845,2064085108,1215891208,124809351,1979936890,544245624,298588551,244378638,1846489313,4041412400,240185358,499388467,3625722334,1640111393,2301650462,998489147,3023651792,2210151427,990086204,1981083852,1752893280,2271753991,923232370,2423293824,123760775,1980180598,2021000696,2273343367,1904674911,2557642520,2174253447,250670636,4111393006,820776640,2714264067,480371740,3827081676,1641815201,471976476,3390931396,2429945441,1137195331,960731193,3090760648,1133787331,990087224,3157246868,2197568643,990106683,3289170864,1887176481,2267576835,428310646,4161684433,551814880,115402510,4061008118,2430668480,256962319,244,8305,30,2534228054,3493739759,1275383348,3274967108,918607938,4176723548,85278786,806262033,134317204,3067895704,3854791705,1143967547,554774532,4044033232,565382725,2251587789,3854781445,952078139,255205568,1841690933,1546993819,3539530686,1241793764,4181004109,2166058437,4089338932,1292964856,2120290312,1074191217,13762608,8289,578,742458387,16,11,268471636,1353014353,2768253814,84128337,1158697019,1367676497,9720860,3725095749,2216481858,219424930,35,525859,2439012482,1098916545,7365172,2449868832,822160409,536907874,449185288,102785648,2422354906,101195776,1668553478,872875568,536907874,457573896,104883320,1885484122,101195776,1145838349,69635097,276981952,3255576204,2352022120,134224920,118702466,2735133272,2586837257,161678368,1083846787,117842700,3495985664,2900836824,4352193,3238536844,2350957160,2294485030,545259969,544945176,2350682886,1757414438,646713474,40419852,471370801,1099040768,2154236419,1114161625,136743936,549611713,3272615564,2147598728,2771654688,433915584,3221497484,2349892200,1757547558,411840770,2181562396,1896522561,816055840,2734690458,2586853897,161679408,1885483204,101195776,3252169677,1757528089,646709314,3261645064,270961696,29460675,873996416,1812443316,161679168,546975745,2198446852,205560368,460323,3838861442,29632514,69635102,276981952,3255576204,2352022120,134224920,199442818,2735169701,1088048,809634211,2735222938,1647363081,136314992,2822294790,3263438530,2348827240,1757480998,646717634,1733133072,51486496,809566212,373411960,1174942622,67112972,2269765825,550393945,4206872,1199768592,954296349,858701955,25743248,404783106,190901308,587471251,2181040134,3283415136,275135533,2103436,3821109768,2561718286,425418817,12871624,202391553,2245031966,293735595,1090520067,1642231856,137537047,1051718,2447425796,3396886023,210743328,2153919460,101195776,3271048463,2294351430,545260033,4042603544,68753163,525859,3639632002,1682716675,3692599312,205945985,3238264848,1896333792,404811504,268451889,522683140,2210087366,16827488,504107072,3943112829,53674510,2295304251,545260033,1627143192,123757070,33654977,1008214144,2651746557,102958109,1619132424,973338864,2551215720,2088883226,1051718,31506692,2200990985,205916218,3238264848,1971915232,138052368,1051718,299942148,3941627401,2687971360,411891971,2181562400,3994174401,276039328,2103436,1405190664,107015954,411832436,2181562400,4027737025,276039200,2191268952,269239932,3770745856,1752762689,1174950787,67112972,2303320257,1401112697,269239816,3770745856,679086417,2886226819,1044466702,525859,2968543362,2177121668,102958121,1619132424,1036301552,69829044,525859,3102761090,2175024644,127271977,295641253,1090520067,1113618480,346090015,67309954,2016428288,3793707618,293737632,1090520067,2187622448,346085919,1375988482,25743248,404783106,266417212,3238674553,2147615112,893130784,1081413617,25739530,404783106,302069308,2164932721,3358442113,16827495,504107072,1040781468,3294660024,272630016,412917260,2243443721,16827488,504107072,973676701,1220576696,870586049,536903778,1326384648,3626501272,2153918530,101195776,77352783,809687071,536903778,1334773256,3625845920,1353211970,823783947,68157504,1412171651,560600194,4206872,662897680,335697452,823665002,68157504,1479296899,560598914,94393904,538479865,3246524416,2167281219,2349904906,134225944,374587778,3020570927,538479632,3246524416,19928675,1477489674,2089020447,1051718,1105248516,2211485963,205916285,3238264848,2576042464,142443448,1051718,1373683972,2208340491,338430077,591282676,2181040134,2242965600,1048705101,134619908,4032856576,3964519860,587480705,2181040134,96006240,1048698958,4178204164,51486496,809566212,658694264,2183078212,262929,3799527489,553527138,51479071,809566212,662889592,35594488,2424046923,33654991,1008214144,2786320756,2294353792,545260033,4050992152,260088595,33654977,1008214144,2685796726,4118876032,1741179585,1073807556,3156085776,1480395536,12869769,202391553,2333654174,1619613755,1073807556,3172862992,1480198944,1094893705,1647567909,136315008,2556366598,1151868805,8413744,1325795360,516267103,1647330472,136315008,2690617094,1151868421,311469408,1076959730,2198081536,1121203207,404902420,268451889,875004676,1376772822,1076959266,2198081536,2195207239,2955039247,4178133076,2103436,1136755208,174157338,411832616,2181562400,1863996353,287836721,2103436,1673626120,167866394,525865256,1182565612,67112972,222421185,2642661561,269239816,3770745856,3099168073,1174969731,67112972,223469761,2642649274,1947479048,102972994,1619132424,1569108208,72253944,525859,3035652226,2179751366,102958158,1619132424,1577498864,72253920,557386326,67309983,2016428288,1143948128,293742369,1090520067,1667397680,656473647,67309954,2016428288,4163863396,1258432288,3482360837,2147615112,3023837216,2158368737,25739539,404783106,401716540,3239280803,2147615112,3057391648,2157976065,3254092051,3295135834,272630016,282861068,2337815821,16827488,2651590720,999102684,3294661464,272630016,551362060,2337815053,759251648,2153919460,101195776,2258136847,809871391,536903778,1867449864,2820589212,2153918533,101195776,111177615,1615177758,4061565093,4206873,126026768,339956796,823665362,68157504,1446782851,584191971,4206872,1199768592,260266044,1420829394,2365131584,134225944,507756930,1745510830,538479633,3246524416,833560147,2349950986,134225944,509854082,1745486256,4229912593,205945989,3238264848,3641655776,146768848,1051718,1239466244,2209929615,2687240383,475588611,1182566028,67112972,258072769,3515052251,269239816,3770745856,1490784105,738775427,1491600192,2089196569,1051718,2178990340,2198920463,205916371,3238264848,3717171680,2955103992,1703838592,295641515,1090520067,3286792240,148220983,1051718,2850078980,1452859279,2181083171,37849294,538452096,4045275641,3521715985,2365580,2449572360,521265183,3374010865,2147631496,4179630112,301014529,2354177823,134226968,529662338,4045345248,2294357633,545260097,33103896,521270302,588980713,2181040390,132415584,2085081208,2355922644,134226968,529662338,4045357536,833756785,536907874,2118649352,3296413568,3335027271,2147631496,4179630112,300752385,455193887,591395,3833618562,1203009543,1820826748,2365580,2449572360,519168031,2988347889,9462321,1208354848,2076672126,3362604996,37849286,538452096,3944612345,568925969,151397147,2153808384,2893547492,2262072391,605588588,25298944,2984255378,510783774,33202,0,}},{"DXIL_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_var_2",{0,0,0,"","global_var_2","matrix",0,64,112,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_ubo",{0,0,0,"","global_ubo","ConstantBuffer",0,284,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"global_var_0",{0,0,0,"","global_var_0","matrix",0,64,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_1",{0,0,0,"","global_var_1","matrix",0,44,64,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_3",{0,0,0,"","global_var_3","matrix",0,64,176,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inColor",{0,0,3,"LOCATION","inColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_4",{0,0,0,"","global_var_4","vector",0,12,240,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_5",{0,0,0,"","global_var_5","vector",0,12,256,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_6",{0,0,0,"","global_var_6","vector",0,12,272,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_7.texture",{0,0,0,"","texture","_Texture",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(5)}},{"global_var_7.sampler",{0,0,0,"","sampler","SamplerState",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(6)}},{"input.var_0.inPosition",{0,0,0,"LOCATION","inPosition","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inNormal",{0,0,1,"LOCATION","inNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inTexCoord",{0,0,2,"LOCATION","inTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"DXBC_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", std::vector<uint32_t>{1128421444,3403677492,2713041802,2654069656,652700570,1,2984,5,52,680,804,964,2828,1178944594,620,1,136,1,60,4294837505,1051904,580,625218323,60,24,40,40,36,12,0,100,0,0,0,0,0,1,1,0,0,1651469415,1885301857,1835102817,1919251557,1869374047,1734306659,1633841004,1651859308,3170159,100,1,160,288,0,0,100,0,284,2,544,0,4294967295,0,4294967295,0,1651469415,1969187937,1935634274,1668641396,3170164,1651469415,1985965153,811561569,1711288415,1952542572,3438644,196611,262148,0,0,0,0,0,0,235,1651469415,1985965153,828338785,1711288415,1952542572,3373107,196611,196611,0,0,0,0,0,0,295,1651469415,1985965153,845116001,1728065631,1633841004,1635147628,1597202290,1818689584,1818321519,1918989919,811545695,1869374976,3372129,196609,196609,0,0,0,0,0,0,385,1651469415,1985965153,895447649,1728065631,1633841004,1635147628,1597398898,2880110640,220,244,0,280,304,64,340,244,112,355,244,176,370,392,240,428,392,256,443,392,272,5,4325377,458752,460,0,0,0,0,200,1919117645,1718580079,1378361460,1279795241,1394625619,1701077352,1866670194,1818849389,824210021,3223088,1313297225,116,4,8,104,0,0,3,0,1799,104,1,0,3,1,7,104,2,0,3,2,3,104,3,0,3,3,7,1094930252,1313818964,2880154368,1313297231,152,5,8,128,0,1,3,0,15,140,0,0,3,1,2055,140,1,0,3,2,2055,140,2,0,3,3,3075,140,3,0,3,4,2055,1348425299,1414091599,5132105,1094930252,1313818964,2880154368,1480935507,1856,65617,464,16779370,117440601,3182150,0,0,0,15,0,50331743,1052786,0,67108967,1057010,0,1,50331749,1056882,1,50331749,1056882,2,50331749,1056818,3,50331749,1056882,4,33554536,7,184549432,1048818,0,3179862,0,0,7,3182150,0,0,12,218103858,1048818,0,3182150,0,0,11,3178502,0,0,7,1052230,0,218103858,1048818,0,3182150,0,0,13,3181222,0,0,7,1052230,0,218103858,1048818,0,3182150,0,0,14,3182582,0,0,7,1052230,0,184549432,1048818,1,3179862,0,0,8,3182150,0,0,12,218103858,1048818,1,3182150,0,0,11,3178502,0,0,8,1052230,1,218103858,1048818,1,3182150,0,0,13,3181222,0,0,8,1052230,1,218103858,1048818,1,3182150,0,0,14,3182582,0,0,8,1052230,1,184549432,1048818,2,3179862,0,0,9,3182150,0,0,12,218103858,1048818,2,3182150,0,0,11,3178502,0,0,9,1052230,2,218103858,1048818,2,3182150,0,0,13,3181222,0,0,9,1052230,2,218103858,1048818,2,3182150,0,0,14,3182582,0,0,9,1052230,2,184549432,1048818,3,3179862,0,0,10,3182150,0,0,12,218103858,1048818,3,3182150,0,0,11,3178502,0,0,10,1052230,3,218103858,1048818,3,3182150,0,0,13,3181222,0,0,10,1052230,3,218103858,1048818,3,3182150,0,0,14,3182582,0,0,10,1052230,3,150995000,1048818,4,1052230,1,3179862,0,0,0,184549426,1048818,4,1052230,0,3178502,0,0,0,1052230,4,184549426,1048818,4,1052230,2,3181222,0,0,0,1052230,4,184549426,1048818,4,1052230,3,3182582,0,0,0,1052230,4,150995000,1048818,5,1052230,1,3179862,0,0,1,184549426,1048818,5,1052230,0,3178502,0,0,1,1052230,5,184549426,1048818,5,1052230,2,3181222,0,0,1,1052230,5,184549426,1048818,5,1052230,3,3182582,0,0,1,1052230,5,150995000,1048818,6,1052230,1,3179862,0,0,2,184549426,1048818,6,1052230,0,3178502,0,0,2,1052230,6,184549426,1048818,6,1052230,2,3181222,0,0,2,1052230,6,184549426,1048818,6,1052230,3,3182582,0,0,2,1052230,6,150995000,1048818,1,1052230,1,3179862,0,0,3,184549426,1048818,0,1052230,0,3178502,0,0,3,1052230,1,184549426,1048818,0,1052230,2,3181222,0,0,3,1052230,0,184549426,1048818,0,1052230,3,3182582,0,0,3,1052230,0,117440568,1048818,1,1052230,5,1054038,0,150994994,1048818,1,1052230,4,1052678,0,1052230,1,150994994,1048818,1,1052230,6,1055398,0,1052230,1,117440512,1057010,0,1052230,0,1052230,1,134217782,1056882,1,16386,0,0,0,0,134217782,1056882,2,16386,0,0,0,0,134217782,1056882,4,16386,0,0,0,0,134217782,1056818,3,16386,0,0,0,0,16777278,1413567571,148,41,7,0,6,36,0,0,1,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}},{"DXBC_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_var_2",{0,0,0,"","global_var_2","matrix",0,64,112,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_ubo",{0,0,0,"","global_ubo","ConstantBuffer",0,288,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"global_var_0",{0,0,0,"","global_var_0","matrix",0,64,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_1",{0,0,0,"","global_var_1","matrix",0,48,64,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_3",{0,0,0,"","global_var_3","matrix",0,64,176,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inColor",{0,0,3,"LOCATION","inColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_4",{0,0,0,"","global_var_4","vector",0,12,240,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_5",{0,0,0,"","global_var_5","vector",0,12,256,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_6",{0,0,0,"","global_var_6","vector",0,12,272,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_7.texture",{0,0,0,"","texture","_Texture",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(5)}},{"global_var_7.sampler",{0,0,0,"","sampler","SamplerState",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(6)}},{"input.var_0.inPosition",{0,0,0,"LOCATION","inPosition","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inNormal",{0,0,1,"LOCATION","inNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inTexCoord",{0,0,2,"LOCATION","inTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"SpirV_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", std::vector<uint32_t>{119734787,66816,2621440,186,0,131089,5302,131089,1,196622,0,1,851983,0,2,1852399981,0,52,170,175,178,182,185,15,66,196611,11,1,524293,15,1970302569,1635135092,774922098,1867542121,1769236851,28271,720901,48,1651469415,1885301857,1600680821,1936617315,1953390964,1920234335,1601463157,878998643,12339,393222,48,0,1651469415,1969187937,28514,524293,52,1651469415,1885301857,1600680821,1936617315,1953390964,0,786437,59,1952533855,1400400242,1634889588,1717527911,1952542572,1597274164,1298952003,1919904353,828666995,12340,327686,59,0,1635017060,0,786437,62,1952533855,1400400242,1634889588,1717527911,1952542572,1597208627,1298952003,1919904353,828666995,12340,327686,62,0,1635017060,0,524293,64,1919379297,1952540517,2037669733,811558256,1685353311,3159089,327686,64,0,1954047348,6648437,327686,64,1,1886216563,7497068,589829,58,1651469415,1969187937,1935634274,1668641396,1953718132,808726884,0,458758,58,0,1651469415,1985965153,811561569,0,458758,58,1,1651469415,1985965153,828338785,0,458758,58,2,1651469415,1985965153,845116001,0,458758,58,3,1651469415,1985965153,861893217,0,458758,58,4,1651469415,1985965153,878670433,0,458758,58,5,1651469415,1985965153,895447649,0,458758,58,6,1651469415,1985965153,912224865,0,458758,58,7,1651469415,1985965153,929002081,0,393221,66,1717990754,1632137829,1701602414,115,720901,175,1920233061,1768902777,1632662638,1601003890,1852399981,1918989870,1714304607,1348952434,29551,786437,178,1920233061,1768902777,1632662638,1601003890,1852399981,1918989870,1714304607,1315398002,1634562671,108,786437,182,1920233061,1768902777,1632662638,1601003890,1852399981,1918989870,1714304607,1416061298,1866692709,6582895,786437,185,1920233061,1768902777,1632662638,1601003890,1852399981,1918989870,1714304607,1130848626,1919904879,0,262149,2,1852399981,0,262215,15,30,0,196679,48,2,327752,48,0,35,0,262215,60,6,16,327752,59,0,35,0,262215,63,6,16,327752,62,0,35,0,327752,64,0,35,0,327752,64,1,35,8,196679,58,2,327752,58,0,35,0,327752,58,1,35,64,327752,58,2,35,112,327752,58,3,35,176,327752,58,4,35,240,327752,58,5,35,256,327752,58,6,35,272,327752,58,7,35,288,262215,66,33,0,262215,66,34,3,262215,170,11,0,262215,175,30,0,262215,178,30,1,262215,182,30,2,262215,185,30,3,131091,1,196641,3,1,196630,6,32,262167,7,6,4,262167,9,6,3,262167,10,6,2,262176,14,1,9,262165,26,32,1,262187,26,27,0,262187,26,34,2,262187,26,38,3,262187,6,44,0,393260,9,43,44,44,44,327724,10,45,44,44,262165,49,32,0,262167,50,49,2,196638,48,50,262176,51,9,48,262176,53,9,50,262187,26,61,4,262172,60,7,61,196638,59,60,262172,63,9,38,196638,62,63,262174,64,50,50,655390,58,59,62,59,59,9,9,9,64,196637,57,58,262176,65,2,57,262176,67,2,58,262176,69,2,59,262168,97,7,4,262187,6,162,1065353216,262176,169,3,7,262176,174,3,9,262176,181,3,10,262203,14,15,1,262203,51,52,9,262203,65,66,2,262203,169,170,3,262203,174,175,3,262203,174,178,3,262203,181,182,3,262203,174,185,3,327734,1,2,0,3,131320,4,262205,9,13,15,327746,53,54,52,27,262205,50,55,54,327761,49,56,55,0,327745,67,68,66,56,327746,69,70,68,38,262205,59,71,70,327761,60,72,71,0,327761,7,73,72,0,327761,6,74,73,0,327761,6,75,73,1,327761,6,76,73,2,327761,6,77,73,3,327761,7,78,72,1,327761,6,79,78,0,327761,6,80,78,1,327761,6,81,78,2,327761,6,82,78,3,327761,7,83,72,2,327761,6,84,83,0,327761,6,85,83,1,327761,6,86,83,2,327761,6,87,83,3,327761,7,88,72,3,327761,6,89,88,0,327761,6,90,88,1,327761,6,91,88,2,327761,6,92,88,3,458832,7,93,74,79,84,89,458832,7,94,75,80,85,90,458832,7,95,76,81,86,91,458832,7,96,77,82,87,92,458832,97,98,93,94,95,96,327761,49,99,55,0,327745,67,100,66,99,327746,69,101,100,34,262205,59,102,101,327761,60,103,102,0,327761,7,104,103,0,327761,6,105,104,0,327761,6,106,104,1,327761,6,107,104,2,327761,6,108,104,3,327761,7,109,103,1,327761,6,110,109,0,327761,6,111,109,1,327761,6,112,109,2,327761,6,113,109,3,327761,7,114,103,2,327761,6,115,114,0,327761,6,116,114,1,327761,6,117,114,2,327761,6,118,114,3,327761,7,119,103,3,327761,6,120,119,0,327761,6,121,119,1,327761,6,122,119,2,327761,6,123,119,3,458832,7,124,105,110,115,120,458832,7,125,106,111,116,121,458832,7,126,107,112,117,122,458832,7,127,108,113,118,123,458832,97,128,124,125,126,127,327826,97,129,128,98,327761,49,130,55,0,327745,67,131,66,130,327746,69,132,131,27,262205,59,133,132,327761,60,134,133,0,327761,7,135,134,0,327761,6,136,135,0,327761,6,137,135,1,327761,6,138,135,2,327761,6,139,135,3,327761,7,140,134,1,327761,6,141,140,0,327761,6,142,140,1,327761,6,143,140,2,327761,6,144,140,3,327761,7,145,134,2,327761,6,146,145,0,327761,6,147,145,1,327761,6,148,145,2,327761,6,149,145,3,327761,7,150,134,3,327761,6,151,150,0,327761,6,152,150,1,327761,6,153,150,2,327761,6,154,150,3,458832,7,155,136,141,146,151,458832,7,156,137,142,147,152,458832,7,157,138,143,148,153,458832,7,158,139,144,149,154,458832,97,159,155,156,157,158,327826,97,160,159,129,327760,7,161,13,162,327824,7,163,161,160,196670,170,163,196670,175,43,196670,178,43,196670,182,45,196670,185,43,65789,65592,}},{"SpirV_Reflection_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_ubo",{0,0,0,"","global_ubo","vector",0,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inNormal",{0,0,1,"LOCATION","inNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inPosition",{0,0,0,"LOCATION","inPosition","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inTexCoord",{0,0,2,"LOCATION","inTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inColor",{0,0,3,"LOCATION","inColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"GLSL_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", R"(#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 12144 0
struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    vec4  data_0[4];
};


#line 5 1
struct _MatrixStorage_float3x3_ColMajorstd140_0
{
    vec3  data_1[3];
};


#line 1
struct aggregate_type_0_std140_0
{
    uvec2 texture_0;
    uvec2 sampler_0;
};


#line 1
struct global_ubo_struct_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_0_0;
    _MatrixStorage_float3x3_ColMajorstd140_0 global_var_1_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_2_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_3_0;
    vec3 global_var_4_0;
    vec3 global_var_5_0;
    vec3 global_var_6_0;
    aggregate_type_0_std140_0 global_var_7_0;
};


#line 22879 0
struct global_push_constant_struct_std430_0
{
    uvec2 global_ubo_0;
};


#line 30 1
layout(push_constant)
layout(std430) uniform block_global_push_constant_struct_std430_0
{
    uvec2 global_ubo_0;
}global_push_constant;

#line 30
layout(binding = 0, set = 3)
layout(std140) uniform block_global_ubo_struct_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_0_0;
    _MatrixStorage_float3x3_ColMajorstd140_0 global_var_1_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_2_0;
    _MatrixStorage_float4x4_ColMajorstd140_0 global_var_3_0;
    vec3 global_var_4_0;
    vec3 global_var_5_0;
    vec3 global_var_6_0;
    aggregate_type_0_std140_0 global_var_7_0;
}bufferHandles[];

#line 11
struct aggregate_type_2_0
{
    vec3 fragPos_0;
    vec3 fragNormal_0;
    vec2 fragTexCoord_0;
    vec3 fragColor_0;
};


#line 11
aggregate_type_2_0 aggregate_type_2_x24init_0(vec3 fragPos_1, vec3 fragNormal_1, vec2 fragTexCoord_1, vec3 fragColor_1)
{

#line 11
    aggregate_type_2_0 _S1;
    _S1.fragPos_0 = fragPos_1;
    _S1.fragNormal_0 = fragNormal_1;
    _S1.fragTexCoord_0 = fragTexCoord_1;
    _S1.fragColor_0 = fragColor_1;

#line 11
    return _S1;
}


#line 12144 0
layout(location = 0)
out vec3 entryPointParam_main_var_6_fragPos_0;


#line 12144
layout(location = 1)
out vec3 entryPointParam_main_var_6_fragNormal_0;


#line 12144
layout(location = 2)
out vec2 entryPointParam_main_var_6_fragTexCoord_0;


#line 12144
layout(location = 3)
out vec3 entryPointParam_main_var_6_fragColor_0;


#line 12144
layout(location = 0)
in vec3 input_var_0_inPosition_0;


#line 79 1
struct vertex_output_0
{
    vec4 position_output_0;
    aggregate_type_2_0 var_6_0;
};


#line 84
void main()
{

#line 12
    const vec3 _S2 = vec3(0.0, 0.0, 0.0);

#line 86
    aggregate_type_2_0 var_1_0 = aggregate_type_2_x24init_0(_S2, _S2, vec2(0.0, 0.0), _S2);

#line 85
    vertex_output_0 output_0;

    output_0.position_output_0 = (((vec4(input_var_0_inPosition_0, 1.0)) * ((((mat4x4(bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[0][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[1][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[2][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[3][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[0][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[1][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[2][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[3][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[0][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[1][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[2][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[3][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[0][3], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[1][3], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[2][3], bufferHandles[global_push_constant.global_ubo_0.x].global_var_0_0.data_0[3][3])) * ((((mat4x4(bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[0][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[1][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[2][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[3][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[0][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[1][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[2][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[3][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[0][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[1][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[2][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[3][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[0][3], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[1][3], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[2][3], bufferHandles[global_push_constant.global_ubo_0.x].global_var_2_0.data_0[3][3])) * (mat4x4(bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[0][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[1][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[2][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[3][0], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[0][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[1][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[2][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[3][1], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[0][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[1][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[2][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[3][2], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[0][3], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[1][3], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[2][3], bufferHandles[global_push_constant.global_ubo_0.x].global_var_3_0.data_0[3][3]))))))))));

#line 92
    output_0.var_6_0 = var_1_0;
    vertex_output_0 _S3 = output_0;

#line 93
    gl_Position = output_0.position_output_0;

#line 93
    entryPointParam_main_var_6_fragPos_0 = _S3.var_6_0.fragPos_0;

#line 93
    entryPointParam_main_var_6_fragNormal_0 = _S3.var_6_0.fragNormal_0;

#line 93
    entryPointParam_main_var_6_fragTexCoord_0 = _S3.var_6_0.fragTexCoord_0;

#line 93
    entryPointParam_main_var_6_fragColor_0 = _S3.var_6_0.fragColor_0;

#line 93
    return;
}

)"},{"GLSL_Reflection_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_ubo",{0,0,0,"","global_ubo","vector",0,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inNormal",{0,0,1,"LOCATION","inNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inPosition",{0,0,0,"LOCATION","inPosition","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inTexCoord",{0,0,2,"LOCATION","inTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inColor",{0,0,3,"LOCATION","inColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"Slang_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", R"(struct aggregate_type_0 {
	Texture2D<float4>.Handle texture;
	SamplerState.Handle sampler;
}
struct aggregate_type_1 {
	float3 inPosition;
	float3 inNormal;
	float2 inTexCoord;
	float3 inColor;
}
struct aggregate_type_2 {
	float3 fragPos;
	float3 fragNormal;
	float2 fragTexCoord;
	float3 fragColor;
}
struct global_ubo_struct {
	float4x4 global_var_0;
	float3x3 global_var_1;
	float4x4 global_var_2;
	float4x4 global_var_3;
	float3 global_var_4;
	float3 global_var_5;
	float3 global_var_6;
	aggregate_type_0 global_var_7;
}
struct global_push_constant_struct {
	ConstantBuffer<global_ubo_struct>.Handle global_ubo;
}
[[vk::push_constant]] ConstantBuffer<global_push_constant_struct> global_push_constant;
struct parameter_block_struct {
}
ParameterBlock<parameter_block_struct> global_parameter_block;
[vk::binding(0, 1)]
__DynamicResource<__DynamicResourceKind.Sampler> samplerHandles[];

[vk::binding(0, 2)]
__DynamicResource<__DynamicResourceKind.General> textureHandles[];

[vk::binding(0, 3)]
__DynamicResource<__DynamicResourceKind.General> bufferHandles[];

[vk::binding(0, 4)]
__DynamicResource<__DynamicResourceKind.General> combinedTextureSamplerHandles[];

[vk::binding(0, 5)]
__DynamicResource<__DynamicResourceKind.General> accelerationStructureHandles[];

[vk::binding(0, 6)]
__DynamicResource<__DynamicResourceKind.General> texelBufferHandles[];

export T getDescriptorFromHandle<T>(DescriptorHandle<T> handle) where T : IOpaqueDescriptor
{
	__target_switch
	{
		case spirv:
		case glsl:
		if (T.kind == DescriptorKind.Sampler)
			return samplerHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else if (T.kind == DescriptorKind.Texture)
			return textureHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else if (T.kind == DescriptorKind.Buffer)
			return bufferHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else if (T.kind == DescriptorKind.CombinedTextureSampler)
			return combinedTextureSamplerHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else if (T.kind == DescriptorKind.AccelerationStructure)
			return accelerationStructureHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else if (T.kind == DescriptorKind.TexelBuffer)
			return texelBufferHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else
			return defaultGetDescriptorFromHandle(handle);
		default:
		return defaultGetDescriptorFromHandle(handle);
	}
}
struct vertex_input {
	aggregate_type_1 var_0 : LOCATION0;
}
struct vertex_output {
	float4 position_output : SV_POSITION;
	aggregate_type_2 var_6 : LOCATION0;
}
[shader("vertex")]
vertex_output main(vertex_input input) {
	vertex_output output;
	aggregate_type_2 var_1 = {};
	output.position_output = mul(mul(mul((*global_push_constant.global_ubo).global_var_3,(*global_push_constant.global_ubo).global_var_2),(*global_push_constant.global_ubo).global_var_0),float4(input.var_0.inPosition,1.000000));
	float3 var_2 = mul((*global_push_constant.global_ubo).global_var_0,float4(input.var_0.inPosition,1.000000)).xyz;
	float3 var_3 = normalize(mul((*global_push_constant.global_ubo).global_var_1,input.var_0.inNormal));
	float3 var_4 = input.var_0.inColor;
	float2 var_5 = input.var_0.inTexCoord;
	output.var_6 = var_1;
	return output;
}
)"},{"Slang_Reflection_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_ubo",{0,0,0,"","global_ubo","vector",0,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inNormal",{0,0,1,"LOCATION","inNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inPosition",{0,0,0,"LOCATION","inPosition","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inTexCoord",{0,0,2,"LOCATION","inTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inColor",{0,0,3,"LOCATION","inColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"DXIL_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", std::vector<uint32_t>{1128421444,1626416619,1754851866,1413775730,1786628540,1,6908,7,60,76,232,432,844,2796,2824,810108499,8,33554432,0,826757961,148,4,8,0,136,0,0,3,0,1799,0,0,136,1,0,3,1,7,0,0,136,2,0,3,2,3,0,0,136,3,0,3,3,7,0,1094930252,1313818964,0,826757967,192,5,8,0,168,0,1,3,0,15,0,0,180,0,0,3,1,2055,0,0,180,1,0,3,2,2055,0,0,180,2,0,3,3,3075,0,0,180,3,0,3,4,2055,0,1348425299,1953067887,7237481,1094930252,1313818964,0,810963792,404,52,1,0,0,0,0,4294967295,1,67110148,5,0,0,0,73,1,24,2,0,0,0,13,0,80,1129270272,1330205761,1330380878,1230258499,1275088463,1413563215,5132105,1094930252,1313818964,1129270272,1330205761,1330380878,1230258499,1275088463,1413563215,5132105,1094930252,1313818964,1767992576,110,4,0,1,2,3,16,1,0,4390913,3,10,1,4391169,3,19,2,4325889,3,28,3,4391681,3,0,0,54788097,1027,37,0,4391169,515,46,1,4391425,515,55,2,4326145,515,64,3,4391937,515,15,15,15,0,0,0,0,0,0,0,0,0,0,0,0,0,1413567571,1944,65638,486,1279875140,262,16,1920,3737142082,3105,477,2130443,2,19,2435023111,1225050177,959582214,209977746,419956005,1653277726,38082688,1108054594,338825412,1259866168,2288136714,538218568,2777171523,1110579456,2416855268,1355031313,2357285185,2330297313,105263364,6225,8,4292906011,134217727,229114432,4294963332,537133055,4286984301,536870911,11012352,6217,3,1113621011,101207072,0,8329,65,159916594,75850784,2225349395,3810726660,345022852,2358004754,1287947275,146903056,167774528,149291110,9330880,272680489,1368408388,2283831308,2323914850,2680915579,4237330352,2240178192,1194492224,560108560,3869345288,670228904,2669945324,627222747,1840824102,818089028,371319316,2258870402,2276499403,2741927056,2090374538,1091679972,139527184,2584238217,3272618314,3681898544,4128504176,314544656,38093714,1137591953,1212289860,1116217416,292733996,4184920182,2450585107,2261914303,136446772,2974427266,3502316048,2351960118,105320480,3260093650,327883748,1358940928,1309405650,859852067,1360319229,684979394,372412144,4288020519,400794785,2157594268,1203847782,1930446432,40964,3228701715,2271245447,2038917174,3228697448,1353584007,248540430,1829654650,813305600,127955463,1829183603,2691763856,119567111,2014220397,3497527200,118548742,1896325238,2423064416,119567118,1913073786,1625884368,127955975,1829191798,1618021984,118520327,3859206262,2691827504,119567111,1980653677,1618609984,114324487,2047312110,2692089616,119567111,1946640506,2148131888,0,0,1674076160,1048832,0,2428502016,1073873159,0,2147483648,67325729,64,0,136200960,98312,0,1015414784,50335763,0,201326592,2108025,2,0,1080095256,1024,0,3655610368,524416,0,186646528,4,20,412622386,2420904217,1193675148,574817478,1176019018,211945984,1242866122,2747926691,2494171664,2301143047,674398225,2980070018,285278464,27002897,2147905696,1738715588,499322,0,2147483648,6590,6265,163,2420900634,1142096454,3966700687,604786157,1170671046,3332769446,67520101,3322293317,650587174,1130704006,2282769424,675316324,2558624795,113971232,3626512577,270036429,2216728706,2364163905,2351713773,1856900077,3966496014,781086157,2188283340,270217280,1661753667,1091403872,33203,2082510855,3056834847,3056644407,3081845423,34183215,135285969,1010860647,2583823379,114364448,1663338628,2204838562,68377792,272031792,454632132,1356547588,1838551064,3623945235,202927104,2081479631,60917275,411175299,822221688,2518077728,3917760230,713566313,2187971049,2215860304,403757794,3674375187,2261993504,855845061,942932792,2349670787,226709555,425832641,3456942282,1624116608,216105800,897650048,229779521,1711691147,1886258800,1478214406,141295719,1418399140,108029446,1124495473,6768666,2698387564,239132685,2103004344,2781723969,3116213713,1829376077,457719938,906938628,2552038956,1101011521,1618283549,1813889436,808617560,2202022787,1624381495,3627352888,1617254576,113272582,2175021167,2959738485,228888641,1711690891,4033742448,1209778950,319271015,2967101956,2551792116,3250854337,3496545307,111175431,3359377947,65027331,3425753151,318775233,920138116,1814578696,3291688,1746159493,2243801496,273534417,2588248584,3705478363,1755873540,2107483596,2563871153,2600155979,2424363939,3264751938,2491588649,2236642826,3450785165,3433860389,273313421,1942167892,2610113457,2608036683,3360493586,3270270192,3838504152,491127974,966212658,850538676,2989991609,19510834,3473704530,4000099909,2892910253,3762974316,1942168148,730057633,2071163403,1268976547,2610656123,1348496418,3318680711,787262766,780823689,1248636556,3363112480,3704249584,2700272872,3906786014,2718213862,3473704400,3467504741,1850519181,177228398,0,6265,76,478152755,1713168836,2285699348,3280222275,125846156,1929869433,3859575192,283971328,243332110,507644979,3458056642,811998369,1133001989,461603896,3359441868,59522371,2356726845,2064085108,1215891208,124809351,1979936890,544245624,298588551,244378638,1846489313,4041412400,240185358,499388467,3625722334,1640111393,2301650462,998489147,3023651792,2210151427,990086204,1981083852,1752893280,2271753991,923232370,2423293824,123760775,1980180598,2021000696,2273343367,1904674911,2557642520,2174253447,250670636,4111393006,820776640,2714264067,480371740,3827081676,1641815201,471976476,3390931396,2429945441,1137195331,960731193,3090760648,1133787331,990087224,3157246868,2197568643,990106683,3289170864,1887176481,2267576835,428310646,4161684433,551814880,115402510,4061008118,2430668480,256962319,244,8305,41,2534228086,3493739759,1275383348,3274967108,918624322,4176723548,85278786,806262033,134317204,3067895704,3854789653,70225723,1360023892,1229127913,4185948184,1109093832,4089338935,4473592,3439464723,3831435440,215096112,2161664840,4089338934,4473592,3439464723,3831435440,1006706992,2534223894,3808312559,1020822272,3071796436,1893007981,1273220857,672207763,3839114549,107853591,3472453842,876892131,4186193953,1797573,55050433,0,1213415752,20,0,1274963180,859455462,3284021716,3655297120,1279875140,4076,65638,1019,1279875140,262,16,4052,3737142082,3105,1010,2130443,2,19,2435023111,1225050177,959582214,209977746,419956005,1653277726,38082688,1108054594,338825412,1259866168,2288136714,538218568,2777171523,1110579456,2416855268,1355031313,2357285185,2330297313,105263364,6225,8,4292906011,134217727,229114432,4294963332,537133055,4286984301,536870911,11012352,6217,3,1113621011,101207072,0,8329,65,159916594,75850784,2225349395,3810726660,345022852,2358004754,1287947275,146903056,167774528,149291110,9330880,272680489,1368408388,2283831308,2323914850,2680915579,4237330352,2240178192,1194492224,560108560,3869345288,670228904,2669945324,627222747,1840824102,818089028,371319316,2258870402,2276499403,2741927056,2090374538,1091679972,139527184,2584238217,3272618314,3681898544,4128504176,314544656,38093714,1137591953,1212289860,1116217416,292733996,4184920182,2450585107,2261914303,136446772,2974427266,3502316048,2351960118,105320480,3260093650,327883748,1358940928,1309405650,859852067,1360319229,684979394,372412144,4288020519,400794785,2157594268,1203847782,1930446432,40964,3228701715,2271245447,2038917174,3228697448,1353584007,248540430,1829654650,813305600,127955463,1829183603,2691763856,119567111,2014220397,3497527200,118548742,1896325238,2423064416,119567118,1913073786,1625884368,127955975,1829191798,1618021984,118520327,3859206262,2691827504,119567111,1980653677,1618609984,114324487,2047312110,2692089616,119567111,1946640506,648240,0,0,1674076160,1048832,0,2428502016,1073873159,0,2147483648,67325729,64,0,136200960,98312,0,1015414784,50335763,0,201326592,2108025,2,0,1080095256,1024,0,3655610368,524416,0,186646528,4,17,345513522,2420904217,1193675148,574817478,1243127882,547357794,2317131978,2164931744,18387594,673350440,269163012,2165379088,1738715588,499322,0,2147483648,6594,6265,110,2420900634,1142096454,3966700687,604786157,1170671046,3332769446,67520101,3322293317,650587174,1130704006,2282769424,675316324,2558624795,113971232,3626512481,270036429,270571650,203490508,68354612,679532962,1126736651,2617797635,1006633997,205587648,876678578,301994049,1852399981,2258540612,2660411522,3305645724,2821210420,1093828877,34187816,2282769633,1839235943,2997163547,840625198,70705893,2825225923,2260454316,159430764,141361410,1350721728,806755160,204188840,3222028718,70648544,2873460442,2920057772,1613681441,1669189891,2791048784,3735076524,3538473702,136764638,541144517,2525105570,3843431174,1124903948,2483880323,741739851,2556257603,1893272600,806755185,332115112,2198020483,1746983819,2873460442,3289131948,3976400992,34177906,3763891489,841921184,2200011536,1356565379,3021668780,3250854849,3826041222,2350946304,1346483713,826499304,3573576529,909514049,882325179,3124179634,2236629289,2932199180,3979243084,1078619597,1942167860,3009612721,312185643,4039669016,3268470364,2598685384,3637168350,424150182,3105307698,985119154,3115495865,2270413097,784715532,2301553837,2351860332,273312942,1942168021,2476962601,1266320331,2610668403,119826,6265,76,478152755,1713168836,2285699348,3280222275,125846156,1929869433,3859575192,283971328,243332110,507644979,3458056642,811998369,1133001989,461603896,3359441868,59522371,2356726845,2064085108,1215891208,124809351,1979936890,544245624,298588551,244378638,1846489313,4041412400,240185358,499388467,3625722334,1640111393,2301650462,998489147,3023651792,2210151427,990086204,1981083852,1752893280,2271753991,923232370,2423293824,123760775,1980180598,2021000696,2273343367,1904674911,2557642520,2174253447,250670636,4111393006,820776640,2714264067,480371740,3827081676,1641815201,471976476,3390931396,2429945441,1137195331,960731193,3090760648,1133787331,990087224,3157246868,2197568643,990106683,3289170864,1887176481,2267576835,428310646,4161684433,551814880,115402510,4061008118,2430668480,256962319,244,8305,41,2534228086,3493739759,1275383348,3274967108,918624322,4176723548,85278786,806262033,134317204,3067895704,3854789653,70225723,1360023892,1229127913,4185948184,1109093832,4089338935,4473592,3439464723,3831435440,215096112,2161664840,4089338934,4473592,3439464723,3831435440,1006706992,2534223894,3808312559,1020822272,3071796436,1893007981,1273220857,672207763,3839114549,107853591,3472453842,876892131,4186193953,1797573,55050433,8289,587,742458387,16,13,268471492,1353014353,53585270,620998736,62148667,374361605,1159483781,3490268481,3611296304,3436691996,1080327201,3271826650,5680,525859,2578473090,3246920449,7365172,403048480,822163482,536907874,465962504,104883320,2422354972,101195776,1702632454,877135440,536907874,474351112,2551808384,411903745,2181562396,352773313,161678961,2153918465,101195776,2168986765,471371174,25298944,2689926902,1841292,872514056,2367707399,1757414438,646713474,40419852,471370801,25298944,1637746564,3221497485,2349892200,1757547558,411840770,2181562396,1661457409,1757513904,646709314,3261645064,270961696,29460675,1075323008,3490087074,1088055,809634211,2735222938,1647363081,136314992,3022639110,2735195457,2586837257,161678368,1083846787,117842700,6324736,1746698929,278540451,1107927808,137402928,818153891,536899682,743441928,178766300,805374371,2734956698,2586870537,102960192,1619132423,520280320,2350678416,1757414438,646713474,40419852,108320817,1076959730,2198081536,1632114506,3294668300,272630016,677227020,1175089029,67112972,98738369,3232159827,3482334628,2147615112,3193182240,543623808,134619909,1348502016,2351579901,411833473,2181562400,2853045569,1166100976,295641199,1090520067,2177148976,169924117,1051718,446742788,2201507463,1076959272,2198081536,1633426570,2696978956,2295304249,545260033,15094808,83914507,525859,2639290498,28060739,538479636,1099040768,1890717317,1667256326,435292000,268451889,491422468,3322823016,1076959264,2198081536,3244957034,404801037,268451889,495616772,3322757484,3492884512,411891971,2181562400,3087983937,274859872,2103436,4114711048,119585038,411832418,2181562400,3121546561,274859744,2180520024,269239932,2697004032,3495823250,1174941827,67112972,2275057857,813936734,269239816,2697004032,2422146978,201863299,1044464652,525859,3645923458,33044483,102958104,1619132424,943971664,68682216,525859,3780141186,30947459,120980504,295641253,1090520067,2180294704,241254940,67309954,2821734656,4196180470,293736032,1090520067,3254298672,241234460,1417908994,25743248,404783106,243334740,3238469797,2147615112,4283701280,547425936,25739527,404783106,245440596,2164727933,3358278085,16827495,705433664,1418158225,3294659464,272630016,1754376716,2206749319,16827488,705433664,1351053458,3770712968,870585888,536903778,1234503176,3224060864,2153918529,101195776,1136937429,809615402,536903778,1242891784,3223929800,1084448833,823783947,68157504,3894758019,558241281,4206872,1786971152,360835621,823664966,68157504,3961883267,558241025,96489776,538479865,1099040768,1089999589,2349900299,134225944,318062978,2718630137,538479632,1099040768,3237614341,1477485066,2088955944,1051718,2460008708,2243984905,205916240,3238264848,2122947232,139494816,1051718,2728444164,2239790857,336332880,591282624,2181040134,81350752,671281224,134619908,1348502016,3762832605,587474946,2181040134,2229358688,671273032,117842692,6324736,2756199661,646728899,2187902980,203852816,822241474,1841292,67207688,249636374,278540603,1107927808,137402928,818153891,536899682,1498416648,3829187756,4352196,3238536844,2350957160,2294485030,545259969,23740440,369162259,805374371,2734956698,2586870537,3308653632,205945986,3238264848,2617995936,405374320,268451889,770343684,2786419314,16827488,705433664,1409929400,1788019352,102972993,1619132424,1325779280,2350154440,134225944,389366146,1376448829,8413744,352716832,721746013,3108340040,51486496,809566212,738389160,1175069036,67112972,2344263873,679844016,4206872,176358416,369280047,1587593378,25743248,404783106,373389908,587530426,2181040134,1173180512,335731801,2103436,3309404680,188835351,274948176,823783947,68157504,3492792963,549852610,4206872,3934454800,358797871,823664838,68157504,3560180355,549852418,94908720,538479865,1099040768,560732709,2349883915,134225944,439697794,1644884333,538479632,1099040768,3245218373,1477468682,2088952856,1051718,849395972,2242951181,205916208,3238264848,3096263328,137397656,1051718,1117831428,2239805709,202115120,591282554,2181040134,111759456,402843741,134619908,1348502016,3697100461,587470850,2181040134,2259767392,402837597,2970039812,51486496,809566212,788749480,2181980508,262929,1588080705,1633038115,51479054,809566212,792944808,34496848,2421851027,33654991,1410867328,2987901362,2294351648,545260033,3518190616,119582487,33654977,1410867328,2887246260,3313567520,1741172289,1073807556,3676965904,2287668480,12869763,202391553,2366167978,1619232857,1073807556,3693743120,2287471888,2699083907,1647567895,136315008,2423166214,1103113862,8413744,3573942304,767988846,1647329728,136315008,2557416710,1103113478,185639776,1076959730,2198081536,3276814282,404833813,268451889,938115844,1175806802,1076959265,2198081536,55852042,807486997,4177897553,2103436,625050120,186756126,411832482,2181562400,2904438081,279055089,2103436,1161921032,180465182,676860066,1182565092,67112972,254976193,1350938840,269239816,2697004032,2564329274,1174950021,67112972,256024769,1350926553,2048134152,102972993,1619132424,1829218640,69731048,525859,2907725954,48000327,102958120,1619132424,1837609296,69731024,564332502,67309983,2821734656,4097246172,293745760,1090520067,601794608,878767671,15207170,2802259514,51486497,809566212,931390632,2184503516,262929,3869782081,3772135331,3490382388,106314240,295641515,1090520067,65841200,895534140,67309954,2821734656,3191612398,3760982229,1809408928,12871624,202391553,253820202,293746155,1090520067,2750851120,148350524,866156588,536907874,2286421512,3699230616,590642119,2181040390,2290384992,2110250105,1647462120,136315024,2559068166,1205632455,102962299,1619132425,2038989952,3095250908,2422353991,101195776,2274920520,2076695678,1099482852,404783106,509747488,3122622970,102964113,1619132425,2038989952,3028142056,411856455,2181562404,3860992513,4059504433,1647425822,136315024,2559068166,2279374023,2294735982,545260097,1646338072,527561502,589009389,2181040390,2290384992,2110248057,2356037564,134226968,571605378,4146024934,834214817,536907874,2286421512,3699132312,3336862663,2147631496,555751456,1911627362,462535199,591395,2223005826,3351542152,1850193021,32,0,}},{"DXIL_Reflection_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_ubo",{0,0,0,"","global_ubo","vector",0,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inNormal",{0,0,1,"LOCATION","inNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inPosition",{0,0,0,"LOCATION","inPosition","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inTexCoord",{0,0,2,"LOCATION","inTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.inColor",{0,0,3,"LOCATION","inColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},
};