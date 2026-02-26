#include"HardcodeShaders.h"
std::unordered_map<std::string, std::variant<EmbeddedShader::ShaderCodeModule::ShaderResources,std::variant<std::vector<uint32_t>,std::string>>> EmbeddedShader::HardcodeShaders::hardcodeShadersFragmentShader = {{"SpirV_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", std::vector<uint32_t>{119734787,67072,524299,348,0,131089,1,131089,5302,393227,1,1280527431,1685353262,808793134,0,196622,0,1,917519,4,4,1852399981,0,151,154,296,307,311,313,314,327,347,196624,4,7,196611,2,450,524292,1163873351,1851741272,1853189743,1919903337,1970364269,1718185057,7497065,262149,4,1852399981,0,589829,14,1953720644,1969383794,1852795252,676874055,993224310,993224310,3879270,196613,11,78,196613,12,72,327685,13,1735749490,1936027240,115,589829,19,1836016967,2037544037,1818780499,1198220137,1713920071,828783409,59,262149,17,1953457230,86,327685,18,1735749490,1936027240,115,655365,26,1836016967,2037544037,1953066323,1719019624,1719024435,1719024435,828783411,59,196613,22,78,196613,23,86,196613,24,76,327685,25,1735749490,1936027240,115,524293,31,1936028262,1399612782,1768712291,1713924963,1719024433,15155,327685,29,1416851299,1635018088,0,196613,30,12358,720901,39,1668047203,1952541813,1819231077,1982362223,1983591270,1983591270,1715155814,828783409,59,327685,34,1819438935,1936674916,0,262149,35,1836216142,27745,262149,36,1700949089,28516,327685,37,1635018093,1667853420,0,327685,38,1735749490,1936027240,115,196613,41,97,196613,45,12897,262149,49,1953457230,72,262149,55,1953457230,12872,196613,59,7171950,262149,61,1869505892,109,196613,78,114,196613,81,107,196613,87,7171950,262149,89,1869505892,109,262149,101,1953457230,86,262149,106,1953457230,76,262149,111,846751591,0,262149,112,1634886000,109,262149,114,1634886000,109,262149,117,829974375,0,262149,118,1634886000,109,262149,120,1634886000,109,196613,141,78,196613,144,86,458757,148,1919906899,1113941857,1701209717,1784827762,7627621,458758,148,0,1954047348,1231385205,2019910766,0,327686,148,1,1701080941,108,327686,148,2,2003134838,0,327686,148,3,1785688688,0,327686,148,4,2003134838,7565136,393222,148,5,1751607660,1819231092,29295,393222,148,6,1751607660,1936674932,0,524293,151,1919906931,1113941857,1701209717,1784827762,1937007461,0,327685,152,1752397136,1936617283,29556,524294,152,0,1919906931,1113941857,1701209717,1684949362,30821,524294,152,1,1718185589,1114468975,1701209717,1684949362,30821,327685,154,1752397168,1936617283,29556,196613,167,12358,196613,175,28492,196613,177,76,196613,186,72,327685,191,1702130785,1952544110,7237481,327685,192,1768186226,1701015137,0,196613,200,4605006,262149,201,1634886000,109,262149,203,1634886000,109,262149,205,1634886000,109,196613,208,71,262149,209,1634886000,109,262149,211,1634886000,109,262149,213,1634886000,109,262149,215,1634886000,109,196613,218,70,262149,223,1634886000,109,262149,224,1634886000,109,327685,227,1701672302,1869898098,114,327685,233,1869505892,1634625901,7499636,327685,247,1667592307,1918987381,0,196613,252,21355,196613,254,17515,262149,262,1953457230,76,262149,280,1768058209,7630437,262149,291,1869377379,114,327685,296,1954047348,1936028277,0,393221,307,1734439526,1131963732,1685221231,0,327685,311,1131705711,1919904879,0,327685,313,1867542121,1769236851,28271,327685,314,1867411049,1818324338,0,262149,327,1866690153,7499628,262149,330,1634886000,109,262149,332,1634886000,109,262149,334,1634886000,109,262149,336,1634886000,109,262149,337,1634886000,109,458757,343,1651469383,1851092065,1919903337,1918980205,28001,393222,343,0,1651469415,1767140449,25965,393222,343,1,1651469415,1666411617,6646881,393222,343,2,1835102822,1970226021,29806,327686,343,3,1684300144,6778473,393221,347,1651469415,1632660577,1936548210,0,196679,148,2,262216,148,0,24,327752,148,0,35,0,262216,148,1,5,327752,148,1,7,16,262216,148,1,24,327752,148,1,35,16,262216,148,2,5,327752,148,2,7,16,262216,148,2,24,327752,148,2,35,80,262216,148,3,5,327752,148,3,7,16,262216,148,3,24,327752,148,3,35,144,262216,148,4,24,327752,148,4,35,208,262216,148,5,24,327752,148,5,35,224,262216,148,6,24,327752,148,6,35,240,196679,151,24,262215,151,33,0,262215,151,34,1,196679,152,2,327752,152,0,35,0,327752,152,1,35,4,262215,296,33,0,262215,296,34,0,262215,307,30,2,262215,311,30,0,262215,313,30,0,262215,314,30,1,262215,327,30,3,196679,343,2,327752,343,0,35,0,327752,343,1,35,4,327752,343,2,35,8,327752,343,3,35,12,262215,347,33,0,262215,347,34,3,131091,2,196641,3,2,196630,6,32,262167,7,6,3,262176,8,7,7,262176,9,7,6,393249,10,6,8,8,9,327713,16,6,9,9,458785,21,6,8,8,8,9,327713,28,7,9,8,524321,33,7,8,8,8,9,9,262187,6,53,0,262187,6,64,1065353216,262187,6,68,1078530011,262187,6,85,1090519040,262187,6,135,1084227584,262165,145,32,0,262167,146,6,4,262168,147,146,4,589854,148,145,147,147,147,7,7,7,196637,149,148,262176,150,12,149,262203,150,151,12,262174,152,145,145,262176,153,9,152,262203,153,154,9,262165,155,32,1,262187,155,156,0,262176,157,9,145,262187,155,160,4,262176,161,12,7,262187,6,168,1025758986,393260,7,169,168,168,168,393260,7,176,53,53,53,262187,155,180,6,262187,155,195,5,262187,6,234,1082130432,262187,6,245,953267991,393260,7,255,64,64,64,262187,6,281,1022739087,393260,7,282,281,281,281,262176,290,7,146,589849,292,6,1,0,0,0,1,0,196635,293,292,196637,294,293,262176,295,0,294,262203,295,296,0,262176,299,12,145,262176,302,0,293,262167,305,6,2,262176,306,1,305,262203,306,307,1,262176,310,3,146,262203,310,311,3,262176,312,1,7,262203,312,313,1,262203,312,314,1,262187,145,315,3,262187,6,318,1008981770,131092,319,262203,312,327,1,262187,6,329,1056964608,393246,343,6,6,145,145,262187,145,344,1,262172,345,343,344,262176,346,2,345,262203,346,347,2,327734,2,4,0,3,131320,5,262203,290,291,7,262203,8,321,7,262203,8,330,7,262203,8,332,7,262203,8,334,7,262203,9,336,7,262203,9,337,7,327745,157,297,154,156,262205,145,298,297,393281,299,300,151,298,156,262205,145,301,300,327745,302,303,296,301,262205,293,304,303,262205,305,308,307,458840,146,309,304,308,2,53,196670,291,309,327745,9,316,291,315,262205,6,317,316,327866,319,320,317,318,196855,323,0,262394,320,322,326,131320,322,262205,146,324,291,524367,7,325,324,324,0,1,2,196670,321,325,131321,323,131320,326,262205,7,328,327,196670,321,328,131321,323,131320,323,262205,7,331,313,196670,330,331,262205,7,333,314,196670,332,333,262205,7,335,321,196670,334,335,196670,336,329,196670,337,329,589881,7,338,39,330,332,334,336,337,327761,6,339,338,0,327761,6,340,338,1,327761,6,341,338,2,458832,146,342,339,340,341,64,196670,311,342,65789,65592,327734,6,14,0,10,196663,8,11,196663,8,12,196663,9,13,131320,15,262203,9,41,7,262203,9,45,7,262203,9,49,7,262203,9,55,7,262203,9,59,7,262203,9,61,7,262205,6,42,13,262205,6,43,13,327813,6,44,42,43,196670,41,44,262205,6,46,41,262205,6,47,41,327813,6,48,46,47,196670,45,48,262205,7,50,11,262205,7,51,12,327828,6,52,50,51,458764,6,54,1,40,52,53,196670,49,54,262205,6,56,49,262205,6,57,49,327813,6,58,56,57,196670,55,58,262205,6,60,45,196670,59,60,262205,6,62,55,262205,6,63,45,327811,6,65,63,64,327813,6,66,62,65,327809,6,67,66,64,196670,61,67,262205,6,69,61,327813,6,70,68,69,262205,6,71,61,327813,6,72,70,71,196670,61,72,262205,6,73,59,262205,6,74,61,327816,6,75,73,74,131326,75,65592,327734,6,19,0,16,196663,9,17,196663,9,18,131320,20,262203,9,78,7,262203,9,81,7,262203,9,87,7,262203,9,89,7,262205,6,79,18,327809,6,80,79,64,196670,78,80,262205,6,82,78,262205,6,83,78,327813,6,84,82,83,327816,6,86,84,85,196670,81,86,262205,6,88,17,196670,87,88,262205,6,90,17,262205,6,91,81,327811,6,92,64,91,327813,6,93,90,92,262205,6,94,81,327809,6,95,93,94,196670,89,95,262205,6,96,87,262205,6,97,89,327816,6,98,96,97,131326,98,65592,327734,6,26,0,21,196663,8,22,196663,8,23,196663,8,24,196663,9,25,131320,27,262203,9,101,7,262203,9,106,7,262203,9,111,7,262203,9,112,7,262203,9,114,7,262203,9,117,7,262203,9,118,7,262203,9,120,7,262205,7,102,22,262205,7,103,23,327828,6,104,102,103,458764,6,105,1,40,104,53,196670,101,105,262205,7,107,22,262205,7,108,24,327828,6,109,107,108,458764,6,110,1,40,109,53,196670,106,110,262205,6,113,101,196670,112,113,262205,6,115,25,196670,114,115,393273,6,116,19,112,114,196670,111,116,262205,6,119,106,196670,118,119,262205,6,121,25,196670,120,121,393273,6,122,19,118,120,196670,117,122,262205,6,123,117,262205,6,124,111,327813,6,125,123,124,131326,125,65592,327734,7,31,0,28,196663,9,29,196663,8,30,131320,32,262205,7,128,30,262205,7,129,30,393296,7,130,64,64,64,327811,7,131,130,129,262205,6,132,29,327811,6,133,64,132,524300,6,134,1,43,133,53,64,458764,6,136,1,26,134,135,327822,7,137,131,136,327809,7,138,128,137,131326,138,65592,327734,7,39,0,33,196663,8,34,196663,8,35,196663,8,36,196663,9,37,196663,9,38,131320,40,262203,8,141,7,262203,8,144,7,262203,8,167,7,262203,8,175,7,262203,8,177,7,262203,8,186,7,262203,9,191,7,262203,8,192,7,262203,9,200,7,262203,8,201,7,262203,8,203,7,262203,9,205,7,262203,9,208,7,262203,8,209,7,262203,8,211,7,262203,8,213,7,262203,9,215,7,262203,8,218,7,262203,9,223,7,262203,8,224,7,262203,8,227,7,262203,9,233,7,262203,8,247,7,262203,8,252,7,262203,8,254,7,262203,9,262,7,262203,8,280,7,262205,7,142,35,393228,7,143,1,69,142,196670,141,143,327745,157,158,154,156,262205,145,159,158,393281,161,162,151,159,160,262205,7,163,162,262205,7,164,34,327811,7,165,163,164,393228,7,166,1,69,165,196670,144,166,196670,167,169,262205,7,170,167,262205,7,171,36,262205,6,172,37,393296,7,173,172,172,172,524300,7,174,1,46,170,171,173,196670,167,174,196670,175,176,327745,157,178,154,156,262205,145,179,178,393281,161,181,151,179,180,262205,7,182,181,262205,7,183,34,327811,7,184,182,183,393228,7,185,1,69,184,196670,177,185,262205,7,187,144,262205,7,188,177,327809,7,189,187,188,393228,7,190,1,69,189,196670,186,190,196670,191,64,327745,157,193,154,156,262205,145,194,193,393281,161,196,151,194,195,262205,7,197,196,262205,6,198,191,327822,7,199,197,198,196670,192,199,262205,7,202,141,196670,201,202,262205,7,204,186,196670,203,204,262205,6,206,38,196670,205,206,458809,6,207,14,201,203,205,196670,200,207,262205,7,210,141,196670,209,210,262205,7,212,144,196670,211,212,262205,7,214,177,196670,213,214,262205,6,216,38,196670,215,216,524345,6,217,26,209,211,213,215,196670,208,217,262205,7,219,186,262205,7,220,144,327828,6,221,219,220,524300,6,222,1,43,221,53,64,196670,223,222,262205,7,225,167,196670,224,225,393273,7,226,31,223,224,196670,218,226,262205,6,228,200,262205,6,229,208,327813,6,230,228,229,262205,7,231,218,327822,7,232,231,230,196670,227,232,262205,7,235,141,262205,7,236,144,327828,6,237,235,236,458764,6,238,1,40,237,53,327813,6,239,234,238,262205,7,240,141,262205,7,241,177,327828,6,242,240,241,458764,6,243,1,40,242,53,327813,6,244,239,243,327809,6,246,244,245,196670,233,246,262205,7,248,227,262205,6,249,233,393296,7,250,249,249,249,327816,7,251,248,250,196670,247,251,262205,7,253,218,196670,252,253,262205,7,256,252,327811,7,257,255,256,196670,254,257,262205,6,258,37,327811,6,259,64,258,262205,7,260,254,327822,7,261,260,259,196670,254,261,262205,7,263,141,262205,7,264,177,327828,6,265,263,264,458764,6,266,1,40,265,53,196670,262,266,262205,7,267,254,262205,7,268,36,327813,7,269,267,268,393296,7,270,68,68,68,327816,7,271,269,270,262205,7,272,247,327809,7,273,271,272,262205,7,274,192,327813,7,275,273,274,262205,6,276,262,327822,7,277,275,276,262205,7,278,175,327809,7,279,278,277,196670,175,279,262205,7,283,36,327813,7,284,282,283,196670,280,284,262205,7,285,280,262205,7,286,175,327809,7,287,285,286,131326,287,65592,}},{"SpirV_Reflection_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"textures",{0,0,0,"","textures","sampler2D",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(4)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"outColor",{0,0,0,"","outColor","float",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"GLSL_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", R"(#version 460
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
} globalParams[1];

layout(push_constant, std430) uniform PushConsts
{
    uint storageBufferIndex;
    uint uniformBufferIndex;
} pushConsts;

layout(set = 0, binding = 0) uniform sampler2D textures[];

layout(location = 2) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 3) in vec3 inColor;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0)) + 1.0;
    denom = (3.1415927410125732421875 * denom) * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    float nom = NdotV;
    float denom = (NdotV * (1.0 - k)) + k;
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float param = NdotV;
    float param_1 = roughness;
    float ggx2 = GeometrySchlickGGX(param, param_1);
    float param_2 = NdotL;
    float param_3 = roughness;
    float ggx1 = GeometrySchlickGGX(param_2, param_3);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + ((vec3(1.0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0));
}

