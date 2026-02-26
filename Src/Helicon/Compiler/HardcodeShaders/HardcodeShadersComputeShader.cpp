#include"HardcodeShaders.h"
std::unordered_map<std::string, std::variant<EmbeddedShader::ShaderCodeModule::ShaderResources,std::variant<std::vector<uint32_t>,std::string>>> EmbeddedShader::HardcodeShaders::hardcodeShadersComputeShader = {{"SpirV_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_283_column_37", std::vector<uint32_t>{119734787,67072,524299,130,0,131089,1,131089,5302,393227,1,1280527431,1685353262,808793134,0,196622,0,1,655375,5,4,1852399981,0,57,60,75,82,93,393232,4,17,8,8,1,196611,2,450,524292,1163873351,1851741272,1853189743,1919903337,1970364269,1718185057,7497065,524292,1163873351,1935627352,1634492771,1818386290,1600873327,1870225772,29813,262149,4,1852399981,0,589829,11,1936024417,1835821382,1867801449,1632462190,1920287600,1982358902,3879782,196613,10,120,196613,14,97,196613,16,98,196613,18,99,196613,20,100,196613,22,101,262149,53,1734438249,4475237,458757,54,1919906899,1113941857,1701209717,1784827762,7627621,327686,54,0,1734438249,4475237,524293,57,1919906931,1113941857,1701209717,1784827762,1937007461,0,327685,58,1752397136,1936617283,29556,524294,58,0,1919906931,1113941857,1701209717,1684949362,30821,524294,58,1,1718185589,1114468975,1701209717,1684949362,30821,327685,60,1752397168,1936617283,29556,262149,71,1869377379,114,458757,75,1970302569,1634552180,1196582247,909197634,0,524293,82,1197436007,1633841004,1986939244,1952539503,1231974249,68,393221,89,1701209701,1632007267,1919906915,0,458757,90,1651469383,1851092065,1919903337,1918980205,28001,393222,90,0,1651469415,1767140449,25965,393222,90,1,1651469415,1666411617,6646881,393222,90,2,1835102822,1970226021,29806,327686,90,3,1684300144,6778473,393221,93,1651469415,1632660577,1936548210,0,393221,106,1969906785,1684370547,1869377347,114,262149,120,1634886000,109,196679,54,2,327752,54,0,35,0,262215,57,33,0,262215,57,34,1,196679,58,2,327752,58,0,35,0,327752,58,1,35,4,262215,75,33,0,262215,75,34,2,262215,82,11,28,196679,90,2,327752,90,0,35,0,327752,90,1,35,4,327752,90,2,35,8,327752,90,3,35,12,262215,93,33,0,262215,93,34,3,131091,2,196641,3,2,196630,6,32,262167,7,6,3,262176,8,7,7,262177,9,7,8,262176,13,7,6,262187,6,15,1075880919,262187,6,17,1022739087,262187,6,19,1075545375,262187,6,21,1058474557,262187,6,23,1041194025,262187,6,44,0,262187,6,45,1065353216,262165,51,32,0,262176,52,7,51,196638,54,51,196637,55,54,262176,56,12,55,262203,56,57,12,262174,58,51,51,262176,59,9,58,262203,59,60,9,262165,61,32,1,262187,61,62,0,262176,63,9,51,262176,66,12,51,262167,69,6,4,262176,70,7,69,589849,72,6,1,0,0,0,2,2,196637,73,72,262176,74,0,73,262203,74,75,0,262176,77,0,72,262167,80,51,3,262176,81,1,80,262203,81,82,1,262167,83,51,2,262167,86,61,2,393246,90,6,6,51,51,196637,91,90,262176,92,2,91,262203,92,93,2,262187,61,94,1,262176,97,2,6,262187,6,100,1073741824,262187,6,103,1056964608,262187,6,110,1045220557,262187,51,127,8,262187,51,128,1,393260,80,129,127,127,128,327734,2,4,0,3,131320,5,262203,52,53,7,262203,70,71,7,262203,13,89,7,262203,8,106,7,262203,8,120,7,327745,63,64,60,62,262205,51,65,64,393281,66,67,57,65,62,262205,51,68,67,196670,53,68,262205,51,76,53,327745,77,78,75,76,262205,72,79,78,262205,80,84,82,458831,83,85,84,84,0,1,262268,86,87,85,327778,69,88,79,87,196670,71,88,327745,63,95,60,94,262205,51,96,95,393281,97,98,93,96,62,262205,6,99,98,327813,6,101,99,100,393228,6,102,1,13,101,327813,6,104,102,103,327809,6,105,104,103,196670,89,105,262205,69,107,71,524367,7,108,107,107,0,1,2,262205,6,109,89,327813,6,111,109,110,327809,6,112,45,111,327822,7,113,108,112,196670,106,113,262205,51,114,53,327745,77,115,75,114,262205,72,116,115,262205,80,117,82,458831,83,118,117,117,0,1,262268,86,119,118,262205,7,121,106,196670,120,121,327737,7,122,11,120,327761,6,123,122,0,327761,6,124,122,1,327761,6,125,122,2,458832,69,126,123,124,125,45,262243,116,119,126,65789,65592,327734,7,11,0,9,196663,8,10,131320,12,262203,13,14,7,262203,13,16,7,262203,13,18,7,262203,13,20,7,262203,13,22,7,196670,14,15,196670,16,17,196670,18,19,196670,20,21,196670,22,23,262205,7,24,10,262205,6,25,14,262205,7,26,10,327822,7,27,26,25,262205,6,28,16,393296,7,29,28,28,28,327809,7,30,27,29,327813,7,31,24,30,262205,7,32,10,262205,6,33,18,262205,7,34,10,327822,7,35,34,33,262205,6,36,20,393296,7,37,36,36,36,327809,7,38,35,37,327813,7,39,32,38,262205,6,40,22,393296,7,41,40,40,40,327809,7,42,39,41,327816,7,43,31,42,393296,7,46,44,44,44,393296,7,47,45,45,45,524300,7,48,1,43,43,46,47,131326,48,65592,}},{"SpirV_Reflection_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_283_column_37", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"pushConsts.uniformBufferIndex",{0,0,0,"","uniformBufferIndex","",0,4,4,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"GLSL_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_283_column_37", R"(#version 460
#extension GL_EXT_nonuniform_qualifier : require
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 1, binding = 0, std430) buffer StorageBufferObject
{
    uint imageID;
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

layout(set = 2, binding = 0, rgba16f) uniform image2D inputImageRGBA16[];

vec3 acesFilmicToneMapCurve(vec3 x)
{
    float a = 2.5099999904632568359375;
    float b = 0.02999999932944774627685546875;
    float c = 2.4300000667572021484375;
    float d = 0.589999973773956298828125;
    float e = 0.14000000059604644775390625;
    return clamp((x * ((x * a) + vec3(b))) / ((x * ((x * c) + vec3(d))) + vec3(e)), vec3(0.0), vec3(1.0));
}

void main()
{
    uint imageID = storageBufferObjects[pushConsts.storageBufferIndex].imageID;
    vec4 color = imageLoad(inputImageRGBA16[imageID], ivec2(gl_GlobalInvocationID.xy));
    float effectFactor = (sin(globalParams[pushConsts.uniformBufferIndex].globalTime * 2.0) * 0.5) + 0.5;
    vec3 adjustedColor = color.xyz * (1.0 + (effectFactor * 0.20000000298023223876953125));
    vec3 param = adjustedColor;
    imageStore(inputImageRGBA16[imageID], ivec2(gl_GlobalInvocationID.xy), vec4(acesFilmicToneMapCurve(param), 1.0));
}

)"},{"GLSL_Reflection_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_283_column_37", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"pushConsts.uniformBufferIndex",{0,0,0,"","uniformBufferIndex","",0,4,4,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"HLSL_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_283_column_37", R"(RWByteAddressBuffer storageBufferObjects[] : register(u0, space1);
struct GlobalUniformParam_1
{
    float globalTime;
    float globalScale;
    uint frameCount;
    uint padding;
};

ConstantBuffer<GlobalUniformParam_1> globalParams[] : register(b0, space3);
cbuffer PushConsts
{
    uint pushConsts_storageBufferIndex : packoffset(c0);
    uint pushConsts_uniformBufferIndex : packoffset(c0.y);
};

RWTexture2D<float4> inputImageRGBA16[] : register(u0, space2);

static uint3 gl_GlobalInvocationID;
struct SPIRV_Cross_Input
{
    uint3 gl_GlobalInvocationID : SV_DispatchThreadID;
};

float3 acesFilmicToneMapCurve(float3 x)
{
    float a = 2.5099999904632568359375f;
    float b = 0.02999999932944774627685546875f;
    float c = 2.4300000667572021484375f;
    float d = 0.589999973773956298828125f;
    float e = 0.14000000059604644775390625f;
    return clamp((x * ((x * a) + b.xxx)) / ((x * ((x * c) + d.xxx)) + e.xxx), 0.0f.xxx, 1.0f.xxx);
}

void comp_main()
{
    uint imageID = storageBufferObjects[pushConsts_storageBufferIndex].Load<uint>(0);
    float4 color = inputImageRGBA16[imageID][int2(gl_GlobalInvocationID.xy)];
    float effectFactor = (sin(globalParams[pushConsts_uniformBufferIndex].globalTime * 2.0f) * 0.5f) + 0.5f;
    float3 adjustedColor = color.xyz * (1.0f + (effectFactor * 0.20000000298023223876953125f));
    float3 param = adjustedColor;
    inputImageRGBA16[imageID][int2(gl_GlobalInvocationID.xy)] = float4(acesFilmicToneMapCurve(param), 1.0f);
}

[numthreads(8, 8, 1)]
void main(SPIRV_Cross_Input stage_input)
{
    gl_GlobalInvocationID = stage_input.gl_GlobalInvocationID;
    comp_main();
}
)"},{"HLSL_Reflection_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_283_column_37", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"pushConsts.uniformBufferIndex",{0,0,0,"","uniformBufferIndex","",0,4,4,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},
};