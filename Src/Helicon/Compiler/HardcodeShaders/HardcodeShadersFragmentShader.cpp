#include"HardcodeShaders.h"
std::unordered_map<std::string, std::variant<EmbeddedShader::ShaderCodeModule::ShaderResources,std::variant<std::vector<uint32_t>,std::string>>> EmbeddedShader::HardcodeShaders::hardcodeShadersFragmentShader = {{"SpirV_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_263_column_56", std::vector<uint32_t>{119734787,67072,524299,348,0,131089,1,131089,5302,393227,1,1280527431,1685353262,808793134,0,196622,0,1,917519,4,4,1852399981,0,151,154,296,307,311,313,314,327,347,196624,4,7,196611,2,450,524292,1163873351,1851741272,1853189743,1919903337,1970364269,1718185057,7497065,262149,4,1852399981,0,589829,14,1953720644,1969383794,1852795252,676874055,993224310,993224310,3879270,196613,11,78,196613,12,72,327685,13,1735749490,1936027240,115,589829,19,1836016967,2037544037,1818780499,1198220137,1713920071,828783409,59,262149,17,1953457230,86,327685,18,1735749490,1936027240,115,655365,26,1836016967,2037544037,1953066323,1719019624,1719024435,1719024435,828783411,59,196613,22,78,196613,23,86,196613,24,76,327685,25,1735749490,1936027240,115,524293,31,1936028262,1399612782,1768712291,1713924963,1719024433,15155,327685,29,1416851299,1635018088,0,196613,30,12358,720901,39,1668047203,1952541813,1819231077,1982362223,1983591270,1983591270,1715155814,828783409,59,327685,34,1819438935,1936674916,0,262149,35,1836216142,27745,262149,36,1700949089,28516,327685,37,1635018093,1667853420,0,327685,38,1735749490,1936027240,115,196613,41,97,196613,45,12897,262149,49,1953457230,72,262149,55,1953457230,12872,196613,59,7171950,262149,61,1869505892,109,196613,78,114,196613,81,107,196613,87,7171950,262149,89,1869505892,109,262149,101,1953457230,86,262149,106,1953457230,76,262149,111,846751591,0,262149,112,1634886000,109,262149,114,1634886000,109,262149,117,829974375,0,262149,118,1634886000,109,262149,120,1634886000,109,196613,141,78,196613,144,86,458757,148,1919906899,1113941857,1701209717,1784827762,7627621,458758,148,0,1954047348,1231385205,2019910766,0,327686,148,1,1701080941,108,327686,148,2,2003134838,0,327686,148,3,1785688688,0,327686,148,4,2003134838,7565136,393222,148,5,1751607660,1819231092,29295,393222,148,6,1751607660,1936674932,0,524293,151,1919906931,1113941857,1701209717,1784827762,1937007461,0,327685,152,1752397136,1936617283,29556,524294,152,0,1919906931,1113941857,1701209717,1684949362,30821,524294,152,1,1718185589,1114468975,1701209717,1684949362,30821,327685,154,1752397168,1936617283,29556,196613,167,12358,196613,175,28492,196613,177,76,196613,186,72,327685,191,1702130785,1952544110,7237481,327685,192,1768186226,1701015137,0,196613,200,4605006,262149,201,1634886000,109,262149,203,1634886000,109,262149,205,1634886000,109,196613,208,71,262149,209,1634886000,109,262149,211,1634886000,109,262149,213,1634886000,109,262149,215,1634886000,109,196613,218,70,262149,223,1634886000,109,262149,224,1634886000,109,327685,227,1701672302,1869898098,114,327685,233,1869505892,1634625901,7499636,327685,247,1667592307,1918987381,0,196613,252,21355,196613,254,17515,262149,262,1953457230,76,262149,280,1768058209,7630437,262149,291,1869377379,114,327685,296,1954047348,1936028277,0,393221,307,1734439526,1131963732,1685221231,0,327685,311,1131705711,1919904879,0,327685,313,1867542121,1769236851,28271,327685,314,1867411049,1818324338,0,262149,327,1866690153,7499628,262149,330,1634886000,109,262149,332,1634886000,109,262149,334,1634886000,109,262149,336,1634886000,109,262149,337,1634886000,109,458757,343,1651469383,1851092065,1919903337,1918980205,28001,393222,343,0,1651469415,1767140449,25965,393222,343,1,1651469415,1666411617,6646881,393222,343,2,1835102822,1970226021,29806,327686,343,3,1684300144,6778473,393221,347,1651469415,1632660577,1936548210,0,196679,148,2,262216,148,0,24,327752,148,0,35,0,262216,148,1,5,327752,148,1,7,16,262216,148,1,24,327752,148,1,35,16,262216,148,2,5,327752,148,2,7,16,262216,148,2,24,327752,148,2,35,80,262216,148,3,5,327752,148,3,7,16,262216,148,3,24,327752,148,3,35,144,262216,148,4,24,327752,148,4,35,208,262216,148,5,24,327752,148,5,35,224,262216,148,6,24,327752,148,6,35,240,196679,151,24,262215,151,33,0,262215,151,34,1,196679,152,2,327752,152,0,35,0,327752,152,1,35,4,262215,296,33,0,262215,296,34,0,262215,307,30,2,262215,311,30,0,262215,313,30,0,262215,314,30,1,262215,327,30,3,196679,343,2,327752,343,0,35,0,327752,343,1,35,4,327752,343,2,35,8,327752,343,3,35,12,262215,347,33,0,262215,347,34,3,131091,2,196641,3,2,196630,6,32,262167,7,6,3,262176,8,7,7,262176,9,7,6,393249,10,6,8,8,9,327713,16,6,9,9,458785,21,6,8,8,8,9,327713,28,7,9,8,524321,33,7,8,8,8,9,9,262187,6,53,0,262187,6,64,1065353216,262187,6,68,1078530011,262187,6,85,1090519040,262187,6,135,1084227584,262165,145,32,0,262167,146,6,4,262168,147,146,4,589854,148,145,147,147,147,7,7,7,196637,149,148,262176,150,12,149,262203,150,151,12,262174,152,145,145,262176,153,9,152,262203,153,154,9,262165,155,32,1,262187,155,156,0,262176,157,9,145,262187,155,160,4,262176,161,12,7,262187,6,168,1025758986,393260,7,169,168,168,168,393260,7,176,53,53,53,262187,155,180,6,262187,155,195,5,262187,6,234,1082130432,262187,6,245,953267991,393260,7,255,64,64,64,262187,6,281,1022739087,393260,7,282,281,281,281,262176,290,7,146,589849,292,6,1,0,0,0,1,0,196635,293,292,196637,294,293,262176,295,0,294,262203,295,296,0,262176,299,12,145,262176,302,0,293,262167,305,6,2,262176,306,1,305,262203,306,307,1,262176,310,3,146,262203,310,311,3,262176,312,1,7,262203,312,313,1,262203,312,314,1,262187,145,315,3,262187,6,318,1008981770,131092,319,262203,312,327,1,262187,6,329,1056964608,393246,343,6,6,145,145,262187,145,344,1,262172,345,343,344,262176,346,2,345,262203,346,347,2,327734,2,4,0,3,131320,5,262203,290,291,7,262203,8,321,7,262203,8,330,7,262203,8,332,7,262203,8,334,7,262203,9,336,7,262203,9,337,7,327745,157,297,154,156,262205,145,298,297,393281,299,300,151,298,156,262205,145,301,300,327745,302,303,296,301,262205,293,304,303,262205,305,308,307,458840,146,309,304,308,2,53,196670,291,309,327745,9,316,291,315,262205,6,317,316,327866,319,320,317,318,196855,323,0,262394,320,322,326,131320,322,262205,146,324,291,524367,7,325,324,324,0,1,2,196670,321,325,131321,323,131320,326,262205,7,328,327,196670,321,328,131321,323,131320,323,262205,7,331,313,196670,330,331,262205,7,333,314,196670,332,333,262205,7,335,321,196670,334,335,196670,336,329,196670,337,329,589881,7,338,39,330,332,334,336,337,327761,6,339,338,0,327761,6,340,338,1,327761,6,341,338,2,458832,146,342,339,340,341,64,196670,311,342,65789,65592,327734,6,14,0,10,196663,8,11,196663,8,12,196663,9,13,131320,15,262203,9,41,7,262203,9,45,7,262203,9,49,7,262203,9,55,7,262203,9,59,7,262203,9,61,7,262205,6,42,13,262205,6,43,13,327813,6,44,42,43,196670,41,44,262205,6,46,41,262205,6,47,41,327813,6,48,46,47,196670,45,48,262205,7,50,11,262205,7,51,12,327828,6,52,50,51,458764,6,54,1,40,52,53,196670,49,54,262205,6,56,49,262205,6,57,49,327813,6,58,56,57,196670,55,58,262205,6,60,45,196670,59,60,262205,6,62,55,262205,6,63,45,327811,6,65,63,64,327813,6,66,62,65,327809,6,67,66,64,196670,61,67,262205,6,69,61,327813,6,70,68,69,262205,6,71,61,327813,6,72,70,71,196670,61,72,262205,6,73,59,262205,6,74,61,327816,6,75,73,74,131326,75,65592,327734,6,19,0,16,196663,9,17,196663,9,18,131320,20,262203,9,78,7,262203,9,81,7,262203,9,87,7,262203,9,89,7,262205,6,79,18,327809,6,80,79,64,196670,78,80,262205,6,82,78,262205,6,83,78,327813,6,84,82,83,327816,6,86,84,85,196670,81,86,262205,6,88,17,196670,87,88,262205,6,90,17,262205,6,91,81,327811,6,92,64,91,327813,6,93,90,92,262205,6,94,81,327809,6,95,93,94,196670,89,95,262205,6,96,87,262205,6,97,89,327816,6,98,96,97,131326,98,65592,327734,6,26,0,21,196663,8,22,196663,8,23,196663,8,24,196663,9,25,131320,27,262203,9,101,7,262203,9,106,7,262203,9,111,7,262203,9,112,7,262203,9,114,7,262203,9,117,7,262203,9,118,7,262203,9,120,7,262205,7,102,22,262205,7,103,23,327828,6,104,102,103,458764,6,105,1,40,104,53,196670,101,105,262205,7,107,22,262205,7,108,24,327828,6,109,107,108,458764,6,110,1,40,109,53,196670,106,110,262205,6,113,101,196670,112,113,262205,6,115,25,196670,114,115,393273,6,116,19,112,114,196670,111,116,262205,6,119,106,196670,118,119,262205,6,121,25,196670,120,121,393273,6,122,19,118,120,196670,117,122,262205,6,123,117,262205,6,124,111,327813,6,125,123,124,131326,125,65592,327734,7,31,0,28,196663,9,29,196663,8,30,131320,32,262205,7,128,30,262205,7,129,30,393296,7,130,64,64,64,327811,7,131,130,129,262205,6,132,29,327811,6,133,64,132,524300,6,134,1,43,133,53,64,458764,6,136,1,26,134,135,327822,7,137,131,136,327809,7,138,128,137,131326,138,65592,327734,7,39,0,33,196663,8,34,196663,8,35,196663,8,36,196663,9,37,196663,9,38,131320,40,262203,8,141,7,262203,8,144,7,262203,8,167,7,262203,8,175,7,262203,8,177,7,262203,8,186,7,262203,9,191,7,262203,8,192,7,262203,9,200,7,262203,8,201,7,262203,8,203,7,262203,9,205,7,262203,9,208,7,262203,8,209,7,262203,8,211,7,262203,8,213,7,262203,9,215,7,262203,8,218,7,262203,9,223,7,262203,8,224,7,262203,8,227,7,262203,9,233,7,262203,8,247,7,262203,8,252,7,262203,8,254,7,262203,9,262,7,262203,8,280,7,262205,7,142,35,393228,7,143,1,69,142,196670,141,143,327745,157,158,154,156,262205,145,159,158,393281,161,162,151,159,160,262205,7,163,162,262205,7,164,34,327811,7,165,163,164,393228,7,166,1,69,165,196670,144,166,196670,167,169,262205,7,170,167,262205,7,171,36,262205,6,172,37,393296,7,173,172,172,172,524300,7,174,1,46,170,171,173,196670,167,174,196670,175,176,327745,157,178,154,156,262205,145,179,178,393281,161,181,151,179,180,262205,7,182,181,262205,7,183,34,327811,7,184,182,183,393228,7,185,1,69,184,196670,177,185,262205,7,187,144,262205,7,188,177,327809,7,189,187,188,393228,7,190,1,69,189,196670,186,190,196670,191,64,327745,157,193,154,156,262205,145,194,193,393281,161,196,151,194,195,262205,7,197,196,262205,6,198,191,327822,7,199,197,198,196670,192,199,262205,7,202,141,196670,201,202,262205,7,204,186,196670,203,204,262205,6,206,38,196670,205,206,458809,6,207,14,201,203,205,196670,200,207,262205,7,210,141,196670,209,210,262205,7,212,144,196670,211,212,262205,7,214,177,196670,213,214,262205,6,216,38,196670,215,216,524345,6,217,26,209,211,213,215,196670,208,217,262205,7,219,186,262205,7,220,144,327828,6,221,219,220,524300,6,222,1,43,221,53,64,196670,223,222,262205,7,225,167,196670,224,225,393273,7,226,31,223,224,196670,218,226,262205,6,228,200,262205,6,229,208,327813,6,230,228,229,262205,7,231,218,327822,7,232,231,230,196670,227,232,262205,7,235,141,262205,7,236,144,327828,6,237,235,236,458764,6,238,1,40,237,53,327813,6,239,234,238,262205,7,240,141,262205,7,241,177,327828,6,242,240,241,458764,6,243,1,40,242,53,327813,6,244,239,243,327809,6,246,244,245,196670,233,246,262205,7,248,227,262205,6,249,233,393296,7,250,249,249,249,327816,7,251,248,250,196670,247,251,262205,7,253,218,196670,252,253,262205,7,256,252,327811,7,257,255,256,196670,254,257,262205,6,258,37,327811,6,259,64,258,262205,7,260,254,327822,7,261,260,259,196670,254,261,262205,7,263,141,262205,7,264,177,327828,6,265,263,264,458764,6,266,1,40,265,53,196670,262,266,262205,7,267,254,262205,7,268,36,327813,7,269,267,268,393296,7,270,68,68,68,327816,7,271,269,270,262205,7,272,247,327809,7,273,271,272,262205,7,274,192,327813,7,275,273,274,262205,6,276,262,327822,7,277,275,276,262205,7,278,175,327809,7,279,278,277,196670,175,279,262205,7,283,36,327813,7,284,282,283,196670,280,284,262205,7,285,280,262205,7,286,175,327809,7,287,285,286,131326,287,65592,}},{"SpirV_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_263_column_56", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"textures",{0,0,0,"","textures","sampler2D",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(4)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"outColor",{0,0,0,"","outColor","float",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"GLSL_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_263_column_56", R"(#version 460
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

)"},{"GLSL_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_263_column_56", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"textures",{0,0,0,"","textures","sampler2D",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(4)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"outColor",{0,0,0,"","outColor","float",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"HLSL_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_263_column_56", R"(ByteAddressBuffer storageBufferObjects[] : register(t0, space1);
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
)"},{"HLSL_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_263_column_56", ShaderCodeModule::ShaderResources{8,"pushConsts",{{"fragTexCoord",{0,0,2,"","fragTexCoord","float",2,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"GlobalUniformParam",{3,0,0,"","GlobalUniformParam","uniform",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"inNormal",{0,0,1,"","inNormal","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"textures",{0,0,0,"","textures","sampler2D",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(4)}},{"inColor",{0,0,3,"","inColor","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"inPosition",{0,0,0,"","inPosition","float",3,12,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(1)}},{"outColor",{0,0,0,"","outColor","float",4,16,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(2)}},{"pushConsts.storageBufferIndex",{0,0,0,"","storageBufferIndex","",0,4,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)}},}}},{"SpirV_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", std::vector<uint32_t>{119734787,66816,2621440,211,0,131089,1,393227,56,1280527431,1685353262,808793134,0,196622,0,1,851983,4,2,1852399981,0,71,30,34,202,18,20,24,26,196624,2,7,196611,11,1,262149,12,1601331574,49,262149,12,1601331574,49,458757,18,1970302569,1635135092,774922098,1734439526,7565136,524293,20,1970302569,1635135092,774922098,1734439526,1836216142,27745,589829,24,1970302569,1635135092,774922098,1734439526,1131963732,1685221231,0,524293,26,1970302569,1635135092,774922098,1734439526,1869377347,114,1048581,30,1651469415,1885301857,1835102817,1919251557,1869374047,1731095395,1633841004,1651859308,1818701423,1818321519,1918989919,1949185887,1970567269,25970,1048581,34,1651469415,1885301857,1835102817,1919251557,1869374047,1731095395,1633841004,1651859308,1818701423,1818321519,1918989919,1932408671,1819307361,29285,393221,36,1886216563,1231316332,1701273965,0,262149,37,1886216563,6579564,393221,48,1886216563,1231316332,1701273965,0,262149,49,1886216563,6579564,786437,63,1952533855,1400400242,1634889588,1717527911,1952542572,1597274164,1298952003,1919904353,828666995,12340,327686,63,0,1635017060,0,786437,67,1952533855,1400400242,1634889588,1717527911,1952542572,1597208627,1298952003,1919904353,828666995,12340,327686,67,0,1635017060,0,589829,62,1651469415,1969187937,1935634274,1668641396,1953718132,808726884,0,458758,62,0,1651469415,1985965153,811561569,0,458758,62,1,1651469415,1985965153,828338785,0,458758,62,2,1651469415,1985965153,845116001,0,458758,62,3,1651469415,1985965153,861893217,0,458758,62,4,1651469415,1985965153,878670433,0,458758,62,5,1651469415,1985965153,895447649,0,458758,62,6,1651469415,1985965153,912224865,0,720901,71,1651469415,1885301857,1835102817,1919251557,1869374047,1731095395,1633841004,1651859308,111,262149,115,1601331574,12849,262149,136,1601331574,14130,589829,202,1920233061,1768902777,1632662638,1601003890,1852399981,1918989870,3289951,262149,2,1852399981,0,262215,18,30,0,262215,20,30,1,262215,24,30,2,262215,26,30,3,262215,30,33,1,262215,30,34,0,262215,34,33,2,262215,34,34,0,262215,64,6,16,327752,63,0,35,0,262215,68,6,16,327752,67,0,35,0,196679,62,2,327752,62,0,35,0,327752,62,1,35,64,327752,62,2,35,112,327752,62,3,35,176,327752,62,4,35,240,327752,62,5,35,256,327752,62,6,35,272,262215,71,33,0,262215,71,34,0,262215,202,30,0,131091,1,196641,3,1,196630,6,32,262167,7,6,4,262167,10,6,3,262176,11,7,10,262176,17,1,10,262167,21,6,2,262176,23,1,21,589849,27,6,1,2,0,0,1,0,262176,29,0,27,131098,31,262176,33,0,31,196635,35,27,131092,40,262187,6,42,1008981770,262187,6,58,1025758986,393260,10,57,58,58,58,262187,6,60,1056964608,393260,10,59,60,60,60,262165,65,32,1,262187,65,66,4,262172,64,7,66,196638,63,64,262187,65,69,3,262172,68,10,69,196638,67,68,589854,62,63,67,63,63,10,10,10,262176,70,2,62,262176,72,2,10,262187,65,77,6,262187,6,86,0,262187,6,95,3211788288,262187,6,97,1065353216,262187,6,99,1078530012,393260,10,123,97,97,97,262187,6,134,1084227584,262187,6,143,1031798784,262187,6,145,1060634624,262187,6,147,1049624576,262187,6,171,1082130432,262187,6,177,953267991,262187,65,181,5,262187,6,195,1022739087,393260,10,194,195,195,195,262176,201,3,7,262203,17,18,1,262203,17,20,1,262203,23,24,1,262203,17,26,1,262203,29,30,0,262203,33,34,0,262203,70,71,2,262203,201,202,3,262187,6,209,1050868098,393260,10,210,209,209,209,327734,1,2,0,3,131320,4,262203,11,12,7,262205,10,16,18,262205,10,19,20,262205,21,22,24,262205,10,25,26,262205,27,28,30,262205,31,32,34,327766,35,36,28,32,393303,7,37,36,22,0,327761,6,39,37,3,327866,40,41,39,42,196855,15,0,262394,41,14,13,131320,13,196670,12,25,131321,15,131320,14,262205,27,46,30,262205,31,47,34,327766,35,48,46,47,393303,7,49,48,22,0,524367,10,51,49,49,0,1,2,196670,12,51,131321,15,131320,15,262205,10,54,12,524300,10,55,56,46,57,54,59,393228,10,61,56,69,19,327746,72,73,71,66,262205,10,74,73,327811,10,75,74,16,393228,10,76,56,69,75,327746,72,78,71,77,262205,10,79,78,327811,10,80,79,16,393228,10,81,56,69,80,327809,10,82,76,81,393228,10,83,56,69,82,327828,6,84,61,83,458764,6,85,56,40,84,86,393228,10,87,56,69,75,393228,10,88,56,69,80,327809,10,89,87,88,393228,10,90,56,69,89,327828,6,91,61,90,458764,6,92,56,40,91,86,327813,6,93,85,92,327813,6,94,93,95,327809,6,96,94,97,327813,6,98,99,96,393228,10,100,56,69,75,393228,10,101,56,69,80,327809,10,102,100,101,393228,10,103,56,69,102,327828,6,104,61,103,458764,6,105,56,40,104,86,393228,10,106,56,69,75,393228,10,107,56,69,80,327809,10,108,106,107,393228,10,109,56,69,108,327828,6,110,61,109,458764,6,111,56,40,110,86,327813,6,112,105,111,327813,6,113,112,95,327809,6,114,113,97,327813,6,115,98,114,393228,10,116,56,69,75,327828,6,117,61,116,458764,6,118,56,40,117,86,393228,10,119,56,69,80,327828,6,120,61,119,458764,6,121,56,40,120,86,327811,10,122,123,55,393228,10,124,56,69,75,393228,10,125,56,69,80,327809,10,126,124,125,393228,10,127,56,69,126,393228,10,128,56,69,75,327828,6,129,127,128,524300,6,130,56,43,129,86,97,327811,6,131,97,130,524300,6,132,56,43,131,86,97,458764,6,133,56,26,132,134,327822,10,135,122,133,327809,10,136,55,135,327811,10,137,123,136,327813,10,139,137,54,327813,10,141,139,210,327816,6,142,143,115,327813,6,144,121,145,327809,6,146,144,147,327816,6,148,121,146,327813,6,149,118,145,327809,6,150,149,147,327816,6,151,118,150,327813,6,152,148,151,327813,6,153,142,152,393228,10,154,56,69,75,393228,10,155,56,69,80,327809,10,156,154,155,393228,10,157,56,69,156,393228,10,158,56,69,75,327828,6,159,157,158,524300,6,160,56,43,159,86,97,327811,6,161,97,160,524300,6,162,56,43,161,86,97,458764,6,163,56,26,162,134,327822,10,164,122,163,327809,10,165,55,164,327822,10,166,165,153,393228,10,167,56,69,75,327828,6,168,61,167,458764,6,169,56,40,168,86,327813,6,170,171,169,393228,10,172,56,69,80,327828,6,173,61,172,458764,6,174,56,40,173,86,327813,6,175,170,174,327809,6,176,175,177,393296,10,178,176,176,176,327816,10,179,166,178,327809,10,180,141,179,327746,72,182,71,181,262205,10,183,182,327813,10,184,180,183,393228,10,185,56,69,80,327828,6,186,61,185,458764,6,187,56,40,186,86,327822,10,188,184,187,327813,10,193,194,54,327809,10,196,193,188,327760,7,197,196,97,196670,202,197,65789,65592,}},{"SpirV_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_var_2",{0,0,0,"","global_var_2","matrix",0,64,112,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_ubo",{0,0,0,"","global_ubo","ConstantBuffer",0,288,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"global_var_0",{0,0,0,"","global_var_0","matrix",0,64,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragNormal",{0,0,1,"LOCATION","fragNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_1",{0,0,0,"","global_var_1","matrix",0,48,64,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_3",{0,0,0,"","global_var_3","matrix",0,64,176,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_4",{0,0,0,"","global_var_4","vector",0,12,240,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_5",{0,0,0,"","global_var_5","vector",0,12,256,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_6",{0,0,0,"","global_var_6","vector",0,12,272,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragPos",{0,0,0,"LOCATION","fragPos","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_7.texture",{0,0,0,"","texture","_Texture",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(5)}},{"global_var_7.sampler",{0,0,0,"","sampler","SamplerState",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(6)}},{"input.var_0.fragTexCoord",{0,0,2,"LOCATION","fragTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragColor",{0,0,3,"LOCATION","fragColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"GLSL_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", R"(#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 561 0
struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    vec4  data_0[4];
};


#line 12399
struct _MatrixStorage_float3x3_ColMajorstd140_0
{
    vec3  data_1[3];
};


#line 24 1
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

#line 1875 0
layout(binding = 1)
uniform texture2D global_parameter_block_global_ubo_global_var_7_texture_0;


#line 890
layout(binding = 2)
uniform sampler global_parameter_block_global_ubo_global_var_7_sampler_0;


#line 12442
layout(location = 0)
out vec4 entryPointParam_main_var_32_0;


#line 12442
layout(location = 0)
in vec3 input_var_0_fragPos_0;


#line 12442
layout(location = 1)
in vec3 input_var_0_fragNormal_0;


#line 12442
layout(location = 2)
in vec2 input_var_0_fragTexCoord_0;


#line 12442
layout(location = 3)
in vec3 input_var_0_fragColor_0;


#line 34 1
struct fragment_output_0
{
    vec4 var_32_0;
};


#line 38
void main()
{

#line 39
    fragment_output_0 output_0;

    vec4 _S1 = (texture(sampler2D(global_parameter_block_global_ubo_global_var_7_texture_0,global_parameter_block_global_ubo_global_var_7_sampler_0), (input_var_0_fragTexCoord_0)));

#line 41
    vec3 var_1_0;

#line 41
    if((_S1.w) > 0.00999999977648258)
    {

#line 41
        var_1_0 = _S1.xyz;

#line 41
    }
    else
    {

#line 41
        var_1_0 = input_var_0_fragColor_0;

#line 41
    }

#line 50
    vec3 var_4_0 = mix(vec3(0.03999999910593033, 0.03999999910593033, 0.03999999910593033), var_1_0, vec3(0.5));

#line 58
    vec3 _S2 = normalize(input_var_0_fragNormal_0);

#line 58
    vec3 _S3 = normalize(global_parameter_block_global_ubo_0.global_var_4_0 - input_var_0_fragPos_0);

#line 58
    vec3 _S4 = normalize(global_parameter_block_global_ubo_0.global_var_6_0 - input_var_0_fragPos_0);

#line 58
    vec3 _S5 = normalize(_S3 + _S4);

#line 58
    float _S6 = max(dot(_S2, _S5), 0.0);

#line 58
    float _S7 = _S6 * _S6 * -0.9375 + 1.0;
    float var_13_0 = max(dot(_S2, _S3), 0.0);

#line 64
    float var_18_0 = max(dot(_S2, _S4), 0.0);

#line 70
    const vec3 var_24_0 = vec3(1.0, 1.0, 1.0);


    vec3 var_27_0 = var_4_0 + (var_24_0 - var_4_0) * pow(clamp(1.0 - clamp(dot(_S5, _S3), 0.0, 1.0), 0.0, 1.0), 5.0);

#line 79
    output_0.var_32_0 = vec4(vec3(0.02999999932944775, 0.02999999932944775, 0.02999999932944775) * var_1_0 + ((var_24_0 - var_27_0) * var_1_0 / 3.14159297943115234 + 0.0625 / (3.14159297943115234 * _S7 * _S7) * (var_18_0 / (var_18_0 * 0.71875 + 0.28125) * (var_13_0 / (var_13_0 * 0.71875 + 0.28125))) * var_27_0 / (4.0 * var_13_0 * var_18_0 + 0.00009999999747379)) * global_parameter_block_global_ubo_0.global_var_5_0 * var_18_0, 1.0);

#line 79
    entryPointParam_main_var_32_0 = output_0.var_32_0;

#line 79
    return;
}

)"},{"GLSL_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_var_2",{0,0,0,"","global_var_2","matrix",0,64,112,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_ubo",{0,0,0,"","global_ubo","ConstantBuffer",0,284,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"global_var_0",{0,0,0,"","global_var_0","matrix",0,64,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragNormal",{0,0,1,"LOCATION","fragNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_1",{0,0,0,"","global_var_1","matrix",0,44,64,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_3",{0,0,0,"","global_var_3","matrix",0,64,176,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_4",{0,0,0,"","global_var_4","vector",0,12,240,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_5",{0,0,0,"","global_var_5","vector",0,12,256,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_6",{0,0,0,"","global_var_6","vector",0,12,272,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragPos",{0,0,0,"LOCATION","fragPos","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_7.texture",{0,0,0,"","texture","_Texture",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(5)}},{"global_var_7.sampler",{0,0,0,"","sampler","SamplerState",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(6)}},{"input.var_0.fragTexCoord",{0,0,2,"LOCATION","fragTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragColor",{0,0,3,"LOCATION","fragColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"Slang_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", R"(struct aggregate_type_0 {
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
struct fragment_input {
	aggregate_type_2 var_0 : LOCATION0;
}
struct fragment_output {
	float4 var_32 : SV_TARGET0;
}
[shader("fragment")]
fragment_output main(fragment_input input) {
	fragment_output output;
	float3 var_1 = float3(0.000000, 0.000000, 0.000000);
	if ((global_parameter_block.global_ubo.global_var_7.texture.Sample(global_parameter_block.global_ubo.global_var_7.sampler,input.var_0.fragTexCoord).w > 0.010000)) {
		var_1 = global_parameter_block.global_ubo.global_var_7.texture.Sample(global_parameter_block.global_ubo.global_var_7.sampler,input.var_0.fragTexCoord).xyz;
	}
	else {
		var_1 = input.var_0.fragColor;
	}
	float var_2 = 0.500000;
	float var_3 = 0.500000;
	float3 var_4 = float3(0.040000, 0.040000, 0.040000);
	var_4 = lerp(var_4,var_1,var_3);
	float3 var_5 = float3(0.000000, 0.000000, 0.000000);
	float var_6 = 1.000000;
	float var_7 = var_2;
	float var_8 = ((var_7 * var_7) * (var_7 * var_7));
	float var_9 = 1.000000;
	float var_10 = 1.000000;
	float var_11 = 3.141593;
	float var_12 = ((var_11 * (((max(dot(normalize(input.var_0.fragNormal),normalize((normalize((global_parameter_block.global_ubo.global_var_4 - input.var_0.fragPos)) + normalize((global_parameter_block.global_ubo.global_var_6 - input.var_0.fragPos))))),0.000000) * max(dot(normalize(input.var_0.fragNormal),normalize((normalize((global_parameter_block.global_ubo.global_var_4 - input.var_0.fragPos)) + normalize((global_parameter_block.global_ubo.global_var_6 - input.var_0.fragPos))))),0.000000)) * (((var_7 * var_7) * (var_7 * var_7)) - var_9)) + var_10)) * (((max(dot(normalize(input.var_0.fragNormal),normalize((normalize((global_parameter_block.global_ubo.global_var_4 - input.var_0.fragPos)) + normalize((global_parameter_block.global_ubo.global_var_6 - input.var_0.fragPos))))),0.000000) * max(dot(normalize(input.var_0.fragNormal),normalize((normalize((global_parameter_block.global_ubo.global_var_4 - input.var_0.fragPos)) + normalize((global_parameter_block.global_ubo.global_var_6 - input.var_0.fragPos))))),0.000000)) * (((var_7 * var_7) * (var_7 * var_7)) - var_9)) + var_10));
	float var_13 = max(dot(normalize(input.var_0.fragNormal),normalize((global_parameter_block.global_ubo.global_var_4 - input.var_0.fragPos))),0.000000);
	float var_14 = 1.000000;
	float var_15 = 8.000000;
	float var_16 = var_13;
	float var_17 = 1.000000;
	float var_18 = max(dot(normalize(input.var_0.fragNormal),normalize((global_parameter_block.global_ubo.global_var_6 - input.var_0.fragPos))),0.000000);
	float var_19 = 1.000000;
	float var_20 = 8.000000;
	float var_21 = var_18;
	float var_22 = 1.000000;
	float var_23 = 1.000000;
	float3 var_24 = float3(1.000000, 1.000000, 1.000000);
	float var_25 = 4.000000;
	float var_26 = 0.000100;
	float3 var_27 = (var_4 + ((var_24 - var_4) * pow(clamp((var_23 - clamp(dot(normalize((normalize((global_parameter_block.global_ubo.global_var_4 - input.var_0.fragPos)) + normalize((global_parameter_block.global_ubo.global_var_6 - input.var_0.fragPos)))),normalize((global_parameter_block.global_ubo.global_var_4 - input.var_0.fragPos))),0.000000,1.000000)),0.000000,1.000000),5.000000)));
	float3 var_28 = float3(1.000000, 1.000000, 1.000000);
	float var_29 = 1.000000;
	float var_30 = 3.141593;
	var_5 = (var_5 + ((((((var_28 - var_27) * var_1) / var_30) + ((((var_8 / var_12) * ((var_21 / ((var_18 * (var_22 - (((var_2 + var_19) * (var_2 + var_19)) / var_20))) + (((var_2 + var_19) * (var_2 + var_19)) / var_20))) * (var_16 / ((var_13 * (var_17 - (((var_2 + var_14) * (var_2 + var_14)) / var_15))) + (((var_2 + var_14) * (var_2 + var_14)) / var_15))))) * (var_4 + ((var_24 - var_4) * pow(clamp((var_23 - clamp(dot(normalize((normalize((global_parameter_block.global_ubo.global_var_4 - input.var_0.fragPos)) + normalize((global_parameter_block.global_ubo.global_var_6 - input.var_0.fragPos)))),normalize((global_parameter_block.global_ubo.global_var_4 - input.var_0.fragPos))),0.000000,1.000000)),0.000000,1.000000),5.000000)))) / (((var_25 * max(dot(normalize(input.var_0.fragNormal),normalize((global_parameter_block.global_ubo.global_var_4 - input.var_0.fragPos))),0.000000)) * max(dot(normalize(input.var_0.fragNormal),normalize((global_parameter_block.global_ubo.global_var_6 - input.var_0.fragPos))),0.000000)) + var_26))) * (global_parameter_block.global_ubo.global_var_5 * var_6)) * max(dot(normalize(input.var_0.fragNormal),normalize((global_parameter_block.global_ubo.global_var_6 - input.var_0.fragPos))),0.000000)));
	float3 var_31 = float3(0.030000, 0.030000, 0.030000);
	output.var_32 = float4(((var_31 * var_1) + var_5),1.000000);
	return output;
}
)"},{"Slang_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_var_2",{0,0,0,"","global_var_2","matrix",0,64,112,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_ubo",{0,0,0,"","global_ubo","ConstantBuffer",0,288,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"global_var_0",{0,0,0,"","global_var_0","matrix",0,64,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragNormal",{0,0,1,"LOCATION","fragNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_1",{0,0,0,"","global_var_1","matrix",0,48,64,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_3",{0,0,0,"","global_var_3","matrix",0,64,176,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_4",{0,0,0,"","global_var_4","vector",0,12,240,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_5",{0,0,0,"","global_var_5","vector",0,12,256,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_6",{0,0,0,"","global_var_6","vector",0,12,272,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragPos",{0,0,0,"LOCATION","fragPos","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_7.texture",{0,0,0,"","texture","_Texture",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(5)}},{"global_var_7.sampler",{0,0,0,"","sampler","SamplerState",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(6)}},{"input.var_0.fragTexCoord",{0,0,2,"LOCATION","fragTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragColor",{0,0,3,"LOCATION","fragColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"DXIL_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", std::vector<uint32_t>{1128421444,2476567581,4068563320,1452523721,3612294149,1,6464,7,60,76,232,292,652,3184,3212,810108499,8,0,0,826757961,148,4,8,0,136,0,0,3,0,1799,0,0,136,1,0,3,1,1799,0,0,136,2,0,3,2,771,0,0,136,3,0,3,3,1799,0,1094930252,1313818964,0,826757967,52,1,8,0,40,0,64,3,0,15,0,1415534163,1701278305,116,810963792,352,52,0,0,0,0,0,4294967295,0,67109124,1,0,0,0,37,3,24,2,0,0,0,13,0,1,0,0,0,14,0,3,0,0,0,2,0,44,1129270272,1330205761,1330380878,1230258499,1275088463,1413563215,5132105,1094930252,1313818964,1767992576,110,4,0,1,2,3,16,1,0,4390913,515,10,1,4391169,515,19,2,4325889,515,28,3,4391681,515,0,0,272891905,3,7,7,7,0,7,7,7,0,7,7,0,0,1,2,4,0,1413567571,2524,102,631,1279875140,262,16,2500,3737142082,3105,622,2130443,2,19,2435023111,1225050177,959582214,209977746,419956005,1653277726,38082688,1108054594,338825412,1259866168,2288136714,538218568,2777171523,1110579456,2416855268,1355031313,2357285185,2330297313,105263364,6225,8,4292906011,134217727,229114432,4294963332,537133055,4286984301,536870911,11012352,6217,3,1113621011,101207072,0,8329,120,159916594,75850784,2225349395,3810726660,345022852,2358004754,1287947275,146915344,167774528,149291110,9330880,272680489,1368408388,2283831308,2799968354,3962041057,629089313,4237858212,827438818,1346421516,4184919921,2450585107,2966225439,173549584,856428707,272679692,1095581892,2249744902,703076449,336347331,2250269205,2258871937,2276499403,2741927056,4237858186,823244514,2286994188,2986893450,2133728110,4064419522,2826060375,3369214024,1114146937,67701000,1619396689,437176880,516063022,1113059906,1080463578,3306176772,173031708,2382944390,1635811770,2845336068,495044109,3860750786,2707951041,511832092,3390980586,2168983105,480373775,3726541254,3237486145,501340188,3625877954,3236432896,264290319,3658432756,1642077473,511802910,3390964210,1287914753,3960352866,821235216,188771855,4094656741,1358040912,171995662,3842990204,4042526256,130085390,4010721510,3237219856,121659398,2114322558,607126656,3015706929,613418372,145097119,1902312633,2110245248,820249884,2162438338,11572857,3224626322,1603739924,4180676140,3768811447,580537660,1669109834,4183818865,2784608425,3917783593,2723271079,3868879205,951157133,268218369,822870288,740282641,3338444942,4072670434,3555950931,3175696424,1243781705,1612086424,3632825758,336107529,73843728,167790385,0,3228701715,2271245447,2038917174,3228697448,1353584007,248540430,1829654650,813305600,127955463,1829183603,2691763856,119567111,2014220397,3497527200,118548742,1896325238,2423064416,119567118,1913073786,1625884368,127955975,1829191798,1618021984,118520327,3859206262,2691827504,119567111,1980653677,1618609984,114324487,2047312110,2692089616,119567111,1946640506,2148131888,0,0,1674076160,1048832,0,2428502016,1073873159,0,2147483648,67325729,192,0,127812352,32776,0,1015414784,16781329,0,201326592,2107001,2,0,1079570968,3072,0,3387174912,524416,0,3361734656,268501459,0,1073741824,2070,27,412622386,2420904217,1193675148,574817478,1176019018,614500864,278959882,169648330,62955568,3231734794,10701319,2694056457,2688338492,2194277748,18387594,674398760,3080733328,285278464,536976680,3598714485,1735000089,723372032,954435706,2151211904,1082130659,27027488,0,6265,244,2420900634,1142096454,3966700687,604786157,1170671046,3332769446,67520101,3322293317,650587174,1130704006,135285776,742425189,2558624795,114036768,3223859393,270036429,2350946434,252713601,3302938830,3770603714,3670205634,3838503114,3738748094,3468613318,3267682008,3303718616,3637427934,3636643038,3837979838,3904794302,3941134538,1623116516,860096550,228335681,1215091074,2352047372,228340097,1480668097,417346435,2310910364,3246240133,3045443973,3382038933,3182528893,2642259341,2240396721,2312469937,1287749053,57116688,1636975714,565187340,1829250720,2210662016,2973218355,3090134576,3065035056,3107109426,3082170671,867153329,816920502,2973413302,3056840631,3056644407,3106978607,3115293615,3057137328,2553264434,1566672909,3222811014,1088094342,1301626944,211866128,3182533864,2108786057,2110359001,1287748801,3278374928,80508368,136743041,553759876,217826872,2632476720,2240396721,2245623217,2093317577,3189271745,270571919,277087955,1040999939,2256732210,2614827470,2547718232,2547816541,80484300,540450945,3727694212,3495954039,2070100432,4217572115,4220718002,2575497626,114626592,3322103940,201381115,1818704410,1818321519,1918989919,811545695,2596406291,114757664,2944622724,1906360333,432967555,408460251,2556270555,399333340,3229680844,470032438,100692348,1954290573,3302938830,3971930306,1824449730,136732862,553760324,61071590,3772540976,1632403675,2959607376,119566086,1346449525,1087910659,1818743312,3627820560,496703568,1277051356,56624144,4163920304,70651776,409055357,638566646,110128648,542114331,3868595921,1776911430,3911878696,1350724041,3800306552,320344283,638321284,1813529864,3627040272,1084430512,174105866,2191536295,2963868241,741742625,1116743875,700623913,2486216716,141306434,4156689816,1368938605,1750238496,1310217093,1459956097,461115608,1209298838,3781170337,4166256148,907354442,180398608,1275793583,2831068326,3402556610,2231903976,2316323864,270571569,3017869018,831196549,188785675,1124749478,1889867821,1543641867,1120415960,2215837742,226697271,453751170,84950410,2172178525,1541085574,81583384,2757286017,3453078969,2382893133,3654966470,81337111,3429427329,3447561637,731976145,2071133123,2698734483,2150613752,3280484419,1464045624,907363086,2536946486,926069300,1346447802,3318680709,778980526,1248710125,3273733184,1661710707,2610113459,3363117074,3504758000,3838494914,3402161818,1208264408,389820953,3082400441,2997991738,1447111097,2244938889,749604654,2381172168,3979250734,2322492877,1350436752,3318680711,787262766,780823689,1248636556,3363113472,3704249584,2700272872,3906786014,3154421478,3473704400,3467504741,1850519181,240142958,0,6265,76,478152755,1713168836,2285699348,3280222275,125846156,1929869433,3859575192,283971328,243332110,507644979,3458056642,811998369,1133001989,461603896,3359441868,59522371,2356726845,2064085108,1215891208,124809351,1979936890,544245624,298588551,244378638,1846489313,4041412400,240185358,499388467,3625722334,1640111393,2301650462,998489147,3023651792,2210151427,990086204,1981083852,1752893280,2271753991,923232370,2423293824,123760775,1980180598,2021000696,2273343367,1904674911,2557642520,2174253447,250670636,4111393006,820776640,2714264067,480371740,3827081676,1641815201,471976476,3390931396,2429945441,1137195331,960731193,3090760648,1133787331,990087224,3157246868,2197568643,990106683,3364668336,1887176481,2266001923,2063822963,1618544392,2278060167,2047322231,3829170584,1080954752,248571151,240,8305,40,2534228102,3493739759,1275383348,3274967108,872519746,4176723548,285266433,1840715750,4184922887,17556430,1413747797,307281978,2120228870,3499547505,1022334733,3222343870,859866180,957858860,53774028,3762690258,1022334732,3075268862,95870047,1004913484,3224944527,890189344,3681404409,3193713664,3222468851,400624444,3573706167,3821992304,286495563,3560781096,1840715748,4184919813,223601614,1904107792,809502427,53760,0,1213415752,20,0,4207059486,3140104377,2898504675,4004871538,1279875140,3244,102,811,1279875140,262,16,3220,3737142082,3105,802,2130443,2,19,2435023111,1225050177,959582214,209977746,419956005,1653277726,38082688,1108054594,338825412,1259866168,2288136714,538218568,2777171523,1110579456,2416855268,1355031313,2357285185,2330297313,105263364,6225,8,4292906011,134217727,229114432,4294963332,537133055,4286984301,536870911,11012352,6217,3,1113621011,101207072,0,8329,120,159916594,75850784,2225349395,3810726660,345022852,2358004754,1287947275,146915344,167774528,149291110,9330880,272680489,1368408388,2283831308,2799968354,3962041057,629089313,4237858212,827438818,1346421516,4184919921,2450585107,2966225439,173549584,856428707,272679692,1095581892,2249744902,703076449,336347331,2250269205,2258871937,2276499403,2741927056,4237858186,823244514,2286994188,2986893450,2133728110,4064419522,2826060375,3369214024,1114146937,67701000,1619396689,437176880,516063022,1113059906,1080463578,3306176772,173031708,2382944390,1635811770,2845336068,495044109,3860750786,2707951041,511832092,3390980586,2168983105,480373775,3726541254,3237486145,501340188,3625877954,3236432896,264290319,3658432756,1642077473,511802910,3390964210,1287914753,3960352866,821235216,188771855,4094656741,1358040912,171995662,3842990204,4042526256,130085390,4010721510,3237219856,121659398,2114322558,607126656,3015706929,613418372,145097119,1902312633,2110245248,820249884,2162438338,11572857,3224626322,1603739924,4180676140,3768811447,580537660,1669109834,4183818865,2784608425,3917783593,2723271079,3868879205,951157133,268218369,822870288,740282641,3338444942,4072670434,3555950931,3175696424,1243781705,1612086424,3632825758,336107529,73843728,167790385,0,3228701715,2271245447,2038917174,3228697448,1353584007,248540430,1829654650,813305600,127955463,1829183603,2691763856,119567111,2014220397,3497527200,118548742,1896325238,2423064416,119567118,1913073786,1625884368,127955975,1829191798,1618021984,118520327,3859206262,2691827504,119567111,1980653677,1618609984,114324487,2047312110,2692089616,119567111,1946640506,648240,0,0,1674076160,1048832,0,2428502016,1073873159,0,2147483648,67325729,192,0,127812352,32776,0,1015414784,16781329,0,201326592,2107001,2,0,1079570968,3072,0,3387174912,524416,0,3361734656,268501459,0,1073741824,2070,19,345513522,2420904217,1193675148,574817478,1243127882,144704610,211851018,2822545866,2148632872,42149922,192545833,2165313552,1778391510,499646567,3817372203,954433592,2147541888,2624069696,1,6265,108,2420900634,1142096454,3966700687,604786157,1170671046,3332769446,67520101,3322293317,650587174,1130704006,135285776,742425189,2558624795,114036768,3223859297,270036429,270571650,207684813,68354618,673241538,1497663174,437863704,1007737285,3493168147,3015968806,1176654081,3624470097,2215857168,540465206,3391669578,35324064,16843627,1883321812,2420338179,2147749965,2589481796,2812678427,2762612897,1107928871,168840497,1285571434,2188020240,68376656,270572034,207685329,2730713288,101178996,2173186145,2260833778,212916320,1100871581,811731992,565208444,1661482600,2518353456,884929446,203522824,906842010,1638949636,4224615193,101458136,2944604209,1712338444,3496674944,2896579590,3837962430,652790478,1094042888,34189352,3322965281,741740096,1886919626,283051520,121096711,3627680283,490846256,931402496,3389535944,908618781,1009782478,1724301992,1184032471,927393366,2427062309,2514008545,3181759945,2282311097,779641894,1986814326,39023205,2652442275,408558795,3546061145,3675871515,1126760596,1461905286,660010806,926307895,2431301669,2782444001,2435177877,3515208013,2986954133,779641914,1920233061,1768902777,1400075374,15362,6265,76,478152755,1713168836,2285699348,3280222275,125846156,1929869433,3859575192,283971328,243332110,507644979,3458056642,811998369,1133001989,461603896,3359441868,59522371,2356726845,2064085108,1215891208,124809351,1979936890,544245624,298588551,244378638,1846489313,4041412400,240185358,499388467,3625722334,1640111393,2301650462,998489147,3023651792,2210151427,990086204,1981083852,1752893280,2271753991,923232370,2423293824,123760775,1980180598,2021000696,2273343367,1904674911,2557642520,2174253447,250670636,4111393006,820776640,2714264067,480371740,3827081676,1641815201,471976476,3390931396,2429945441,1137195331,960731193,3090760648,1133787331,990087224,3157246868,2197568643,990106683,3364668336,1887176481,2266001923,2063822963,1618544392,2278060167,2047322231,3829170584,1080954752,248571151,240,8305,40,2534228102,3493739759,1275383348,3274967108,872519746,4176723548,285266433,1840715750,4184922887,17556430,1413747797,307281978,2120228870,3499547505,1022334733,3222343870,859866180,957858860,53774028,3762690258,1022334732,3075268862,95870047,1004913484,3224944527,890189344,3681404409,3193713664,3222468851,400624444,3573706167,3821992304,286495563,3560781096,1840715748,4184919813,223601614,1904107792,809502427,53760,8289,322,742458387,16,47,1342213572,340001361,1268033864,1901991049,1766073492,366563860,248070380,2496217748,3256203601,1661796364,2396927748,4601645,2189959298,134289601,2381193346,136347665,18407390,154308591,3187958534,405005309,545264012,811641352,4125270086,73628201,3917613088,2181056055,1669282848,2548769284,4031133674,2063266002,1579811939,1178055327,136348160,704930687,800235358,294387916,3792183424,1627392223,4033284211,1714954700,105940784,2321677,3545043059,1292923736,2352239827,3157572625,8158,525859,2312134786,2114983938,102958800,1619132424,407014864,115375888,525859,2446352514,2685409410,102958832,1619132423,419600784,1099483652,404783106,4260408856,2179737729,9462327,3322284064,671186987,2082226828,591395,3227541634,42211394,402780201,2365580,293700104,171991563,1627906213,9462320,2248542240,692161580,2283602584,37849281,404234368,2852536499,815794800,151397126,1616937472,2887336656,30943746,605588505,2172782592,3227913041,168864010,2422354021,101195776,2174496134,683845674,1099481496,404783106,109098776,2030743723,102958704,1619132425,440590432,2418737840,411834882,2181562396,1879778881,102978800,1619132423,474140048,3246970172,404783107,125876532,3676610626,2964261622,1114168070,136743936,549611713,3272615564,109069424,1356675010,1131725132,231934257,1356346565,3249664006,3278102540,2261237785,227512372,455024746,910049494,1820066228,3640132464,435160816,268458033,491225860,818626814,205964077,3238264844,1359434272,3839896584,522650211,2365143067,134224920,247824770,3775287466,1088048,809634211,906506394,1820082560,3640165120,435226112,268458033,516391684,2182111574,205916225,3238264844,1460122144,3825773576,522224995,2365130283,134229016,263438722,3443060912,102958112,1619132423,746842416,1647344900,136314992,3762198022,3271040194,2348827240,1757480998,3772260546,3249664262,2204361229,3295134747,272630112,3365019148,101204357,808523809,2198081536,561521800,2408587952,2893840917,1647567896,136315056,4165619462,2205009730,471370768,3246524416,283120260,3264578220,1288160281,1675758817,4043731111,94910513,3625720057,235699330,3057533294,632418404,422416259,369903228,3770745856,410290593,2182111364,197393,1753755713,721558914,559479042,1099614152,184951614,4032856576,205227236,1611010114,471371248,3246524416,284169092,3246965484,404783105,259078476,798502721,823783684,68157496,3861342595,293990433,1090519939,1115199536,409993759,268447793,667452164,2159026754,1647567901,136314976,2421690630,633430084,4180546335,3351867608,3792970110,521208163,2287011595,2084898191,558242924,4044887614,127424177,3361757433,513891527,562127844,2072345375,2248511376,369903228,3770745856,680725385,2232963589,205966157,3238264846,2483782240,2193745416,2216050240,611089540,1092155522,2466432968,1139380112,134946576,71997689,540586484,1616443657,967962642,2724456456,2985431664,1675633952,3352301732,373179272,1580331528,418906888,2080474159,12720012,4061148734,2408620465,2893841933,1044542485,2088927254,4161398828,3352301976,1715372300,1639052300,2204360716,2261237784,2295300144,545259969,3517929496,392243222,805374371,2734956698,3277882121,236658465,3362838777,2421655239,548408719,1096948511,45626942,91253884,182507768,1211177456,50597888,3270392868,904096822,75698566,1076904192,1815012290,1128806083,4731160,604177424,918744636,2184561516,295697,3258986561,3278647011,740514358,16,0,}},{"DXIL_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_var_2",{0,0,0,"","global_var_2","matrix",0,64,112,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_ubo",{0,0,0,"","global_ubo","ConstantBuffer",0,284,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"global_var_0",{0,0,0,"","global_var_0","matrix",0,64,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragNormal",{0,0,1,"LOCATION","fragNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_1",{0,0,0,"","global_var_1","matrix",0,44,64,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_3",{0,0,0,"","global_var_3","matrix",0,64,176,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_4",{0,0,0,"","global_var_4","vector",0,12,240,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_5",{0,0,0,"","global_var_5","vector",0,12,256,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_6",{0,0,0,"","global_var_6","vector",0,12,272,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragPos",{0,0,0,"LOCATION","fragPos","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_7.texture",{0,0,0,"","texture","_Texture",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(5)}},{"global_var_7.sampler",{0,0,0,"","sampler","SamplerState",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(6)}},{"input.var_0.fragTexCoord",{0,0,2,"LOCATION","fragTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragColor",{0,0,3,"LOCATION","fragColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"DXBC_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", std::vector<uint32_t>{1128421444,3957748084,830336524,2239568463,2416555468,1,3104,5,52,876,1000,1052,2948,1178944594,816,1,332,3,60,4294903041,1051904,776,625218323,60,24,40,40,36,12,0,180,3,0,0,0,0,1,0,0,0,237,2,5,4,4294967295,0,1,12,0,0,294,0,0,0,0,0,1,1,0,0,1651469415,1885301857,1835102817,1919251557,1869374047,1734306659,1633841004,1651859308,1818713967,1818321519,1918989919,1935619935,1819307361,811561573,1869375232,1600938338,1634886000,1702126957,1818386290,1600873327,1651469415,1969187937,1734307682,1633841004,1635147628,1597464434,1954047348,1600483957,1818689584,1818321519,1918988383,1952804193,1650422373,1801678700,1869375327,1600938338,1601135221,2880110640,294,1,356,288,0,0,294,0,284,2,740,0,4294967295,0,4294967295,0,1651469415,1969187937,1935634274,1668641396,3170164,1651469415,1985965153,811561569,1711288415,1952542572,3438644,196611,262148,0,0,0,0,0,0,431,1651469415,1985965153,828338785,1711288415,1952542572,3373107,196611,196611,0,0,0,0,0,0,491,1651469415,1985965153,845116001,1728065631,1633841004,1635147628,1597202290,1818689584,1818321519,1918989919,811545695,1869374976,3372129,196609,196609,0,0,0,0,0,0,581,1651469415,1985965153,895447649,1728065631,1633841004,1635147628,1597398898,2880110640,416,440,0,476,500,64,536,440,112,551,440,176,566,588,240,624,588,256,639,588,272,5,4325377,458752,656,0,0,0,0,396,1919117645,1718580079,1378361460,1279795241,1394625619,1701077352,1866670194,1818849389,824210021,3223088,1313297225,116,4,8,104,0,0,3,0,1799,104,1,0,3,1,1799,104,2,0,3,2,771,104,3,0,3,3,1799,1094930252,1313818964,2880154368,1313297231,44,1,8,32,0,0,3,0,15,1415534163,1162302017,2880110676,1480935507,1888,81,472,16779370,117440601,3182150,0,0,0,18,0,100663386,3173958,0,0,0,0,117446744,3178054,0,0,0,21845,0,50335842,1052786,0,50335842,1052786,1,50335842,1052722,2,50335842,1052786,3,50331749,1057010,0,33554536,7,184549445,1048818,0,1052742,2,2129478,0,0,2121728,0,0,117440561,1048706,0,16385,1008981770,1048634,0,50331679,1048634,0,83886134,1048690,0,1053254,3,16777237,167772160,1048690,1,1049158,0,16386,3173242634,3173242634,3173242634,0,251658290,1048690,1,1049158,1,16386,1056964608,1056964608,1056964608,0,16386,1025758986,1025758986,1025758986,0,117440528,1048706,0,1053254,1,1053254,1,83886148,1048706,0,1048634,0,117440568,1048690,2,1052662,0,1053254,1,167772160,1048690,3,2148536902,65,0,3179078,0,0,15,117440528,1048706,0,1049158,3,1049158,3,83886148,1048706,0,1048634,0,117440568,1048690,4,1052662,0,1049158,3,117440528,1048706,1,1049158,2,1049158,4,117440564,1048706,1,1048634,1,16385,0,167772160,1048690,5,2148536902,65,0,3179078,0,0,17,117440528,1048706,2,1049158,5,1049158,5,83886148,1048706,2,1048634,2,117440568,1048690,5,1052662,2,1049158,5,117440528,1048706,2,1049158,2,1049158,5,117440564,1048706,2,1048634,2,16385,0,184549376,1048690,6,2148532806,65,1,16386,1065353216,1065353216,1065353216,0,150994994,1048690,3,1049158,3,1052662,0,1049158,5,117440528,1048706,0,1049158,3,1049158,3,83886148,1048706,0,1048634,0,117440568,1048690,3,1052662,0,1049158,3,117448720,1048706,0,1049158,3,1049158,4,134217728,1048706,0,2148532282,65,0,16385,1065353216,117440568,1048706,3,1048634,0,1048634,0,117440568,1048706,3,1048634,3,1048634,3,117440568,1048706,0,1048634,0,1048634,3,150994994,1048690,1,1049158,6,1052662,0,1049158,1,184549376,1048690,4,2148532806,65,1,16386,1065353216,1065353216,1065353216,0,117440568,1048690,4,1049158,0,1049158,4,117440528,1048706,0,1049158,2,1049158,3,117440564,1048706,0,1048634,0,16385,0,117440568,1048706,0,1048634,0,1048634,0,150994994,1048706,0,1048634,0,16385,3211788288,16385,1065353216,117440568,1048706,0,1048634,0,1048634,0,117440568,1048706,0,1048634,0,16385,1078530012,117440526,1048706,0,16385,1031798784,1048634,0,150994994,1048594,2,1048634,2,16385,1060634624,16385,1049624576,117440526,1048594,2,1048634,2,1048586,2,150994994,1048610,2,1048634,1,16385,1060634624,16385,1049624576,117440526,1048610,2,1048634,1,1048602,2,117440568,1048594,2,1048602,2,1048586,2,117440568,1048706,0,1048634,0,1048586,2,117440568,1048690,1,1049158,1,1052662,0,117440568,1048706,0,1048634,1,1048634,2,150994994,1048706,0,1048634,0,16385,1082130432,16385,953267991,117440526,1048690,1,1049158,1,1052662,0,201326642,1048690,1,1049158,4,16386,1050868098,1050868098,1050868098,0,1049158,1,150995000,1048690,1,1049158,1,3179078,0,0,16,117440568,1048690,1,1052662,2,1049158,1,201326642,1056882,0,1049158,0,16386,1022739087,1022739087,1022739087,0,1049158,1,83886134,1056898,0,16385,1065353216,16777278,1413567571,148,58,7,0,5,52,0,0,1,1,0,0,0,0,0,1,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}},{"DXBC_Reflection_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_var_2",{0,0,0,"","global_var_2","matrix",0,64,112,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_ubo",{0,0,0,"","global_ubo","ConstantBuffer",0,288,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(3)}},{"global_var_0",{0,0,0,"","global_var_0","matrix",0,64,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragNormal",{0,0,1,"LOCATION","fragNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_1",{0,0,0,"","global_var_1","matrix",0,48,64,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_3",{0,0,0,"","global_var_3","matrix",0,64,176,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_4",{0,0,0,"","global_var_4","vector",0,12,240,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_5",{0,0,0,"","global_var_5","vector",0,12,256,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_6",{0,0,0,"","global_var_6","vector",0,12,272,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragPos",{0,0,0,"LOCATION","fragPos","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"global_var_7.texture",{0,0,0,"","texture","_Texture",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(5)}},{"global_var_7.sampler",{0,0,0,"","sampler","SamplerState",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(6)}},{"input.var_0.fragTexCoord",{0,0,2,"LOCATION","fragTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragColor",{0,0,3,"LOCATION","fragColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"SpirV_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", std::vector<uint32_t>{119734787,66816,2621440,351,0,131089,5302,131089,1,393227,116,1280527431,1685353262,808793134,0,196622,0,1,917519,4,2,1852399981,0,31,342,18,20,24,26,75,61,48,196624,2,7,196611,11,1,262149,12,1601331574,49,262149,12,1601331574,49,458757,18,1970302569,1635135092,774922098,1734439526,7565136,524293,20,1970302569,1635135092,774922098,1734439526,1836216142,27745,589829,24,1970302569,1635135092,774922098,1734439526,1131963732,1685221231,0,524293,26,1970302569,1635135092,774922098,1734439526,1869377347,114,720901,27,1651469415,1885301857,1600680821,1936617315,1953390964,1920234335,1601463157,878998643,12339,393222,27,0,1651469415,1969187937,28514,524293,31,1651469415,1885301857,1600680821,1936617315,1953390964,0,786437,40,1952533855,1400400242,1634889588,1717527911,1952542572,1597274164,1298952003,1919904353,828666995,12340,327686,40,0,1635017060,0,786437,43,1952533855,1400400242,1634889588,1717527911,1952542572,1597208627,1298952003,1919904353,828666995,12340,327686,43,0,1635017060,0,524293,46,1919379297,1952540517,2037669733,811558256,1685353311,3159089,327686,46,0,1954047348,6648437,327686,46,1,1886216563,7497068,589829,39,1651469415,1969187937,1935634274,1668641396,1953718132,808726884,0,458758,39,0,1651469415,1985965153,811561569,0,458758,39,1,1651469415,1985965153,828338785,0,458758,39,2,1651469415,1985965153,845116001,0,458758,39,3,1651469415,1985965153,861893217,0,458758,39,4,1651469415,1985965153,878670433,0,458758,39,5,1651469415,1985965153,895447649,0,458758,39,6,1651469415,1985965153,912224865,0,458758,39,7,1651469415,1985965153,929002081,0,393221,48,1717990754,1632137829,1701602414,115,393221,61,1954047348,1214607989,1818521185,29541,393221,75,1886216563,1215456620,1818521185,29541,393221,81,1886216563,1231316332,1701273965,0,262149,82,1886216563,6579564,393221,108,1886216563,1231316332,1701273965,0,262149,109,1886216563,6579564,262149,199,1601331574,12849,262149,245,1601331574,14130,589829,342,1920233061,1768902777,1632662638,1601003890,1852399981,1918989870,3289951,262149,2,1852399981,0,262215,18,30,0,262215,20,30,1,262215,24,30,2,262215,26,30,3,196679,27,2,327752,27,0,35,0,262215,41,6,16,327752,40,0,35,0,262215,44,6,16,327752,43,0,35,0,327752,46,0,35,0,327752,46,1,35,8,196679,39,2,327752,39,0,35,0,327752,39,1,35,64,327752,39,2,35,112,327752,39,3,35,176,327752,39,4,35,240,327752,39,5,35,256,327752,39,6,35,272,327752,39,7,35,288,262215,48,33,0,262215,48,34,3,262215,61,33,0,262215,61,34,2,262215,75,33,0,262215,75,34,1,262215,342,30,0,131091,1,196641,3,1,196630,6,32,262167,7,6,4,262167,10,6,3,262176,11,7,10,262176,17,1,10,262167,21,6,2,262176,23,1,21,262165,28,32,0,262167,29,28,2,196638,27,29,262176,30,9,27,262165,32,32,1,262187,32,33,0,262176,34,9,29,262187,32,42,4,262172,41,7,42,196638,40,41,262187,32,45,3,262172,44,10,45,196638,43,44,262174,46,29,29,655390,39,40,43,40,40,10,10,10,46,196637,38,39,262176,47,2,38,262176,49,2,39,262187,32,51,7,262176,52,2,46,262176,54,2,29,589849,59,6,1,2,0,0,1,0,196637,58,59,262176,60,0,58,262176,62,0,59,262187,32,68,1,131098,73,196637,72,73,262176,74,0,72,262176,76,0,73,196635,80,59,131092,85,262187,6,87,1008981770,262187,6,118,1025758986,393260,10,117,118,118,118,262187,6,120,1056964608,393260,10,119,120,120,120,262176,124,2,10,262187,32,131,6,262187,6,140,0,262187,6,159,3211788288,262187,6,161,1065353216,262187,6,163,1078530012,393260,10,217,161,161,161,262187,6,243,1084227584,262187,6,252,1031798784,262187,6,254,1060634624,262187,6,256,1049624576,262187,6,300,1082130432,262187,6,311,953267991,262187,32,317,5,262187,6,335,1022739087,393260,10,334,335,335,335,262176,341,3,7,262203,17,18,1,262203,17,20,1,262203,23,24,1,262203,17,26,1,262203,30,31,9,262203,47,48,2,262203,60,61,0,262203,74,75,0,262203,341,342,3,262187,6,349,1050868098,393260,10,350,349,349,349,327734,1,2,0,3,131320,4,262203,11,12,7,262205,10,16,18,262205,10,19,20,262205,21,22,24,262205,10,25,26,327746,34,35,31,33,262205,29,36,35,327761,28,37,36,0,327745,49,50,48,37,327746,52,53,50,51,327746,54,55,53,33,262205,29,56,55,327761,28,57,56,0,327745,62,63,61,57,327761,28,65,36,0,327745,49,66,48,65,327746,52,67,66,51,327746,54,69,67,68,262205,29,70,69,327761,28,71,70,0,327745,76,77,75,71,262205,73,78,77,262205,59,79,63,327766,80,81,79,78,393303,7,82,81,22,0,327761,6,84,82,3,327866,85,86,84,87,196855,15,0,262394,86,14,13,131320,13,196670,12,25,131321,15,131320,14,327761,28,91,36,0,327745,49,92,48,91,327746,52,93,92,51,327746,54,94,93,33,262205,29,95,94,327761,28,96,95,0,327745,62,97,61,96,327761,28,99,36,0,327745,49,100,48,99,327746,52,101,100,51,327746,54,102,101,68,262205,29,103,102,327761,28,104,103,0,327745,76,105,75,104,262205,73,106,105,262205,59,107,97,327766,80,108,107,106,393303,7,109,108,22,0,524367,10,111,109,109,0,1,2,196670,12,111,131321,15,131320,15,262205,10,114,12,524300,10,115,116,46,117,114,119,393228,10,121,116,69,19,327761,28,122,36,0,327745,49,123,48,122,327746,124,125,123,42,262205,10,126,125,327811,10,127,126,16,393228,10,128,116,69,127,327761,28,129,36,0,327745,49,130,48,129,327746,124,132,130,131,262205,10,133,132,327811,10,134,133,16,393228,10,135,116,69,134,327809,10,136,128,135,393228,10,137,116,69,136,327828,6,138,121,137,458764,6,139,116,40,138,140,327761,28,141,36,0,327745,49,142,48,141,327746,124,143,142,42,262205,10,144,143,327811,10,145,144,16,393228,10,146,116,69,145,327761,28,147,36,0,327745,49,148,48,147,327746,124,149,148,131,262205,10,150,149,327811,10,151,150,16,393228,10,152,116,69,151,327809,10,153,146,152,393228,10,154,116,69,153,327828,6,155,121,154,458764,6,156,116,40,155,140,327813,6,157,139,156,327813,6,158,157,159,327809,6,160,158,161,327813,6,162,163,160,327761,28,164,36,0,327745,49,165,48,164,327746,124,166,165,42,262205,10,167,166,327811,10,168,167,16,393228,10,169,116,69,168,327761,28,170,36,0,327745,49,171,48,170,327746,124,172,171,131,262205,10,173,172,327811,10,174,173,16,393228,10,175,116,69,174,327809,10,176,169,175,393228,10,177,116,69,176,327828,6,178,121,177,458764,6,179,116,40,178,140,327761,28,180,36,0,327745,49,181,48,180,327746,124,182,181,42,262205,10,183,182,327811,10,184,183,16,393228,10,185,116,69,184,327761,28,186,36,0,327745,49,187,48,186,327746,124,188,187,131,262205,10,189,188,327811,10,190,189,16,393228,10,191,116,69,190,327809,10,192,185,191,393228,10,193,116,69,192,327828,6,194,121,193,458764,6,195,116,40,194,140,327813,6,196,179,195,327813,6,197,196,159,327809,6,198,197,161,327813,6,199,162,198,327761,28,200,36,0,327745,49,201,48,200,327746,124,202,201,42,262205,10,203,202,327811,10,204,203,16,393228,10,205,116,69,204,327828,6,206,121,205,458764,6,207,116,40,206,140,327761,28,208,36,0,327745,49,209,48,208,327746,124,210,209,131,262205,10,211,210,327811,10,212,211,16,393228,10,213,116,69,212,327828,6,214,121,213,458764,6,215,116,40,214,140,327811,10,216,217,115,327761,28,218,36,0,327745,49,219,48,218,327746,124,220,219,42,262205,10,221,220,327811,10,222,221,16,393228,10,223,116,69,222,327761,28,224,36,0,327745,49,225,48,224,327746,124,226,225,131,262205,10,227,226,327811,10,228,227,16,393228,10,229,116,69,228,327809,10,230,223,229,393228,10,231,116,69,230,327761,28,232,36,0,327745,49,233,48,232,327746,124,234,233,42,262205,10,235,234,327811,10,236,235,16,393228,10,237,116,69,236,327828,6,238,231,237,524300,6,239,116,43,238,140,161,327811,6,240,161,239,524300,6,241,116,43,240,140,161,458764,6,242,116,26,241,243,327822,10,244,216,242,327809,10,245,115,244,327811,10,246,217,245,327813,10,248,246,114,327813,10,250,248,350,327816,6,251,252,199,327813,6,253,215,254,327809,6,255,253,256,327816,6,257,215,255,327813,6,258,207,254,327809,6,259,258,256,327816,6,260,207,259,327813,6,261,257,260,327813,6,262,251,261,327761,28,263,36,0,327745,49,264,48,263,327746,124,265,264,42,262205,10,266,265,327811,10,267,266,16,393228,10,268,116,69,267,327761,28,269,36,0,327745,49,270,48,269,327746,124,271,270,131,262205,10,272,271,327811,10,273,272,16,393228,10,274,116,69,273,327809,10,275,268,274,393228,10,276,116,69,275,327761,28,277,36,0,327745,49,278,48,277,327746,124,279,278,42,262205,10,280,279,327811,10,281,280,16,393228,10,282,116,69,281,327828,6,283,276,282,524300,6,284,116,43,283,140,161,327811,6,285,161,284,524300,6,286,116,43,285,140,161,458764,6,287,116,26,286,243,327822,10,288,216,287,327809,10,289,115,288,327822,10,290,289,262,327761,28,291,36,0,327745,49,292,48,291,327746,124,293,292,42,262205,10,294,293,327811,10,295,294,16,393228,10,296,116,69,295,327828,6,297,121,296,458764,6,298,116,40,297,140,327813,6,299,300,298,327761,28,301,36,0,327745,49,302,48,301,327746,124,303,302,131,262205,10,304,303,327811,10,305,304,16,393228,10,306,116,69,305,327828,6,307,121,306,458764,6,308,116,40,307,140,327813,6,309,299,308,327809,6,310,309,311,393296,10,312,310,310,310,327816,10,313,290,312,327809,10,314,250,313,327761,28,315,36,0,327745,49,316,48,315,327746,124,318,316,317,262205,10,319,318,327813,10,320,314,319,327761,28,321,36,0,327745,49,322,48,321,327746,124,323,322,131,262205,10,324,323,327811,10,325,324,16,393228,10,326,116,69,325,327828,6,327,121,326,458764,6,328,116,40,327,140,327822,10,329,320,328,327813,10,333,334,114,327809,10,336,333,329,327760,7,337,336,161,196670,342,337,65789,65592,}},{"SpirV_Reflection_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_ubo",{0,0,0,"","global_ubo","vector",0,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragPos",{0,0,0,"LOCATION","fragPos","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragNormal",{0,0,1,"LOCATION","fragNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragTexCoord",{0,0,2,"LOCATION","fragTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragColor",{0,0,3,"LOCATION","fragColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"GLSL_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", R"(#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 561 0
struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    vec4  data_0[4];
};


#line 12399
struct _MatrixStorage_float3x3_ColMajorstd140_0
{
    vec3  data_1[3];
};


#line 22879
struct aggregate_type_0_std140_0
{
    uvec2 texture_0;
    uvec2 sampler_0;
};


#line 22879
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


#line 22879
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
layout(binding = 0, set = 1)
uniform sampler  samplerHandles[];


#line 30
layout(binding = 0, set = 2)
uniform texture2D  textureHandles[];


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

#line 12442 0
layout(location = 0)
out vec4 entryPointParam_main_var_32_0;


#line 12442
layout(location = 0)
in vec3 input_var_0_fragPos_0;


#line 12442
layout(location = 1)
in vec3 input_var_0_fragNormal_0;


#line 12442
layout(location = 2)
in vec2 input_var_0_fragTexCoord_0;


#line 12442
layout(location = 3)
in vec3 input_var_0_fragColor_0;


#line 79 1
struct fragment_output_0
{
    vec4 var_32_0;
};


#line 83
void main()
{

#line 84
    fragment_output_0 output_0;

#line 84
    uvec2 _S1 = global_push_constant.global_ubo_0;

#line 84
    vec3 var_1_0;

    if(((texture(sampler2D(textureHandles[bufferHandles[global_push_constant.global_ubo_0.x].global_var_7_0.texture_0.x],samplerHandles[bufferHandles[global_push_constant.global_ubo_0.x].global_var_7_0.sampler_0.x]), (input_var_0_fragTexCoord_0))).w) > 0.00999999977648258)
    {

#line 86
        var_1_0 = (texture(sampler2D(textureHandles[bufferHandles[_S1.x].global_var_7_0.texture_0.x],samplerHandles[bufferHandles[_S1.x].global_var_7_0.sampler_0.x]), (input_var_0_fragTexCoord_0))).xyz;

#line 86
    }
    else
    {

#line 86
        var_1_0 = input_var_0_fragColor_0;

#line 86
    }

#line 95
    vec3 var_4_0 = mix(vec3(0.03999999910593033, 0.03999999910593033, 0.03999999910593033), var_1_0, vec3(0.5));

#line 103
    vec3 _S2 = normalize(input_var_0_fragNormal_0);
    float var_13_0 = max(dot(_S2, normalize(bufferHandles[_S1.x].global_var_4_0 - input_var_0_fragPos_0)), 0.0);

#line 109
    float var_18_0 = max(dot(_S2, normalize(bufferHandles[_S1.x].global_var_6_0 - input_var_0_fragPos_0)), 0.0);

#line 115
    const vec3 var_24_0 = vec3(1.0, 1.0, 1.0);


    vec3 _S3 = var_24_0 - var_4_0;

#line 124
    output_0.var_32_0 = vec4(vec3(0.02999999932944775, 0.02999999932944775, 0.02999999932944775) * var_1_0 + ((var_24_0 - (var_4_0 + _S3 * pow(clamp(1.0 - clamp(dot(normalize(normalize(bufferHandles[_S1.x].global_var_4_0 - input_var_0_fragPos_0) + normalize(bufferHandles[_S1.x].global_var_6_0 - input_var_0_fragPos_0)), normalize(bufferHandles[_S1.x].global_var_4_0 - input_var_0_fragPos_0)), 0.0, 1.0), 0.0, 1.0), 5.0))) * var_1_0 / 3.14159297943115234 + 0.0625 / (3.14159297943115234 * (max(dot(_S2, normalize(normalize(bufferHandles[_S1.x].global_var_4_0 - input_var_0_fragPos_0) + normalize(bufferHandles[_S1.x].global_var_6_0 - input_var_0_fragPos_0))), 0.0) * max(dot(_S2, normalize(normalize(bufferHandles[_S1.x].global_var_4_0 - input_var_0_fragPos_0) + normalize(bufferHandles[_S1.x].global_var_6_0 - input_var_0_fragPos_0))), 0.0) * -0.9375 + 1.0) * (max(dot(_S2, normalize(normalize(bufferHandles[_S1.x].global_var_4_0 - input_var_0_fragPos_0) + normalize(bufferHandles[_S1.x].global_var_6_0 - input_var_0_fragPos_0))), 0.0) * max(dot(_S2, normalize(normalize(bufferHandles[_S1.x].global_var_4_0 - input_var_0_fragPos_0) + normalize(bufferHandles[_S1.x].global_var_6_0 - input_var_0_fragPos_0))), 0.0) * -0.9375 + 1.0)) * (var_18_0 / (var_18_0 * 0.71875 + 0.28125) * (var_13_0 / (var_13_0 * 0.71875 + 0.28125))) * (var_4_0 + _S3 * pow(clamp(1.0 - clamp(dot(normalize(normalize(bufferHandles[_S1.x].global_var_4_0 - input_var_0_fragPos_0) + normalize(bufferHandles[_S1.x].global_var_6_0 - input_var_0_fragPos_0)), normalize(bufferHandles[_S1.x].global_var_4_0 - input_var_0_fragPos_0)), 0.0, 1.0), 0.0, 1.0), 5.0)) / (4.0 * max(dot(_S2, normalize(bufferHandles[_S1.x].global_var_4_0 - input_var_0_fragPos_0)), 0.0) * max(dot(_S2, normalize(bufferHandles[_S1.x].global_var_6_0 - input_var_0_fragPos_0)), 0.0) + 0.00009999999747379)) * bufferHandles[_S1.x].global_var_5_0 * max(dot(_S2, normalize(bufferHandles[_S1.x].global_var_6_0 - input_var_0_fragPos_0)), 0.0), 1.0);

#line 124
    entryPointParam_main_var_32_0 = output_0.var_32_0;

#line 124
    return;
}

)"},{"GLSL_Reflection_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_ubo",{0,0,0,"","global_ubo","vector",0,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragPos",{0,0,0,"LOCATION","fragPos","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragNormal",{0,0,1,"LOCATION","fragNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragTexCoord",{0,0,2,"LOCATION","fragTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragColor",{0,0,3,"LOCATION","fragColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"Slang_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", R"(struct aggregate_type_0 {
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
struct fragment_input {
	aggregate_type_2 var_0 : LOCATION0;
}
struct fragment_output {
	float4 var_32 : SV_TARGET0;
}
[shader("fragment")]
fragment_output main(fragment_input input) {
	fragment_output output;
	float3 var_1 = float3(0.000000, 0.000000, 0.000000);
	if (((*global_push_constant.global_ubo).global_var_7.texture.Sample((*global_push_constant.global_ubo).global_var_7.sampler,input.var_0.fragTexCoord).w > 0.010000)) {
		var_1 = (*global_push_constant.global_ubo).global_var_7.texture.Sample((*global_push_constant.global_ubo).global_var_7.sampler,input.var_0.fragTexCoord).xyz;
	}
	else {
		var_1 = input.var_0.fragColor;
	}
	float var_2 = 0.500000;
	float var_3 = 0.500000;
	float3 var_4 = float3(0.040000, 0.040000, 0.040000);
	var_4 = lerp(var_4,var_1,var_3);
	float3 var_5 = float3(0.000000, 0.000000, 0.000000);
	float var_6 = 1.000000;
	float var_7 = var_2;
	float var_8 = ((var_7 * var_7) * (var_7 * var_7));
	float var_9 = 1.000000;
	float var_10 = 1.000000;
	float var_11 = 3.141593;
	float var_12 = ((var_11 * (((max(dot(normalize(input.var_0.fragNormal),normalize((normalize(((*global_push_constant.global_ubo).global_var_4 - input.var_0.fragPos)) + normalize(((*global_push_constant.global_ubo).global_var_6 - input.var_0.fragPos))))),0.000000) * max(dot(normalize(input.var_0.fragNormal),normalize((normalize(((*global_push_constant.global_ubo).global_var_4 - input.var_0.fragPos)) + normalize(((*global_push_constant.global_ubo).global_var_6 - input.var_0.fragPos))))),0.000000)) * (((var_7 * var_7) * (var_7 * var_7)) - var_9)) + var_10)) * (((max(dot(normalize(input.var_0.fragNormal),normalize((normalize(((*global_push_constant.global_ubo).global_var_4 - input.var_0.fragPos)) + normalize(((*global_push_constant.global_ubo).global_var_6 - input.var_0.fragPos))))),0.000000) * max(dot(normalize(input.var_0.fragNormal),normalize((normalize(((*global_push_constant.global_ubo).global_var_4 - input.var_0.fragPos)) + normalize(((*global_push_constant.global_ubo).global_var_6 - input.var_0.fragPos))))),0.000000)) * (((var_7 * var_7) * (var_7 * var_7)) - var_9)) + var_10));
	float var_13 = max(dot(normalize(input.var_0.fragNormal),normalize(((*global_push_constant.global_ubo).global_var_4 - input.var_0.fragPos))),0.000000);
	float var_14 = 1.000000;
	float var_15 = 8.000000;
	float var_16 = var_13;
	float var_17 = 1.000000;
	float var_18 = max(dot(normalize(input.var_0.fragNormal),normalize(((*global_push_constant.global_ubo).global_var_6 - input.var_0.fragPos))),0.000000);
	float var_19 = 1.000000;
	float var_20 = 8.000000;
	float var_21 = var_18;
	float var_22 = 1.000000;
	float var_23 = 1.000000;
	float3 var_24 = float3(1.000000, 1.000000, 1.000000);
	float var_25 = 4.000000;
	float var_26 = 0.000100;
	float3 var_27 = (var_4 + ((var_24 - var_4) * pow(clamp((var_23 - clamp(dot(normalize((normalize(((*global_push_constant.global_ubo).global_var_4 - input.var_0.fragPos)) + normalize(((*global_push_constant.global_ubo).global_var_6 - input.var_0.fragPos)))),normalize(((*global_push_constant.global_ubo).global_var_4 - input.var_0.fragPos))),0.000000,1.000000)),0.000000,1.000000),5.000000)));
	float3 var_28 = float3(1.000000, 1.000000, 1.000000);
	float var_29 = 1.000000;
	float var_30 = 3.141593;
	var_5 = (var_5 + ((((((var_28 - var_27) * var_1) / var_30) + ((((var_8 / var_12) * ((var_21 / ((var_18 * (var_22 - (((var_2 + var_19) * (var_2 + var_19)) / var_20))) + (((var_2 + var_19) * (var_2 + var_19)) / var_20))) * (var_16 / ((var_13 * (var_17 - (((var_2 + var_14) * (var_2 + var_14)) / var_15))) + (((var_2 + var_14) * (var_2 + var_14)) / var_15))))) * (var_4 + ((var_24 - var_4) * pow(clamp((var_23 - clamp(dot(normalize((normalize(((*global_push_constant.global_ubo).global_var_4 - input.var_0.fragPos)) + normalize(((*global_push_constant.global_ubo).global_var_6 - input.var_0.fragPos)))),normalize(((*global_push_constant.global_ubo).global_var_4 - input.var_0.fragPos))),0.000000,1.000000)),0.000000,1.000000),5.000000)))) / (((var_25 * max(dot(normalize(input.var_0.fragNormal),normalize(((*global_push_constant.global_ubo).global_var_4 - input.var_0.fragPos))),0.000000)) * max(dot(normalize(input.var_0.fragNormal),normalize(((*global_push_constant.global_ubo).global_var_6 - input.var_0.fragPos))),0.000000)) + var_26))) * ((*global_push_constant.global_ubo).global_var_5 * var_6)) * max(dot(normalize(input.var_0.fragNormal),normalize(((*global_push_constant.global_ubo).global_var_6 - input.var_0.fragPos))),0.000000)));
	float3 var_31 = float3(0.030000, 0.030000, 0.030000);
	output.var_32 = float4(((var_31 * var_1) + var_5),1.000000);
	return output;
}
)"},{"Slang_Reflection_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_ubo",{0,0,0,"","global_ubo","vector",0,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragPos",{0,0,0,"LOCATION","fragPos","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragNormal",{0,0,1,"LOCATION","fragNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragTexCoord",{0,0,2,"LOCATION","fragTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragColor",{0,0,3,"LOCATION","fragColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},{"DXIL_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", std::vector<uint32_t>{1128421444,2255701112,1428715686,3670718870,4062114423,1,5892,7,60,76,232,292,604,2664,2692,810108499,8,100663296,0,826757961,148,4,8,0,136,0,0,3,0,1799,0,0,136,1,0,3,1,1799,0,0,136,2,0,3,2,771,0,0,136,3,0,3,3,1799,0,1094930252,1313818964,0,826757967,52,1,8,0,40,0,64,3,0,15,0,1415534163,1701278305,116,810963792,304,52,0,0,0,0,0,4294967295,0,67109124,1,0,0,0,37,1,24,2,0,0,0,13,0,44,1129270272,1330205761,1330380878,1230258499,1275088463,1413563215,5132105,1094930252,1313818964,1767992576,110,4,0,1,2,3,16,1,0,4390913,515,10,1,4391169,515,19,2,4325889,515,28,3,4391681,515,0,0,272891905,3,7,7,7,0,7,7,7,0,7,7,0,0,1,2,4,0,1413567571,2052,102,513,1279875140,262,16,2028,3737142082,3105,504,2130443,2,19,2435023111,1225050177,959582214,209977746,419956005,1653277726,38082688,1108054594,338825412,1259866168,2288136714,538218568,2777171523,1110579456,2416855268,1355031313,2357285185,2330297313,105263364,6225,8,4292906011,134217727,229114432,4294963332,537133055,4286984301,536870911,11012352,6217,3,1113621011,101207072,0,8329,76,159916594,75850784,2225349395,3810726660,345022852,2358004754,1287947275,146910224,167774528,149291110,9330880,272680489,1368408388,2283831308,2323914850,2680915579,4237330352,2240178192,1194492224,560108560,3869345288,670228904,2669945324,627222747,1840824102,818089028,371319316,2258870402,2276499403,2741927056,2090374538,1091679972,139527184,2584238217,2133727854,4064419522,2304393815,460206025,204522261,1646944580,409044569,3357049352,205572138,51130563,2283300193,205698177,1997549764,335114459,3214020854,1162056210,3460731138,1108558915,2283817032,3804507202,1892710353,284562425,3524444050,570635398,1244168712,3240169649,806369424,1192239244,1360318982,2333861058,4291493907,200397392,1078797134,349371699,3556033105,848242728,2516592406,3819217407,2588318743,2168612480,273049417,10486899,3228701715,2271245447,2038917174,3228697448,1353584007,248540430,1829654650,813305600,127955463,1829183603,2691763856,119567111,2014220397,3497527200,118548742,1896325238,2423064416,119567118,1913073786,1625884368,127955975,1829191798,1618021984,118520327,3859206262,2691827504,119567111,1980653677,1618609984,114324487,2047312110,2692089616,119567111,1946640506,2148131888,0,0,1674076160,1048832,0,2428502016,1073873159,0,2147483648,67325729,64,0,136200960,98312,0,1015414784,50335763,0,201326592,2108537,6,0,1080095256,1024,0,3521392640,524416,0,3361734656,268501443,0,3221225472,33810320,32,0,412033408,1048832,0,373293056,8,21,412622386,2420904217,1193675148,574817478,1176019018,211945984,1242866122,2747926691,1502217164,2164927648,18387594,674398760,3038790288,285278464,27060273,2147906592,2275586514,939757112,238551950,2181302272,6604,6265,153,2420900634,1142096454,3966700687,604786157,1170671046,3332769446,67520101,3322293317,650587174,1130704006,135285776,876642918,2558624795,114167840,3223859393,270036429,2686490754,982521985,185826147,2877487971,452674459,2744873851,2611180299,114233376,3273019588,1130371096,2125008,136708544,1678131302,3302938830,3938375874,1623121604,872679462,161620033,273490178,1082288520,571480912,2861141138,1929514412,550375501,3289396748,68884770,407952198,325948944,14156036,3473676396,461115611,2198053254,2350531081,205555393,2777003080,2053181881,1252133402,546992762,2701449748,3322164952,652255492,1102529800,2332927632,1885734481,110127110,1729632899,2182809264,2486354785,1100749185,2697045019,2887369478,3670050497,2261927968,855845573,1211597624,2886541699,226721843,2439099521,3456942282,1624378944,216597328,3238224086,118517254,3395600499,1277856746,1854713070,326652034,547086980,1090966800,2332918722,1617299281,108029959,1729633731,35325840,3492862013,318774977,2197828868,131170829,210668059,66183416,1634886178,1701340014,3637261427,3874013918,3313764390,2389560610,1082289774,3538302044,3369647822,1174669350,3974966883,2188283499,3896796224,3703501002,920315102,2217779248,42739842,696423465,2754216604,2374324418,634236621,2379000973,1410353774,2977153825,1268486955,312177531,4039658768,3636653148,2800012012,840779268,3023673148,3107107376,850540454,1375807926,1171197062,2918083758,1823239758,1423985262,2708718370,187401163,2742776690,2068554507,580623219,2270191724,784715532,2301553837,2351860332,541748398,4039668998,3906783836,3735089892,3874020562,3497919654,1708067973,2379132396,1852722350,692298,6265,81,478152755,1713168836,2285699348,3280222275,125846156,1929869433,3859575192,283971328,243332110,507644979,3458056642,811998369,1133001989,461603896,3359441868,59522371,2356726845,2064085108,1215891208,124809351,1979936890,544245624,298588551,244378638,1846489313,4041412400,240185358,499388467,3625722334,1640111393,2301650462,998489147,3023651792,2210151427,990086204,1981083852,1752893280,2271753991,923232370,2423293824,123760775,1980180598,2021000696,2273343367,1904674911,2557642520,2174253447,250670636,4111393006,820776640,2714264067,480371740,3827081676,1641815201,471976476,3390931396,2429945441,1137195331,960731193,3090760648,1133787331,990087224,3157246868,2197568643,990106683,3364668336,1887176481,2266001923,2063822963,1618544392,2278060167,2047322231,3829170584,1080954752,248571151,2172858608,3693019592,2716081216,492945948,3827056860,1,8305,51,2534228134,3493739759,1275383348,3274967108,872568898,4176723548,285266433,1840715750,4184922884,17556430,1413747797,307281978,2120228870,2958482289,1022334733,1162875070,61162561,6366476,3072583170,1893534573,199479033,1145831697,1120089096,1824756092,2367697736,1893335905,199479033,1145831697,1120089096,2227409276,2178478084,4089338931,3711140856,383480190,4019653936,14876220,3560757379,1840715748,4184920070,4973518,1602497779,1409922780,2403067331,1145982255,1358222496,3067895699,3854777377,894406459,3321463872,3238009709,215040,0,1213415752,20,0,2968940628,3195873378,696265959,646122917,1279875140,3192,102,798,1279875140,262,16,3168,3737142082,3105,789,2130443,2,19,2435023111,1225050177,959582214,209977746,419956005,1653277726,38082688,1108054594,338825412,1259866168,2288136714,538218568,2777171523,1110579456,2416855268,1355031313,2357285185,2330297313,105263364,6225,8,4292906011,134217727,229114432,4294963332,537133055,4286984301,536870911,11012352,6217,3,1113621011,101207072,0,8329,77,159916594,75850784,2225349395,3810726660,345022852,2358004754,1287947275,146911248,167774528,149291110,9330880,272680489,1368408388,2283831308,2323914850,2680915579,4237330352,2240178192,1194492224,560108560,3869345288,670228904,2669945324,627222747,1840824102,818089028,371319316,2258870402,2276499403,2741927056,2090374538,1091679972,139527184,2584238217,2133727854,4064419522,2304393815,460206025,204522261,1646944580,409044569,3357049352,205572138,51130563,2283300193,205698177,1997549764,335114459,3214020854,1162056210,3460731138,1108558915,2283817032,3804507202,1892710353,284562425,3524444050,570635398,1244168712,3240169649,806369424,1192239244,1360318982,2333861058,4291493907,200397392,1078797134,349371699,3556033105,848242728,2516592406,3819217407,2588318743,2168612480,273049417,815793267,5,3228701715,2271245447,2038917174,3228697448,1353584007,248540430,1829654650,813305600,127955463,1829183603,2691763856,119567111,2014220397,3497527200,118548742,1896325238,2423064416,119567118,1913073786,1625884368,127955975,1829191798,1618021984,118520327,3859206262,2691827504,119567111,1980653677,1618609984,114324487,2047312110,2692089616,119567111,1946640506,648240,0,0,1674076160,1048832,0,2428502016,1073873159,0,2147483648,67325729,64,0,136200960,98312,0,1015414784,50335763,0,201326592,2108537,6,0,1080095256,1024,0,3521392640,524416,0,3361734656,268501443,0,3221225472,33810320,32,0,412033408,1048832,0,373293056,8,17,345513522,2420904217,1193675148,574817478,1243127882,547357794,2317131978,682114208,578818441,688030504,269177348,2198933520,2275586514,939757112,238551950,2181302272,6608,6265,96,2420900634,1142096454,3966700687,604786157,1170671046,3332769446,67520101,3322293317,650587174,1130704006,135285776,876642918,2558624795,114167840,3223859297,270036429,270571650,218170575,68354618,679533058,1126736651,2617797635,1006633997,220267712,876678584,301994049,1852399981,2258540612,2660411522,1158162076,2888319285,1093828877,1107930664,2282769793,1212187240,3683484699,1972709552,698407265,404862807,1698901046,1748393821,273536451,272853384,1837374214,1099215067,2422365541,1545099533,3626086499,1700473910,227566941,460682970,3356233480,1285033152,2236710233,3516243401,1820987469,325603458,551191044,1638951788,1254150225,3558534147,1624815936,453431363,57684870,111182432,2168522459,1153764739,118751677,3973634133,3369229530,2800278730,840253700,851121980,934590905,3506514359,3318680708,2932706350,1615490636,1942168020,587940761,2053870379,2606967587,3362070546,3403963632,3840597734,2800143046,840781572,3032160060,2988751794,3123755561,1413556658,2781809799,776900044,3442338319,1248751245,1808,6265,81,478152755,1713168836,2285699348,3280222275,125846156,1929869433,3859575192,283971328,243332110,507644979,3458056642,811998369,1133001989,461603896,3359441868,59522371,2356726845,2064085108,1215891208,124809351,1979936890,544245624,298588551,244378638,1846489313,4041412400,240185358,499388467,3625722334,1640111393,2301650462,998489147,3023651792,2210151427,990086204,1981083852,1752893280,2271753991,923232370,2423293824,123760775,1980180598,2021000696,2273343367,1904674911,2557642520,2174253447,250670636,4111393006,820776640,2714264067,480371740,3827081676,1641815201,471976476,3390931396,2429945441,1137195331,960731193,3090760648,1133787331,990087224,3157246868,2197568643,990106683,3364668336,1887176481,2266001923,2063822963,1618544392,2278060167,2047322231,3829170584,1080954752,248571151,2172858608,3693019592,2716081216,492945948,3827056860,1,8305,51,2534228134,3493739759,1275383348,3274967108,872568898,4176723548,285266433,1840715750,4184922884,17556430,1413747797,307281978,2120228870,2958482289,1022334733,1162875070,61162561,6366476,3072583170,1893534573,199479033,1145831697,1120089096,1824756092,2367697736,1893335905,199479033,1145831697,1120089096,2227409276,2178478084,4089338931,3711140856,383480190,4019653936,14876220,3560757379,1840715748,4184920070,4973518,1602497779,1409922780,2403067331,1145982255,1358222496,3067895699,3854777377,894406459,3321463872,3238009709,215040,8289,343,742589459,16,48,146855108,1076172032,1251246404,3779630265,1112098581,327239956,557902996,248596117,348734100,1074577644,3646657569,1661800853,2396927748,4601645,2189959298,134289601,2381193346,136347665,1177575594,2015396352,3154404223,405064255,4143452556,811634575,4125270086,73628201,3917613088,2181056055,1669282848,2936876804,541472687,2125037234,1579811939,1178055327,136348160,704930687,800235358,294387916,3792183424,415238367,3955444673,2268566013,483395264,3248757506,108315164,4033218675,4043782224,380732,525859,102785154,3238412451,2294397983,545259969,4037377048,1099482629,404783106,4259360536,3253217345,9462327,101058592,547388460,100692234,591395,3294650498,3263436802,406973480,2365580,562135560,167797003,1644679332,9462320,3322284064,675383340,2350711444,37849281,404234368,2835755188,1082067552,151397126,1616937472,2820211412,1104685506,605588505,2172782592,2959412065,167815178,2422354022,101195776,1100492230,679650346,1099481500,404783106,108050456,2030739626,102958720,1619132425,436397152,2351612588,411834946,2181562396,1728799937,1757589696,411828290,2181562400,1879810881,528611344,1841292,1203864072,1343254027,460323,3912261762,1996757698,1088048,525859,4174405762,2718178562,102959648,1619132423,478339568,40425220,538479649,1099040768,276237843,2351514264,134224920,196592002,593760373,2181041926,1132552288,616632360,3692577157,1104424386,69635096,2957275331,3409428998,1757480976,646709442,1114161424,135449633,293803267,1345323488,503388209,2199192320,1610735633,3351255747,2407544326,520121613,1044438555,2088878134,4177758316,4027003096,3759047089,3223143267,23118951,2953580608,830810232,405832464,2198352131,197393,3805294657,184685889,641614026,1820098964,2365129512,134224920,261357954,3282432169,2348827240,1757480998,2430083266,565309702,1130619405,3295134746,272630112,2290003980,101204357,808523809,50597888,560734125,2408587952,2893840917,1647567896,136314992,3561574406,3281263298,2348827240,1757480998,1356341442,2712793351,1130619406,3295134749,272630112,3902058508,101204357,808523809,50597888,566502701,2408587952,2893840917,3278103576,505093891,3237009656,23118951,2953580608,1099376792,404817416,268447793,640484100,581968340,2182451088,3826822268,11559475,2550532128,415475789,202916232,1885483074,101195776,1134841180,68553584,478196210,283534308,1107871684,2208568894,2182597756,3359660280,23118951,2953580608,906467487,3474264,823906179,68157496,3827813891,294060065,1090520451,2730672176,228645918,2829762776,230161,3235917889,1895964290,2210427184,3761821944,478968897,591274224,2181040902,2241429600,1951550526,105932294,460323,2512412802,3875814981,7365168,2617640992,2420412505,1091479054,235685500,2160133120,142478147,3770966484,202391552,2302194872,205921664,3238264844,2550881088,131607560,404262137,2172782592,288560854,3351822424,910050820,2024927792,1492682040,3871851584,1675760897,2870939969,2408076423,1820083717,1044511766,837949462,4178116698,3351797848,3827581290,522289507,2687502699,556234127,38931231,4094031422,156250417,314595577,137507312,566518760,4177791775,4032835160,3770703025,3246438755,1044514503,2985432118,3046084801,3351793473,2408592898,520183813,2566521395,2361905185,1748385409,2200023536,2081951119,920646,3389047044,133739661,139270334,553963904,68701464,3640200731,2261239920,260456003,3273597284,2408624775,3361309665,2420051143,545200527,1094726431,4028118590,2407580081,1182533133,67113484,2404974785,3742232767,3294727608,272630048,4176842764,2381373195,205918422,3238264850,3213842688,2967408112,549740685,202391553,200865168,3666735327,265016,0,}},{"DXIL_Reflection_Bindless_C__Users_Administrator_CLionProjects_Helicon_Examples_main_cpp_line_409_column_54", ShaderCodeModule::ShaderResources{0,"",{{"global_ubo",{0,0,0,"","global_ubo","vector",0,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragPos",{0,0,0,"LOCATION","fragPos","vector",0,1,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragNormal",{0,0,1,"LOCATION","fragNormal","vector",0,1,1,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragTexCoord",{0,0,2,"LOCATION","fragTexCoord","vector",0,1,2,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},{"input.var_0.fragColor",{0,0,3,"LOCATION","fragColor","vector",0,1,3,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)}},}}},
};