vec3 calculateColor(vec3 WorldPos, vec3 Normal, vec3 albedo, float metallic, float roughness)
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(storageBufferObjects[pushConsts.storageBufferIndex].viewPos - WorldPos);
    vec3 F0 = vec3(0.039999999105930328369140625);
    F0 = mix(F0, albedo, vec3(metallic));
    vec3 Lo = vec3(0.0);
    vec3 L = normalize(storageBufferObjects[pushConsts.storageBufferIndex].lightPos - WorldPos);
    vec3 H = normalize(V + L);
    float attenuation = 1.0;
    vec3 radiance = storageBufferObjects[pushConsts.storageBufferIndex].lightColor * attenuation;
    vec3 param = N;
    vec3 param_1 = H;
    float param_2 = roughness;
    float NDF = DistributionGGX(param, param_1, param_2);
    vec3 param_3 = N;
    vec3 param_4 = V;
    vec3 param_5 = L;
    float param_6 = roughness;
    float G = GeometrySmith(param_3, param_4, param_5, param_6);
    float param_7 = clamp(dot(H, V), 0.0, 1.0);
    vec3 param_8 = F0;
    vec3 F = fresnelSchlick(param_7, param_8);
    vec3 numerator = F * (NDF * G);
    float denominator = ((4.0 * max(dot(N, V), 0.0)) * max(dot(N, L), 0.0)) + 9.9999997473787516355514526367188e-05;
    vec3 specular = numerator / vec3(denominator);
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - metallic);
    float NdotL = max(dot(N, L), 0.0);
    Lo += (((((kD * albedo) / vec3(3.1415927410125732421875)) + specular) * radiance) * NdotL);
    vec3 ambient = vec3(0.02999999932944774627685546875) * albedo;
    return ambient + Lo;
}

