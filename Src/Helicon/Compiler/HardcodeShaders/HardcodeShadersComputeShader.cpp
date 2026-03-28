#include"HardcodeShaders.h"
std::unordered_map<std::string, std::variant<EmbeddedShader::ShaderCodeModule::ShaderResources,std::variant<std::vector<uint32_t>,std::string>>> EmbeddedShader::HardcodeShaders::hardcodeShadersComputeShader = {{"SpirV_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", std::vector<uint32_t>{119734787,66816,2621440,66,0,131089,55,131089,56,131089,1,393227,54,1280527431,1685353262,808793134,0,196622,0,1,458767,5,2,1852399981,0,19,9,393232,2,17,8,8,1,196611,11,1,720901,19,1651469415,1885301857,1835102817,1919251557,1869374047,1731095395,1633841004,1635147628,3170162,262149,21,1886216563,6579564,262149,26,1886216563,6579564,262149,36,1886216563,6579564,262149,40,1886216563,6579564,262149,2,1852399981,0,262215,9,11,28,262215,19,33,0,262215,19,34,0,131091,1,196641,3,1,262165,5,32,0,262167,6,5,3,262176,8,1,6,262167,10,5,2,262165,12,32,1,262167,13,12,2,196630,15,32,589849,16,15,1,2,0,0,2,0,262176,18,0,16,262167,20,15,4,262167,23,15,3,262187,15,30,1075880919,262187,15,32,1022739087,262187,15,44,1075545375,262187,15,46,1058474557,262187,15,50,1041194025,262187,15,56,0,393260,23,55,56,56,56,262187,15,58,1065353216,393260,23,57,58,58,58,262203,8,9,1,262203,18,19,0,393260,23,63,32,32,32,393260,23,64,46,46,46,393260,23,65,50,50,50,327734,1,2,0,3,131320,4,262205,6,7,9,458831,10,11,7,7,0,1,262268,13,14,11,262205,16,17,19,327778,20,21,17,14,524367,23,24,21,21,0,1,2,262205,16,25,19,327778,20,26,25,14,524367,23,28,26,26,0,1,2,327822,23,29,28,30,327809,23,33,63,29,327813,23,34,24,33,262205,16,35,19,327778,20,36,35,14,524367,23,38,36,36,0,1,2,262205,16,39,19,327778,20,40,39,14,524367,23,42,40,40,0,1,2,327822,23,43,42,44,327809,23,47,64,43,327813,23,48,38,47,327809,23,51,65,48,327816,23,52,34,51,524300,23,53,54,43,52,55,57,327760,20,59,53,58,262205,16,60,19,262243,60,11,59,65789,65592,}},{"SpirV_Reflection_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", ShaderCodeModule::ShaderResources{0,"",0,"",{}}},{"GLSL_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", R"(#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 90 0
layout(rgba32f)
layout(binding = 0)
uniform image2D global_parameter_block_global_var_0_0;


#line 10 1
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{



    ivec2 _S1 = ivec2(gl_GlobalInvocationID.xy);

#line 16
    vec4 _S2 = (imageLoad((global_parameter_block_global_var_0_0), (_S1)));

#line 16
    vec3 _S3 = _S2.xyz;

#line 16
    vec4 _S4 = (imageLoad((global_parameter_block_global_var_0_0), (_S1)));

#line 16
    vec3 _S5 = _S3 * (0.02999999932944775 + 2.50999999046325684 * _S4.xyz);

#line 16
    vec4 _S6 = (imageLoad((global_parameter_block_global_var_0_0), (_S1)));

#line 16
    vec3 _S7 = _S6.xyz;

#line 16
    vec4 _S8 = (imageLoad((global_parameter_block_global_var_0_0), (_S1)));

#line 16
    imageStore((global_parameter_block_global_var_0_0), (_S1), vec4(clamp(_S5 / (0.14000000059604645 + _S7 * (0.5899999737739563 + 2.43000006675720215 * _S8.xyz)), vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0)), 1.0));
    return;
}

)"},{"GLSL_Reflection_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", ShaderCodeModule::ShaderResources{0,"",0,"",{{0,0,0,"SV_DISPATCHTHREADID","dispatch_thread_id_input","vector",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)},}}},{"Slang_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", R"(struct parameter_block_struct {
	RWTexture2D<float4> global_var_0;
}
ParameterBlock<parameter_block_struct> global_parameter_block;
struct compute_input {
	uint3 dispatch_thread_id_input : SV_DispatchThreadID;
}
[shader("compute")]
[numthreads(8,8,1)]
void main(compute_input input) {
	float var_0 = 2.510000;
	float var_1 = 0.030000;
	float var_2 = 2.430000;
	float var_3 = 0.590000;
	float var_4 = 0.140000;
	global_parameter_block.global_var_0[input.dispatch_thread_id_input.xy] = float4(clamp(((global_parameter_block.global_var_0[input.dispatch_thread_id_input.xy].xyz * (var_1 + (var_0 * global_parameter_block.global_var_0[input.dispatch_thread_id_input.xy].xyz))) / (var_4 + (global_parameter_block.global_var_0[input.dispatch_thread_id_input.xy].xyz * (var_3 + (var_2 * global_parameter_block.global_var_0[input.dispatch_thread_id_input.xy].xyz))))),float3(0.000000, 0.000000, 0.000000),float3(1.000000, 1.000000, 1.000000)),1.000000);
}
)"},{"Slang_Reflection_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", ShaderCodeModule::ShaderResources{0,"",0,"",{{0,0,0,"SV_DISPATCHTHREADID","dispatch_thread_id_input","vector",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)},}}},{"SpirV_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", std::vector<uint32_t>{119734787,66816,2621440,85,0,131089,55,131089,56,131089,5302,131089,1,393227,73,1280527431,1685353262,808793134,0,196622,0,1,524303,5,2,1852399981,0,13,9,24,393232,2,17,1,1,1,196611,11,1,720901,10,1651469415,1885301857,1600680821,1936617315,1953390964,1920234335,1601463157,878998643,12339,458758,10,0,1651469415,1985965153,811561569,0,524293,13,1651469415,1885301857,1600680821,1936617315,1953390964,0,393221,24,1954047348,1214607989,1818521185,29541,262149,34,1886216563,6579564,262149,41,1886216563,6579564,262149,53,1886216563,6579564,262149,59,1886216563,6579564,262149,2,1852399981,0,262215,9,11,28,196679,10,2,327752,10,0,35,0,262215,24,33,0,262215,24,34,2,131091,1,196641,3,1,262165,5,32,0,262167,6,5,3,262176,8,1,6,262167,11,5,2,196638,10,11,262176,12,9,10,262165,14,32,1,262187,14,15,0,262176,16,9,11,196630,21,32,589849,22,21,1,2,0,0,2,0,196637,20,22,262176,23,0,20,262176,25,0,22,262167,30,14,2,262167,33,21,4,262167,36,21,3,262187,21,45,1075880919,262187,21,47,1022739087,262187,21,63,1075545375,262187,21,65,1058474557,262187,21,69,1041194025,262187,21,75,0,393260,36,74,75,75,75,262187,21,77,1065353216,393260,36,76,77,77,77,262203,8,9,1,262203,12,13,9,262203,23,24,0,393260,36,82,47,47,47,393260,36,83,65,65,65,393260,36,84,69,69,69,327734,1,2,0,3,131320,4,262205,6,7,9,327746,16,17,13,15,262205,11,18,17,327761,5,19,18,0,327745,25,26,24,19,458831,11,27,7,7,0,1,327761,5,28,18,0,327745,25,29,24,28,262268,30,31,27,262205,22,32,29,327778,33,34,32,31,524367,36,37,34,34,0,1,2,327761,5,38,18,0,327745,25,39,24,38,262205,22,40,39,327778,33,41,40,31,524367,36,43,41,41,0,1,2,327822,36,44,43,45,327809,36,48,82,44,327813,36,49,37,48,327761,5,50,18,0,327745,25,51,24,50,262205,22,52,51,327778,33,53,52,31,524367,36,55,53,53,0,1,2,327761,5,56,18,0,327745,25,57,24,56,262205,22,58,57,327778,33,59,58,31,524367,36,61,59,59,0,1,2,327822,36,62,61,63,327809,36,66,83,62,327813,36,67,55,66,327809,36,70,84,67,327816,36,71,49,70,524300,36,72,73,43,71,74,76,327760,33,78,72,77,262205,22,79,26,262243,79,27,78,65789,65592,}},{"SpirV_Reflection_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", ShaderCodeModule::ShaderResources{8,"global_push_constant",0,"",{{0,0,0,"","global_var_0","",0,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)},}}},{"GLSL_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", R"(#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 22879 0
struct global_push_constant_struct_std430_0
{
    uvec2 global_var_0_0;
};


#line 4 1
layout(push_constant)
layout(std430) uniform block_global_push_constant_struct_std430_0
{
    uvec2 global_var_0_0;
}global_push_constant;

#line 4
layout(rgba32f)
layout(binding = 0, set = 2)
uniform image2D  textureHandles[];


#line 37
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{

#line 37
    uvec2 _S1 = global_push_constant.global_var_0_0;

#line 43
    uint _S2 = global_push_constant.global_var_0_0.x;

#line 43
    ivec2 _S3 = ivec2(gl_GlobalInvocationID.xy);

#line 43
    vec4 _S4 = (imageLoad((textureHandles[global_push_constant.global_var_0_0.x]), (_S3)));

#line 43
    vec3 _S5 = _S4.xyz;

#line 43
    vec4 _S6 = (imageLoad((textureHandles[_S1.x]), (_S3)));

#line 43
    vec3 _S7 = _S5 * (0.02999999932944775 + 2.50999999046325684 * _S6.xyz);

#line 43
    vec4 _S8 = (imageLoad((textureHandles[_S1.x]), (_S3)));

#line 43
    vec3 _S9 = _S8.xyz;

#line 43
    vec4 _S10 = (imageLoad((textureHandles[_S1.x]), (_S3)));

#line 43
    imageStore((textureHandles[_S2]), (_S3), vec4(clamp(_S7 / (0.14000000059604645 + _S9 * (0.5899999737739563 + 2.43000006675720215 * _S10.xyz)), vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0)), 1.0));
    return;
}

)"},{"GLSL_Reflection_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", ShaderCodeModule::ShaderResources{0,"",0,"",{{0,0,0,"SV_DISPATCHTHREADID","dispatch_thread_id_input","vector",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)},}}},{"Slang_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", R"(struct global_push_constant_struct {
	RWTexture2D<float4>.Handle global_var_0;
}
[[vk::push_constant]] ConstantBuffer<global_push_constant_struct> global_push_constant;
[vk::binding(0, 0)]
__DynamicResource<__DynamicResourceKind.General> combinedTextureSamplerHandles[];

[vk::binding(0, 1)]
__DynamicResource<__DynamicResourceKind.General> bufferHandles[];

[vk::binding(0, 2)]
__DynamicResource<__DynamicResourceKind.General> textureHandles[];

export T getDescriptorFromHandle<T>(DescriptorHandle<T> handle) where T : IOpaqueDescriptor
{
	__target_switch
	{
		case spirv:
		case glsl:
		if (T.kind == DescriptorKind.CombinedTextureSampler)
			return combinedTextureSamplerHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else if (T.kind == DescriptorKind.Buffer)
			return bufferHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else if (T.kind == DescriptorKind.Texture)
			return textureHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else
			return defaultGetDescriptorFromHandle(handle);
		default:
		return defaultGetDescriptorFromHandle(handle);
	}
}
struct compute_input {
	uint3 dispatch_thread_id_input : SV_DispatchThreadID;
}
[shader("compute")]
[numthreads(1,1,1)]
void main(compute_input input) {
	float var_0 = 2.510000;
	float var_1 = 0.030000;
	float var_2 = 2.430000;
	float var_3 = 0.590000;
	float var_4 = 0.140000;
	global_push_constant.global_var_0[input.dispatch_thread_id_input.xy] = float4(clamp(((global_push_constant.global_var_0[input.dispatch_thread_id_input.xy].xyz * (var_1 + (var_0 * global_push_constant.global_var_0[input.dispatch_thread_id_input.xy].xyz))) / (var_4 + (global_push_constant.global_var_0[input.dispatch_thread_id_input.xy].xyz * (var_3 + (var_2 * global_push_constant.global_var_0[input.dispatch_thread_id_input.xy].xyz))))),float3(0.000000, 0.000000, 0.000000),float3(1.000000, 1.000000, 1.000000)),1.000000);
}
)"},{"Slang_Reflection_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_293_column_36", ShaderCodeModule::ShaderResources{0,"",0,"",{{0,0,0,"SV_DISPATCHTHREADID","dispatch_thread_id_input","vector",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)},}}},{"SpirV_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", std::vector<uint32_t>{119734787,66816,2621440,66,0,131089,55,131089,56,131089,1,393227,54,1280527431,1685353262,808793134,0,196622,0,1,458767,5,2,1852399981,0,19,9,393232,2,17,8,8,1,196611,11,1,720901,19,1651469415,1885301857,1835102817,1919251557,1869374047,1731095395,1633841004,1635147628,3170162,262149,21,1886216563,6579564,262149,26,1886216563,6579564,262149,36,1886216563,6579564,262149,40,1886216563,6579564,262149,2,1852399981,0,262215,9,11,28,262215,19,33,0,262215,19,34,0,131091,1,196641,3,1,262165,5,32,0,262167,6,5,3,262176,8,1,6,262167,10,5,2,262165,12,32,1,262167,13,12,2,196630,15,32,589849,16,15,1,2,0,0,2,0,262176,18,0,16,262167,20,15,4,262167,23,15,3,262187,15,30,1075880919,262187,15,32,1022739087,262187,15,44,1075545375,262187,15,46,1058474557,262187,15,50,1041194025,262187,15,56,0,393260,23,55,56,56,56,262187,15,58,1065353216,393260,23,57,58,58,58,262203,8,9,1,262203,18,19,0,393260,23,63,32,32,32,393260,23,64,46,46,46,393260,23,65,50,50,50,327734,1,2,0,3,131320,4,262205,6,7,9,458831,10,11,7,7,0,1,262268,13,14,11,262205,16,17,19,327778,20,21,17,14,524367,23,24,21,21,0,1,2,262205,16,25,19,327778,20,26,25,14,524367,23,28,26,26,0,1,2,327822,23,29,28,30,327809,23,33,63,29,327813,23,34,24,33,262205,16,35,19,327778,20,36,35,14,524367,23,38,36,36,0,1,2,262205,16,39,19,327778,20,40,39,14,524367,23,42,40,40,0,1,2,327822,23,43,42,44,327809,23,47,64,43,327813,23,48,38,47,327809,23,51,65,48,327816,23,52,34,51,524300,23,53,54,43,52,55,57,327760,20,59,53,58,262205,16,60,19,262243,60,11,59,65789,65592,}},{"SpirV_Reflection_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", ShaderCodeModule::ShaderResources{0,"",0,"",{}}},{"GLSL_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", R"(#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 90 0
layout(rgba32f)
layout(binding = 0)
uniform image2D global_parameter_block_global_var_0_0;


#line 10 1
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{



    ivec2 _S1 = ivec2(gl_GlobalInvocationID.xy);

#line 16
    vec4 _S2 = (imageLoad((global_parameter_block_global_var_0_0), (_S1)));

#line 16
    vec3 _S3 = _S2.xyz;

#line 16
    vec4 _S4 = (imageLoad((global_parameter_block_global_var_0_0), (_S1)));

#line 16
    vec3 _S5 = _S3 * (0.02999999932944775 + 2.50999999046325684 * _S4.xyz);

#line 16
    vec4 _S6 = (imageLoad((global_parameter_block_global_var_0_0), (_S1)));

#line 16
    vec3 _S7 = _S6.xyz;

#line 16
    vec4 _S8 = (imageLoad((global_parameter_block_global_var_0_0), (_S1)));

#line 16
    imageStore((global_parameter_block_global_var_0_0), (_S1), vec4(clamp(_S5 / (0.14000000059604645 + _S7 * (0.5899999737739563 + 2.43000006675720215 * _S8.xyz)), vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0)), 1.0));
    return;
}

)"},{"GLSL_Reflection_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", ShaderCodeModule::ShaderResources{0,"",0,"",{{0,0,0,"SV_DISPATCHTHREADID","dispatch_thread_id_input","vector",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)},}}},{"HLSL_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 90 "core"
RWTexture2D<float4 > global_parameter_block_global_var_0_0 : register(u0);


