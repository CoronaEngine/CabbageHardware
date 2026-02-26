#include"HardcodeShaders.h"
std::unordered_map<std::string, std::variant<EmbeddedShader::ShaderCodeModule::ShaderResources,std::variant<std::vector<uint32_t>,std::string>>> EmbeddedShader::HardcodeShaders::hardcodeShadersVertexShader = {{"SpirV_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", std::vector<uint32_t>{119734787,67072,524299,180,0,131089,1,131089,5302,393227,1,1280527431,1685353262,808793134,0,196622,0,1,1114127,0,4,1852399981,0,16,19,33,119,133,143,155,167,171,172,176,178,196611,2,450,524292,1163873351,1851741272,1853189743,1919903337,1970364269,1718185057,7497065,262149,4,1852399981,0,327685,10,1818321779,1867342949,7103844,458757,13,1919906899,1113941857,1701209717,1784827762,7627621,458758,13,0,1954047348,1231385205,2019910766,0,327686,13,1,1701080941,108,327686,13,2,2003134838,0,327686,13,3,1785688688,0,327686,13,4,2003134838,7565136,393222,13,5,1751607660,1819231092,29295,393222,13,6,1751607660,1936674932,0,524293,16,1919906931,1113941857,1701209717,1784827762,1937007461,0,327685,17,1752397136,1936617283,29556,524294,17,0,1919906931,1113941857,1701209717,1684949362,30821,524294,17,1,1718185589,1114468975,1701209717,1684949362,30821,327685,19,1752397168,1936617283,29556,458757,30,1651469383,1851092065,1919903337,1918980205,28001,393222,30,0,1651469415,1767140449,25965,393222,30,1,1651469415,1666411617,6646881,393222,30,2,1835102822,1970226021,29806,327686,30,3,1684300144,6778473,393221,33,1651469415,1632660577,1936548210,0,393221,117,1348430951,1700164197,2019914866,0,393222,117,0,1348430951,1953067887,7237481,458758,117,1,1348430951,1953393007,1702521171,0,458758,117,2,1130327143,1148217708,1635021673,6644590,458758,117,3,1130327143,1147956341,1635021673,6644590,196613,119,0,327685,133,1867542121,1769236851,28271,262149,143,1734439526,7565136,327685,155,1734439526,1836216142,27745,327685,167,1867411049,1818324338,0,327685,171,1734439526,1869377347,114,262149,172,1866690153,7499628,393221,176,1734439526,1131963732,1685221231,0,327685,178,1700032105,1869562744,25714,196679,13,2,262216,13,0,24,327752,13,0,35,0,262216,13,1,5,327752,13,1,7,16,262216,13,1,24,327752,13,1,35,16,262216,13,2,5,327752,13,2,7,16,262216,13,2,24,327752,13,2,35,80,262216,13,3,5,327752,13,3,7,16,262216,13,3,24,327752,13,3,35,144,262216,13,4,24,327752,13,4,35,208,262216,13,5,24,327752,13,5,35,224,262216,13,6,24,327752,13,6,35,240,196679,16,24,262215,16,33,0,262215,16,34,1,196679,17,2,327752,17,0,35,0,327752,17,1,35,4,196679,30,2,327752,30,0,35,0,327752,30,1,35,4,327752,30,2,35,8,327752,30,3,35,12,262215,33,33,0,262215,33,34,3,196679,117,2,327752,117,0,11,0,327752,117,1,11,1,327752,117,2,11,3,327752,117,3,11,4,262215,133,30,0,262215,143,30,0,262215,155,30,1,262215,167,30,1,262215,171,30,3,262215,172,30,3,262215,176,30,2,262215,178,30,2,131091,2,196641,3,2,196630,6,32,262167,7,6,4,262168,8,7,4,262176,9,7,8,262165,11,32,0,262167,12,6,3,589854,13,11,8,8,8,12,12,12,196637,14,13,262176,15,12,14,262203,15,16,12,262174,17,11,11,262176,18,9,17,262203,18,19,9,262165,20,32,1,262187,20,21,0,262176,22,9,11,262187,20,25,1,262176,26,12,7,393246,30,6,6,11,11,196637,31,30,262176,32,2,31,262203,32,33,2,262176,36,2,6,262187,11,42,3,262176,43,12,6,262187,20,70,2,262187,20,89,3,262187,6,92,1065353216,262187,6,93,0,262187,11,115,1,262172,116,6,115,393246,117,7,6,116,116,262176,118,3,117,262203,118,119,3,262176,122,12,8,262176,132,1,12,262203,132,133,1,262176,140,3,7,262176,142,3,12,262203,142,143,3,262203,142,155,3,262168,159,12,3,262203,132,167,1,262203,142,171,3,262203,132,172,1,262167,174,6,2,262176,175,3,174,262203,175,176,3,262176,177,1,174,262203,177,178,1,327734,2,4,0,3,131320,5,262203,9,10,7,327745,22,23,19,21,262205,11,24,23,458817,26,27,16,24,25,21,262205,7,28,27,524367,12,29,28,28,0,1,2,327745,22,34,19,25,262205,11,35,34,393281,36,37,33,35,25,262205,6,38,37,327822,12,39,29,38,327745,22,40,19,21,262205,11,41,40,524353,43,44,16,41,25,21,42,262205,6,45,44,327761,6,46,39,0,327761,6,47,39,1,327761,6,48,39,2,458832,7,49,46,47,48,45,327745,22,50,19,21,262205,11,51,50,458817,26,52,16,51,25,25,262205,7,53,52,524367,12,54,53,53,0,1,2,327745,22,55,19,25,262205,11,56,55,393281,36,57,33,56,25,262205,6,58,57,327822,12,59,54,58,327745,22,60,19,21,262205,11,61,60,524353,43,62,16,61,25,25,42,262205,6,63,62,327761,6,64,59,0,327761,6,65,59,1,327761,6,66,59,2,458832,7,67,64,65,66,63,327745,22,68,19,21,262205,11,69,68,458817,26,71,16,69,25,70,262205,7,72,71,524367,12,73,72,72,0,1,2,327745,22,74,19,25,262205,11,75,74,393281,36,76,33,75,25,262205,6,77,76,327822,12,78,73,77,327745,22,79,19,21,262205,11,80,79,524353,43,81,16,80,25,70,42,262205,6,82,81,327761,6,83,78,0,327761,6,84,78,1,327761,6,85,78,2,458832,7,86,83,84,85,82,327745,22,87,19,21,262205,11,88,87,458817,26,90,16,88,25,89,262205,7,91,90,327761,6,94,49,0,327761,6,95,49,1,327761,6,96,49,2,327761,6,97,49,3,327761,6,98,67,0,327761,6,99,67,1,327761,6,100,67,2,327761,6,101,67,3,327761,6,102,86,0,327761,6,103,86,1,327761,6,104,86,2,327761,6,105,86,3,327761,6,106,91,0,327761,6,107,91,1,327761,6,108,91,2,327761,6,109,91,3,458832,7,110,94,95,96,97,458832,7,111,98,99,100,101,458832,7,112,102,103,104,105,458832,7,113,106,107,108,109,458832,8,114,110,111,112,113,196670,10,114,327745,22,120,19,21,262205,11,121,120,393281,122,123,16,121,89,262205,8,124,123,327745,22,125,19,21,262205,11,126,125,393281,122,127,16,126,70,262205,8,128,127,327826,8,129,124,128,262205,8,130,10,327826,8,131,129,130,262205,12,134,133,327761,6,135,134,0,327761,6,136,134,1,327761,6,137,134,2,458832,7,138,135,136,137,92,327825,7,139,131,138,327745,140,141,119,21,196670,141,139,262205,8,144,10,262205,12,145,133,327761,6,146,145,0,327761,6,147,145,1,327761,6,148,145,2,458832,7,149,146,147,148,92,327825,7,150,144,149,327761,6,151,150,0,327761,6,152,150,1,327761,6,153,150,2,393296,12,154,151,152,153,196670,143,154,262205,8,156,10,393228,8,157,1,34,156,262228,8,158,157,327761,7,160,158,0,524367,12,161,160,160,0,1,2,327761,7,162,158,1,524367,12,163,162,162,0,1,2,327761,7,164,158,2,524367,12,165,164,164,0,1,2,393296,159,166,161,163,165,262205,12,168,167,327825,12,169,166,168,393228,12,170,1,69,169,196670,155,170,262205,12,173,172,196670,171,173,262205,174,179,178,196670,176,179,65789,65592,}},{"SpirV_Reflection_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"fragPos",{0,0,0,"","fragPos","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragColor",{0,0,3,"","fragColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inTexCoord",{0,0,2,"","inTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragNormal",{0,0,1,"","fragNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},{"pushConsts.uniformBufferIndex",{0,0,0,"","uniformBufferIndex","",0,4,4,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"GLSL_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", R"(#version 460
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

)"},{"GLSL_Reflection_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"fragPos",{0,0,0,"","fragPos","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragColor",{0,0,3,"","fragColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inTexCoord",{0,0,2,"","inTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragNormal",{0,0,1,"","fragNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},{"pushConsts.uniformBufferIndex",{0,0,0,"","uniformBufferIndex","",0,4,4,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"HLSL_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", R"(ByteAddressBuffer storageBufferObjects[] : register(t0, space1);
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
)"},{"HLSL_Reflection_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"fragPos",{0,0,0,"","fragPos","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragColor",{0,0,3,"","fragColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"inTexCoord",{0,0,2,"","inTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"fragNormal",{0,0,1,"","fragNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},{"pushConsts.uniformBufferIndex",{0,0,0,"","uniformBufferIndex","",0,4,4,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},
};