void main()
{
    vec4 color = textureLod(textures[storageBufferObjects[pushConsts.storageBufferIndex].textureIndex], fragTexCoord, 0.0);
    vec3 _321;
    if (color.w > 0.00999999977648258209228515625)
    {
        _321 = color.xyz;
    }
    else
    {
        _321 = inColor;
    }
    vec3 param = inPosition;
    vec3 param_1 = inNormal;
    vec3 param_2 = _321;
    float param_3 = 0.5;
    float param_4 = 0.5;
    outColor = vec4(calculateColor(param, param_1, param_2, param_3, param_4), 1.0);
}

)"},{"GLSL_Reflection_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"textures",{0,0,0,"","textures","sampler2D",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(4)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"outColor",{0,0,0,"","outColor","float",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"HLSL_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", R"(ByteAddressBuffer storageBufferObjects[] : register(t0, space1);
struct GlobalUniformParam_1
{
    float globalTime;
    float globalScale;
    uint frameCount;
    uint padding;
};

ConstantBuffer<GlobalUniformParam_1> globalParams[1] : register(b0, space3);
cbuffer PushConsts
{
    uint pushConsts_storageBufferIndex : packoffset(c0);
    uint pushConsts_uniformBufferIndex : packoffset(c0.y);
};

Texture2D<float4> textures[] : register(t0, space0);
SamplerState _textures_sampler[] : register(s0, space0);

static float2 fragTexCoord;
static float4 outColor;
static float3 inPosition;
static float3 inNormal;
static float3 inColor;

struct SPIRV_Cross_Input
{
    float3 inPosition : TEXCOORD0;
    float3 inNormal : TEXCOORD1;
    float2 fragTexCoord : TEXCOORD2;
    float3 inColor : TEXCOORD3;
};

struct SPIRV_Cross_Output
{
    float4 outColor : SV_Target0;
};

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f)) + 1.0f;
    denom = (3.1415927410125732421875f * denom) * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    float nom = NdotV;
    float denom = (NdotV * (1.0f - k)) + k;
    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float param = NdotV;
    float param_1 = roughness;
    float ggx2 = GeometrySchlickGGX(param, param_1);
    float param_2 = NdotL;
    float param_3 = roughness;
    float ggx1 = GeometrySchlickGGX(param_2, param_3);
    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + ((1.0f.xxx - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f));
}