#line 5 "7502894232044679931.slang"
struct compute_input_0
{
    uint3 dispatch_thread_id_input_0 : SV_DispatchThreadID;
};

[numthreads(8, 8, 1)]
void main(compute_input_0 input_0)
{



    uint2 _S1 = input_0.dispatch_thread_id_input_0.xy;

#line 16
    float4 _S2 = global_parameter_block_global_var_0_0[_S1];

#line 16
    float3 _S3 = _S2.xyz;

#line 16
    float4 _S4 = global_parameter_block_global_var_0_0[_S1];

#line 16
    float3 _S5 = _S3 * (0.02999999932944775f + 2.50999999046325684f * _S4.xyz);

#line 16
    float4 _S6 = global_parameter_block_global_var_0_0[_S1];

#line 16
    float3 _S7 = _S6.xyz;

#line 16
    float4 _S8 = global_parameter_block_global_var_0_0[_S1];

#line 16
    global_parameter_block_global_var_0_0[_S1] = float4(clamp(_S5 / (0.14000000059604645f + _S7 * (0.5899999737739563f + 2.43000006675720215f * _S8.xyz)), float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 1.0f)), 1.0f);
    return;
}

)"},{"HLSL_Reflection_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", ShaderCodeModule::ShaderResources{0,"",0,"",{{0,0,0,"SV_DISPATCHTHREADID","dispatch_thread_id_input","vector",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)},}}},{"Slang_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", R"(struct parameter_block_struct {
	RWTexture2D<float4> global_var_0;
}
ParameterBlock<parameter_block_struct> global_parameter_block;
struct compute_input {
	uint3 dispatch_thread_id_input : SV_DispatchThreadID;
}
[shader("compute")]
[numthreads(8,8,1)]
void main(compute_input input) {
	float var_0 = 2.510000;
	float var_1 = 0.030000;
	float var_2 = 2.430000;
	float var_3 = 0.590000;
	float var_4 = 0.140000;
	global_parameter_block.global_var_0[input.dispatch_thread_id_input.xy] = float4(clamp(((global_parameter_block.global_var_0[input.dispatch_thread_id_input.xy].xyz * (var_1 + (var_0 * global_parameter_block.global_var_0[input.dispatch_thread_id_input.xy].xyz))) / (var_4 + (global_parameter_block.global_var_0[input.dispatch_thread_id_input.xy].xyz * (var_3 + (var_2 * global_parameter_block.global_var_0[input.dispatch_thread_id_input.xy].xyz))))),float3(0.000000, 0.000000, 0.000000),float3(1.000000, 1.000000, 1.000000)),1.000000);
}
)"},{"Slang_Reflection_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", ShaderCodeModule::ShaderResources{0,"",0,"",{{0,0,0,"SV_DISPATCHTHREADID","dispatch_thread_id_input","vector",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)},}}},{"SpirV_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", std::vector<uint32_t>{119734787,66816,2621440,85,0,131089,55,131089,56,131089,5302,131089,1,393227,73,1280527431,1685353262,808793134,0,196622,0,1,524303,5,2,1852399981,0,13,9,24,393232,2,17,1,1,1,196611,11,1,720901,10,1651469415,1885301857,1600680821,1936617315,1953390964,1920234335,1601463157,878998643,12339,458758,10,0,1651469415,1985965153,811561569,0,524293,13,1651469415,1885301857,1600680821,1936617315,1953390964,0,393221,24,1954047348,1214607989,1818521185,29541,262149,34,1886216563,6579564,262149,41,1886216563,6579564,262149,53,1886216563,6579564,262149,59,1886216563,6579564,262149,2,1852399981,0,262215,9,11,28,196679,10,2,327752,10,0,35,0,262215,24,33,0,262215,24,34,2,131091,1,196641,3,1,262165,5,32,0,262167,6,5,3,262176,8,1,6,262167,11,5,2,196638,10,11,262176,12,9,10,262165,14,32,1,262187,14,15,0,262176,16,9,11,196630,21,32,589849,22,21,1,2,0,0,2,0,196637,20,22,262176,23,0,20,262176,25,0,22,262167,30,14,2,262167,33,21,4,262167,36,21,3,262187,21,45,1075880919,262187,21,47,1022739087,262187,21,63,1075545375,262187,21,65,1058474557,262187,21,69,1041194025,262187,21,75,0,393260,36,74,75,75,75,262187,21,77,1065353216,393260,36,76,77,77,77,262203,8,9,1,262203,12,13,9,262203,23,24,0,393260,36,82,47,47,47,393260,36,83,65,65,65,393260,36,84,69,69,69,327734,1,2,0,3,131320,4,262205,6,7,9,327746,16,17,13,15,262205,11,18,17,327761,5,19,18,0,327745,25,26,24,19,458831,11,27,7,7,0,1,327761,5,28,18,0,327745,25,29,24,28,262268,30,31,27,262205,22,32,29,327778,33,34,32,31,524367,36,37,34,34,0,1,2,327761,5,38,18,0,327745,25,39,24,38,262205,22,40,39,327778,33,41,40,31,524367,36,43,41,41,0,1,2,327822,36,44,43,45,327809,36,48,82,44,327813,36,49,37,48,327761,5,50,18,0,327745,25,51,24,50,262205,22,52,51,327778,33,53,52,31,524367,36,55,53,53,0,1,2,327761,5,56,18,0,327745,25,57,24,56,262205,22,58,57,327778,33,59,58,31,524367,36,61,59,59,0,1,2,327822,36,62,61,63,327809,36,66,83,62,327813,36,67,55,66,327809,36,70,84,67,327816,36,71,49,70,524300,36,72,73,43,71,74,76,327760,33,78,72,77,262205,22,79,26,262243,79,27,78,65789,65592,}},{"SpirV_Reflection_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", ShaderCodeModule::ShaderResources{8,"global_push_constant",0,"",{{0,0,0,"","global_var_0","",0,8,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(0)},}}},{"GLSL_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", R"(#version 450
#extension GL_EXT_nonuniform_qualifier : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 22879 0
struct global_push_constant_struct_std430_0
{
    uvec2 global_var_0_0;
};


#line 4 1
layout(push_constant)
layout(std430) uniform block_global_push_constant_struct_std430_0
{
    uvec2 global_var_0_0;
}global_push_constant;

#line 4
layout(rgba32f)
layout(binding = 0, set = 2)
uniform image2D  textureHandles[];


#line 37
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{

#line 37
    uvec2 _S1 = global_push_constant.global_var_0_0;

#line 43
    uint _S2 = global_push_constant.global_var_0_0.x;

#line 43
    ivec2 _S3 = ivec2(gl_GlobalInvocationID.xy);

#line 43
    vec4 _S4 = (imageLoad((textureHandles[global_push_constant.global_var_0_0.x]), (_S3)));

#line 43
    vec3 _S5 = _S4.xyz;

#line 43
    vec4 _S6 = (imageLoad((textureHandles[_S1.x]), (_S3)));

#line 43
    vec3 _S7 = _S5 * (0.02999999932944775 + 2.50999999046325684 * _S6.xyz);

#line 43
    vec4 _S8 = (imageLoad((textureHandles[_S1.x]), (_S3)));

#line 43
    vec3 _S9 = _S8.xyz;

#line 43
    vec4 _S10 = (imageLoad((textureHandles[_S1.x]), (_S3)));

#line 43
    imageStore((textureHandles[_S2]), (_S3), vec4(clamp(_S7 / (0.14000000059604645 + _S9 * (0.5899999737739563 + 2.43000006675720215 * _S10.xyz)), vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0)), 1.0));
    return;
}

)"},{"GLSL_Reflection_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", ShaderCodeModule::ShaderResources{0,"",0,"",{{0,0,0,"SV_DISPATCHTHREADID","dispatch_thread_id_input","vector",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)},}}},{"HLSL_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", R"(#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif

#ifndef __DXC_VERSION_MAJOR
// warning X3557: loop doesn't seem to do anything, forcing loop to unroll
#pragma warning(disable : 3557)
#endif


#line 1 "6534597699277138984.slang"
struct global_push_constant_struct_0
{
    uint2 global_var_0_0;
};


#line 4
cbuffer global_push_constant : register(b0)
{
    global_push_constant_struct_0 global_push_constant;
}

#line 32
struct compute_input_0
{
    uint3 dispatch_thread_id_input_0 : SV_DispatchThreadID;
};

[numthreads(1, 1, 1)]
void main(compute_input_0 input_0)
{

#line 37
    compute_input_0 _S1 = input_0;

#line 43
    RWTexture2D<float4 > _S2 = RWTexture2D<float4 >(ResourceDescriptorHeap[global_push_constant.global_var_0_0.x]);

#line 43
    uint2 _S3 = _S1.dispatch_thread_id_input_0.xy;

#line 43
    RWTexture2D<float4 > _S4 = RWTexture2D<float4 >(ResourceDescriptorHeap[global_push_constant.global_var_0_0.x]);

#line 43
    float4 _S5 = _S4[_S3];

#line 43
    float3 _S6 = _S5.xyz;

#line 43
    RWTexture2D<float4 > _S7 = RWTexture2D<float4 >(ResourceDescriptorHeap[global_push_constant.global_var_0_0.x]);

#line 43
    float4 _S8 = _S7[_S3];

#line 43
    float3 _S9 = _S6 * (0.02999999932944775f + 2.50999999046325684f * _S8.xyz);

#line 43
    RWTexture2D<float4 > _S10 = RWTexture2D<float4 >(ResourceDescriptorHeap[global_push_constant.global_var_0_0.x]);

#line 43
    float4 _S11 = _S10[_S3];

#line 43
    float3 _S12 = _S11.xyz;

#line 43
    RWTexture2D<float4 > _S13 = RWTexture2D<float4 >(ResourceDescriptorHeap[global_push_constant.global_var_0_0.x]);

#line 43
    float4 _S14 = _S13[_S3];

#line 43
    _S2[_S3] = float4(clamp(_S9 / (0.14000000059604645f + _S12 * (0.5899999737739563f + 2.43000006675720215f * _S14.xyz)), float3(0.0f, 0.0f, 0.0f), float3(1.0f, 1.0f, 1.0f)), 1.0f);
    return;
}

)"},{"HLSL_Reflection_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", ShaderCodeModule::ShaderResources{0,"",0,"",{{0,0,0,"SV_DISPATCHTHREADID","dispatch_thread_id_input","vector",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)},}}},{"Slang_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", R"(struct global_push_constant_struct {
	RWTexture2D<float4>.Handle global_var_0;
}
[[vk::push_constant]] ConstantBuffer<global_push_constant_struct> global_push_constant;
[vk::binding(0, 0)]
__DynamicResource<__DynamicResourceKind.General> combinedTextureSamplerHandles[];

[vk::binding(0, 1)]
__DynamicResource<__DynamicResourceKind.General> bufferHandles[];

[vk::binding(0, 2)]
__DynamicResource<__DynamicResourceKind.General> textureHandles[];

export T getDescriptorFromHandle<T>(DescriptorHandle<T> handle) where T : IOpaqueDescriptor
{
	__target_switch
	{
		case spirv:
		case glsl:
		if (T.kind == DescriptorKind.CombinedTextureSampler)
			return combinedTextureSamplerHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else if (T.kind == DescriptorKind.Buffer)
			return bufferHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else if (T.kind == DescriptorKind.Texture)
			return textureHandles[((uint2)handle).x].asOpaqueDescriptor<T>();
		else
			return defaultGetDescriptorFromHandle(handle);
		default:
		return defaultGetDescriptorFromHandle(handle);
	}
}
struct compute_input {
	uint3 dispatch_thread_id_input : SV_DispatchThreadID;
}
[shader("compute")]
[numthreads(1,1,1)]
void main(compute_input input) {
	float var_0 = 2.510000;
	float var_1 = 0.030000;
	float var_2 = 2.430000;
	float var_3 = 0.590000;
	float var_4 = 0.140000;
	global_push_constant.global_var_0[input.dispatch_thread_id_input.xy] = float4(clamp(((global_push_constant.global_var_0[input.dispatch_thread_id_input.xy].xyz * (var_1 + (var_0 * global_push_constant.global_var_0[input.dispatch_thread_id_input.xy].xyz))) / (var_4 + (global_push_constant.global_var_0[input.dispatch_thread_id_input.xy].xyz * (var_3 + (var_2 * global_push_constant.global_var_0[input.dispatch_thread_id_input.xy].xyz))))),float3(0.000000, 0.000000, 0.000000),float3(1.000000, 1.000000, 1.000000)),1.000000);
}
)"},{"Slang_Reflection_Bindless_C__Users_Lee_Documents_Github_CabbageHardware_Examples_main_cpp_line_300_column_29", ShaderCodeModule::ShaderResources{0,"",0,"",{{0,0,0,"SV_DISPATCHTHREADID","dispatch_thread_id_input","vector",0,0,0,static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(-1)},}}},
};