float3 calculateColor(float3 WorldPos, float3 Normal, float3 albedo, float metallic, float roughness)
{
    float3 N = normalize(Normal);
    float3 V = normalize(storageBufferObjects[pushConsts_storageBufferIndex].Load<float3>(208) - WorldPos);
    float3 F0 = 0.039999999105930328369140625f.xxx;
    F0 = lerp(F0, albedo, metallic.xxx);
    float3 Lo = 0.0f.xxx;
    float3 L = normalize(storageBufferObjects[pushConsts_storageBufferIndex].Load<float3>(240) - WorldPos);
    float3 H = normalize(V + L);
    float attenuation = 1.0f;
    float3 radiance = storageBufferObjects[pushConsts_storageBufferIndex].Load<float3>(224) * attenuation;
    float3 param = N;
    float3 param_1 = H;
    float param_2 = roughness;
    float NDF = DistributionGGX(param, param_1, param_2);
    float3 param_3 = N;
    float3 param_4 = V;
    float3 param_5 = L;
    float param_6 = roughness;
    float G = GeometrySmith(param_3, param_4, param_5, param_6);
    float param_7 = clamp(dot(H, V), 0.0f, 1.0f);
    float3 param_8 = F0;
    float3 F = fresnelSchlick(param_7, param_8);
    float3 numerator = F * (NDF * G);
    float denominator = ((4.0f * max(dot(N, V), 0.0f)) * max(dot(N, L), 0.0f)) + 9.9999997473787516355514526367188e-05f;
    float3 specular = numerator / denominator.xxx;
    float3 kS = F;
    float3 kD = 1.0f.xxx - kS;
    kD *= (1.0f - metallic);
    float NdotL = max(dot(N, L), 0.0f);
    Lo += (((((kD * albedo) / 3.1415927410125732421875f.xxx) + specular) * radiance) * NdotL);
    float3 ambient = 0.02999999932944774627685546875f.xxx * albedo;
    return ambient + Lo;
}

void frag_main()
{
    float4 color = textures[storageBufferObjects[pushConsts_storageBufferIndex].Load<uint>(0)].SampleLevel(_textures_sampler[storageBufferObjects[pushConsts_storageBufferIndex].Load<uint>(0)], fragTexCoord, 0.0f);
    float3 _321;
    if (color.w > 0.00999999977648258209228515625f)
    {
        _321 = color.xyz;
    }
    else
    {
        _321 = inColor;
    }
    float3 param = inPosition;
    float3 param_1 = inNormal;
    float3 param_2 = _321;
    float param_3 = 0.5f;
    float param_4 = 0.5f;
    outColor = float4(calculateColor(param, param_1, param_2, param_3, param_4), 1.0f);
}

SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input)
{
    fragTexCoord = stage_input.fragTexCoord;
    inPosition = stage_input.inPosition;
    inNormal = stage_input.inNormal;
    inColor = stage_input.inColor;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.outColor = outColor;
    return stage_output;
}
)"},{"HLSL_Reflection_C__Users_Administrator_CLionProjects_CabbageHardware_Examples_main_cpp_line_232_column_42", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"textures",{0,0,0,"","textures","sampler2D",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(4)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"outColor",{0,0,0,"","outColor","float",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},
};