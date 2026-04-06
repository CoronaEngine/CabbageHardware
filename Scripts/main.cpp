#include <filesystem>
#include <iostream>
#include <Helicon.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <set>
using namespace EmbeddedShader;

void generateBinary(std::stringstream& out, std::string_view name, const std::vector<uint32_t>& shaderCode)
{
	out << "static inline std::vector<uint32_t> " << name <<" {";
	for (uint32_t code : shaderCode)
	{
		out << code << ",";
	}
	out << "};\n";
}
#include <string>
#include <string_view>

std::string typeNameToSlang(std::string_view n)
{
    /* ---------- 向量 ---------- */
    if (n == "ivec2") return "int2";
    if (n == "ivec3") return "int3";
    if (n == "ivec4") return "int4";
    if (n == "uvec2") return "uint2";
    if (n == "uvec3") return "uint3";
    if (n == "uvec4") return "uint4";
    if (n == "vec2")  return "float2";
    if (n == "vec3")  return "float3";
    if (n == "vec4")  return "float4";
    if (n == "bvec2") return "bool2";
    if (n == "bvec3") return "bool3";
    if (n == "bvec4") return "bool4";
    if (n == "dvec2") return "double2";
    if (n == "dvec3") return "double3";
    if (n == "dvec4") return "double4";

    /* ---------- 矩阵 ---------- */
    if (n == "mat2")   return "float2x2";
    if (n == "mat3")   return "float3x3";
    if (n == "mat4")   return "float4x4";
    if (n == "mat2x2") return "float2x2";
    if (n == "mat2x3") return "float2x3";
    if (n == "mat2x4") return "float2x4";
    if (n == "mat3x2") return "float3x2";
    if (n == "mat3x3") return "float3x3";
    if (n == "mat3x4") return "float3x4";
    if (n == "mat4x2") return "float4x2";
    if (n == "mat4x3") return "float4x3";
    if (n == "mat4x4") return "float4x4";
    if (n == "dmat2")   return "double2x2";
    if (n == "dmat3")   return "double3x3";
    if (n == "dmat4")   return "double4x4";
    if (n == "dmat2x2") return "double2x2";
    if (n == "dmat2x3") return "double2x3";
    if (n == "dmat2x4") return "double2x4";
    if (n == "dmat3x2") return "double3x2";
    if (n == "dmat3x3") return "double3x3";
    if (n == "dmat3x4") return "double3x4";
    if (n == "dmat4x2") return "double4x2";
    if (n == "dmat4x3") return "double4x3";
    if (n == "dmat4x4") return "double4x4";

    /* ---------- 纹理（GLSL → Slang） ---------- */
    if (n == "sampler1D")         return "Texture1D";
    if (n == "sampler2D")         return "Texture2D";
    if (n == "sampler3D")         return "Texture3D";
    if (n == "samplerCube")       return "TextureCube";
    if (n == "sampler2DArray")    return "Texture2DArray";
    if (n == "samplerCubeArray")  return "TextureCubeArray";
    if (n == "sampler2DMS")       return "Texture2DMS";
    if (n == "sampler2DShadow")   return "Texture2DShadow";
    if (n == "samplerCubeShadow") return "TextureCubeShadow";
    // int texel sampler types
    if (n == "isampler1D")        return "Texture1D<int4>";
    if (n == "isampler2D")        return "Texture2D<int4>";
    if (n == "isampler3D")        return "Texture3D<int4>";
    if (n == "isamplerCube")      return "TextureCube<int4>";
    if (n == "isampler2DArray")   return "Texture2DArray<int4>";
    // uint texel sampler types
    if (n == "usampler1D")        return "Texture1D<uint4>";
    if (n == "usampler2D")        return "Texture2D<uint4>";
    if (n == "usampler3D")        return "Texture3D<uint4>";
    if (n == "usamplerCube")      return "TextureCube<uint4>";
    if (n == "usampler2DArray")   return "Texture2DArray<uint4>";

    /* ---------- 缓冲（GLSL 风格 → Slang） ---------- */
    // GLSL 名字 → HLSL/Slang 名字
    if (n == "samplerBuffer")       return "Buffer";          // 只读 buffer
    if (n == "imageBuffer")         return "RWBuffer";        // 读写 buffer
    if (n == "sampler2DRect")       return "Texture2D";       // 无 normalized coord
    if (n == "image1D")             return "RWTexture1D";
    if (n == "image2D")             return "RWTexture2D";
    if (n == "image3D")             return "RWTexture3D";
    if (n == "imageCube")           return "RWTextureCube";
    if (n == "image2DArray")        return "RWTexture2DArray";
    if (n == "imageCubeArray")      return "RWTextureCubeArray";
    if (n == "uimage2D")            return "RWTexture2D<uint>";
    if (n == "uimage3D")            return "RWTexture3D<uint>";
    if (n == "uimageCube")          return "RWTextureCube<uint>";
    if (n == "uimage2DArray")       return "RWTexture2DArray<uint>";
    if (n == "iimage2D")            return "RWTexture2D<int>";
    if (n == "iimage3D")            return "RWTexture3D<int>";
    if (n == "iimageCube")          return "RWTextureCube<int>";
    if (n == "iimage2DArray")       return "RWTexture2DArray<int>";

    /* ---------- HLSL 风格缓冲（已同名，列出仅为扩展） ---------- */
    // StructuredBuffer / RWStructuredBuffer / ByteAddressBuffer / ... 在 Slang 里拼写不变
    // 若有新类型需要改名，继续往这里加即可

    /* 未命中则原样返回 */
    return std::string(n);
}

std::string typeNameToCpp(std::string_view typeName)
    {
    // GLSL 向量类型
    if (typeName == "vec2") return "::ktm::fvec2";
    if (typeName == "vec3") return "::ktm::fvec3";
    if (typeName == "vec4") return "::ktm::fvec4";

    if (typeName == "dvec2") return "::ktm::dvec2";
    if (typeName == "dvec3") return "::ktm::dvec3";
    if (typeName == "dvec4") return "::ktm::dvec4";

    if (typeName == "bvec2") return "::ktm::bvec2";
    if (typeName == "bvec3") return "::ktm::bvec3";
    if (typeName == "bvec4") return "::ktm::bvec4";

    if (typeName == "uvec2") return "::ktm::uvec2";
    if (typeName == "uvec3") return "::ktm::uvec3";
    if (typeName == "uvec4") return "::ktm::uvec4";

    if (typeName == "ivec2") return "::ktm::svec2";
    if (typeName == "ivec3") return "::ktm::svec3";
    if (typeName == "ivec4") return "::ktm::svec4";

    // GLSL 矩阵类型
    if (typeName == "mat2") return "::ktm::fmat2x2";
    if (typeName == "mat3") return "::ktm::fmat3x3";
    if (typeName == "mat4") return "::ktm::fmat4x4";

    if (typeName == "mat2x2") return "::ktm::fmat2x2";
    if (typeName == "mat2x3") return "::ktm::fmat2x3";
    if (typeName == "mat2x4") return "::ktm::fmat2x4";
    if (typeName == "mat3x2") return "::ktm::fmat3x2";
    if (typeName == "mat3x3") return "::ktm::fmat3x3";
    if (typeName == "mat3x4") return "::ktm::fmat3x4";
    if (typeName == "mat4x2") return "::ktm::fmat4x2";
    if (typeName == "mat4x3") return "::ktm::fmat4x3";
    if (typeName == "mat4x4") return "::ktm::fmat4x4";

    if (typeName == "dmat2") return "::ktm::dmat2x2";
    if (typeName == "dmat3") return "::ktm::dmat3x3";
    if (typeName == "dmat4") return "::ktm::dmat4x4";

    if (typeName == "dmat2x2") return "::ktm::dmat2x2";
    if (typeName == "dmat2x3") return "::ktm::dmat2x3";
    if (typeName == "dmat2x4") return "::ktm::dmat2x4";
    if (typeName == "dmat3x2") return "::ktm::dmat3x2";
    if (typeName == "dmat3x3") return "::ktm::dmat3x3";
    if (typeName == "dmat3x4") return "::ktm::dmat3x4";
    if (typeName == "dmat4x2") return "::ktm::dmat4x2";
    if (typeName == "dmat4x3") return "::ktm::dmat4x3";
    if (typeName == "dmat4x4") return "::ktm::dmat4x4";

    // HLSL 向量类型
    if (typeName == "float2") return "::ktm::fvec2";
    if (typeName == "float3") return "::ktm::fvec3";
    if (typeName == "float4") return "::ktm::fvec4";

    if (typeName == "int2") return "::ktm::svec2";
    if (typeName == "int3") return "::ktm::svec3";
    if (typeName == "int4") return "::ktm::svec4";

    if (typeName == "uint2") return "::ktm::uvec2";
    if (typeName == "uint3") return "::ktm::uvec3";
    if (typeName == "uint4") return "::ktm::uvec4";

    if (typeName == "double2") return "::ktm::dvec2";
    if (typeName == "double3") return "::ktm::dvec3";
    if (typeName == "double4") return "::ktm::dvec4";

    if (typeName == "half2") return "::ktm::fvec2";
    if (typeName == "half3") return "::ktm::fvec3";
    if (typeName == "half4") return "::ktm::fvec4";

    // HLSL 矩阵类型
    if (typeName == "float2x2") return "::ktm::fmat2x2";
    if (typeName == "float2x3") return "::ktm::fmat2x3";
    if (typeName == "float2x4") return "::ktm::fmat2x4";
    if (typeName == "float3x2") return "::ktm::fmat3x2";
    if (typeName == "float3x3") return "::ktm::fmat3x3";
    if (typeName == "float3x4") return "::ktm::fmat3x4";
    if (typeName == "float4x2") return "::ktm::fmat4x2";
    if (typeName == "float4x3") return "::ktm::fmat4x3";
    if (typeName == "float4x4") return "::ktm::fmat4x4";

    if (typeName == "half2x2") return "::ktm::fmat2x2";
    if (typeName == "half2x3") return "::ktm::fmat2x3";
    if (typeName == "half2x4") return "::ktm::fmat2x4";
    if (typeName == "half3x2") return "::ktm::fmat3x2";
    if (typeName == "half3x3") return "::ktm::fmat3x3";
    if (typeName == "half3x4") return "::ktm::fmat3x4";
    if (typeName == "half4x2") return "::ktm::fmat4x2";
    if (typeName == "half4x3") return "::ktm::fmat4x3";
    if (typeName == "half4x4") return "::ktm::fmat4x4";

    if (typeName == "double2x2") return "::ktm::dmat2x2";
    if (typeName == "double2x3") return "::ktm::dmat2x3";
    if (typeName == "double2x4") return "::ktm::dmat2x4";
    if (typeName == "double3x2") return "::ktm::dmat3x2";
    if (typeName == "double3x3") return "::ktm::dmat3x3";
    if (typeName == "double3x4") return "::ktm::dmat3x4";
    if (typeName == "double4x2") return "::ktm::dmat4x2";
    if (typeName == "double4x3") return "::ktm::dmat4x3";
    if (typeName == "double4x4") return "::ktm::dmat4x4";

    //特定类型名称
    if (typeName == "uint") return "unsigned int";
    if (typeName == "half") return "float";      // half 降级为 float
    if (typeName == "dword") return "unsigned int";

    // Opaque sampler/image types -> Texture2DProxy (EDSL proxy for GLSL sampler/image)
    // float texel types
    if (typeName == "sampler1D")        return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "sampler2D")        return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "sampler3D")        return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "samplerCube")      return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "sampler2DArray")   return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "samplerCubeArray") return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "sampler2DMS")      return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "sampler2DShadow")  return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "samplerCubeShadow") return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "sampler2DRect")    return "@Texture2DProxy<::ktm::fvec4>";
    // int texel types
    if (typeName == "isampler1D")       return "@Texture2DProxy<::ktm::svec4>";
    if (typeName == "isampler2D")       return "@Texture2DProxy<::ktm::svec4>";
    if (typeName == "isampler3D")       return "@Texture2DProxy<::ktm::svec4>";
    if (typeName == "isamplerCube")     return "@Texture2DProxy<::ktm::svec4>";
    if (typeName == "isampler2DArray")  return "@Texture2DProxy<::ktm::svec4>";
    // uint texel types
    if (typeName == "usampler1D")       return "@Texture2DProxy<::ktm::uvec4>";
    if (typeName == "usampler2D")       return "@Texture2DProxy<::ktm::uvec4>";
    if (typeName == "usampler3D")       return "@Texture2DProxy<::ktm::uvec4>";
    if (typeName == "usamplerCube")     return "@Texture2DProxy<::ktm::uvec4>";
    if (typeName == "usampler2DArray")  return "@Texture2DProxy<::ktm::uvec4>";
    // image types (storage images)
    if (typeName == "image1D")          return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "image2D")          return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "image3D")          return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "imageCube")        return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "image2DArray")     return "@Texture2DProxy<::ktm::fvec4>";
    if (typeName == "iimage2D")         return "@Texture2DProxy<::ktm::svec4>";
    if (typeName == "iimage3D")         return "@Texture2DProxy<::ktm::svec4>";
    if (typeName == "iimageCube")       return "@Texture2DProxy<::ktm::svec4>";
    if (typeName == "iimage2DArray")    return "@Texture2DProxy<::ktm::svec4>";
    if (typeName == "uimage2D")         return "@Texture2DProxy<::ktm::uvec4>";
    if (typeName == "uimage3D")         return "@Texture2DProxy<::ktm::uvec4>";
    if (typeName == "uimageCube")       return "@Texture2DProxy<::ktm::uvec4>";
    if (typeName == "uimage2DArray")    return "@Texture2DProxy<::ktm::uvec4>";

	return std::string(typeName);
}

// Helper: check if typeNameToCpp returned an opaque proxy type (prefixed with '@')
bool isOpaqueProxyType(const std::string& cppType)
{
	return !cppType.empty() && cppType[0] == '@';
}

// Helper: emit the C++ type for a parameter or return type.
// Opaque types (prefixed '@') are emitted directly as ::EmbeddedShader::XXX,
// normal types are wrapped in ::EmbeddedShader::VariateProxy<XXX>.
std::string emitCppParamType(const std::string& typeName)
{
	auto cppType = typeNameToCpp(typeName);
	if (isOpaqueProxyType(cppType))
		return "::EmbeddedShader::" + cppType.substr(1);  // strip '@' prefix
	return "::EmbeddedShader::VariateProxy<" + cppType + ">";
}

void buildFunctionParameter(FunctionSignature& signature, std::stringstream& out)
{
	out << "(";
	if (!signature.parameters.empty())
	{
		auto& param0 = signature.parameters[0];
		out << emitCppParamType(param0.typeName) << " " << param0.name;
		for (size_t i = 1; i < signature.parameters.size(); ++i)
		{
			auto& param = signature.parameters[i];
			out << ", " << emitCppParamType(param.typeName) << " " << param.name;
		}
	}
	out << ")";
}

void buildFunctionSignature(FunctionSignature &signature, std::stringstream &out, std::string_view sourceSpv)
{
	out << "static inline ::EmbeddedShader::FunctionProxy<";
	if (signature.returnTypeName == "void")
	{
		out << "void";
		buildFunctionParameter(signature,out);
	}
	else
	{
		auto retCppType = typeNameToCpp(signature.returnTypeName);
		if (isOpaqueProxyType(retCppType))
		{
			// Opaque type: emit as full proxy type directly (e.g. Texture2DProxy<fvec4>)
			out << "::EmbeddedShader::" << retCppType.substr(1);
		}
		else
		{
			out << "::EmbeddedShader::VariateProxy<" << retCppType;
		}
		buildFunctionParameter(signature,out);
		if (!isOpaqueProxyType(retCppType))
			out << ">";
	}
	out << "> " << signature.name << "{";
	out << "\""<< signature.name << "\",\""<< typeNameToSlang(signature.returnTypeName) << "\",{";
	for (auto element: signature.parameters)
	{
		out << "\"" << typeNameToSlang(element.typeName) << "\",";
	}
	out<< "},&" << sourceSpv << "};";
}

void buildStruct(const StructInfo& structInfo, std::stringstream& out)
{
	out << "struct " << structInfo.name << "\n{\n";
	for (auto& member : structInfo.members)
	{
		out << "\t" << emitCppParamType(member.typeName) << " " << member.name << ";\n";
	}
	out << "};";
}

std::string sanitizeToIdentifier(const std::filesystem::path& p)
{
	auto s = p.filename().string();
	for (auto& c : s)
	{
		if (!std::isalnum(static_cast<unsigned char>(c)))
			c = '_';
	}
	if (!s.empty() && std::isdigit(static_cast<unsigned char>(s[0])))
		s = "_" + s;
	return s;
}

std::string extractMemberName(const std::string& qualifiedKey)
{
	auto pos = qualifiedKey.rfind('.');
	if (pos != std::string::npos)
		return qualifiedKey.substr(pos + 1);
	return qualifiedKey;
}

static bool isDirectResourceBindType(ShaderCodeModule::ShaderResources::BindType bindType)
{
	using BindType = ShaderCodeModule::ShaderResources::BindType;
	switch (bindType)
	{
		case BindType::sampledImages:
		case BindType::texture:
		case BindType::sampler:
		case BindType::rawBuffer:
		case BindType::storageTexture:
		case BindType::storageBuffer:
			return true;
		default:
			return false;
	}
}

std::set<std::string> generateBindingKeys(std::stringstream& out, const std::vector<uint32_t>& spirv, ShaderLanguage lang)
{
	auto resources = ShaderLanguageConverter::spirvCrossReflectedBindInfo(spirv, lang);
	std::set<std::string> bindingBlockNames;

	// Push constant block -> struct with static inline BindingKey members
	if (!resources.pushConstantName.empty())
	{
		bindingBlockNames.insert(resources.pushConstantName);
		out << "struct " << resources.pushConstantName << "\n{\n";
		for (auto& info : resources.bindInfoPool)
		{
			if (info.bindType == ShaderCodeModule::ShaderResources::pushConstantMembers)
				out << "\tstatic inline ::EmbeddedShader::BindingKey " << info.variateName
				    << "{" << info.byteOffset << ", " << info.typeSize
				    << ", " << static_cast<int32_t>(info.bindType) << ", " << info.location << "};\n";
		}
		out << "};\n";
	}

	// UBO block -> struct with static inline BindingKey members
	if (!resources.uniformBufferName.empty())
	{
		bindingBlockNames.insert(resources.uniformBufferName);
		out << "struct " << resources.uniformBufferName << "\n{\n";
		for (auto& info : resources.bindInfoPool)
		{
			if (info.bindType == ShaderCodeModule::ShaderResources::uniformBufferMembers)
				out << "\tstatic inline ::EmbeddedShader::BindingKey " << info.variateName
				    << "{" << info.byteOffset << ", " << info.typeSize
				    << ", " << static_cast<int32_t>(info.bindType) << ", " << info.location << "};\n";
		}
		out << "};\n";
	}

	// Stage inputs
	for (auto& info : resources.bindInfoPool)
	{
		if (info.bindType == ShaderCodeModule::ShaderResources::stageInputs)
			out << "static inline ::EmbeddedShader::BindingKey " << info.variateName
			    << "{" << info.byteOffset << ", " << info.typeSize
			    << ", " << static_cast<int32_t>(info.bindType) << ", " << info.location << "};\n";
	}

	// Stage outputs
	for (auto& info : resources.bindInfoPool)
	{
		if (info.bindType == ShaderCodeModule::ShaderResources::stageOutputs)
			out << "static inline ::EmbeddedShader::BindingKey " << info.variateName
			    << "{" << info.byteOffset << ", " << info.typeSize
			    << ", " << static_cast<int32_t>(info.bindType) << ", " << info.location << "};\n";
	}

	// Descriptor resources (sampled/storage/buffer/sampler) for direct key binding.
	for (auto& info : resources.bindInfoPool)
	{
		if (isDirectResourceBindType(info.bindType))
			out << "static inline ::EmbeddedShader::BindingKey " << info.variateName
			    << "{" << info.byteOffset << ", " << info.typeSize
			    << ", " << static_cast<int32_t>(info.bindType) << ", " << info.binding << "};\n";
	}

	return bindingBlockNames;
}

// Helper: emit a block proxy struct (push constants or UBO) into the output stream.
// Returns the initializer string for the parent constructor, or empty if block is absent.
static std::string emitBlockProxy(std::stringstream& out,
                                   const std::string& blockName,
                                   ShaderCodeModule::ShaderResources::BindType bindType,
                                   const ShaderCodeModule::ShaderResources& resources)
{
	if (blockName.empty()) return {};

	out << "struct " << blockName << "_t {\n";
	std::vector<std::string> fieldInits;
	for (auto& info : resources.bindInfoPool)
	{
		if (info.bindType == bindType)
		{
			out << "\t::EmbeddedShader::BoundField<P> " << info.variateName << ";\n";
			std::stringstream ss;
			ss << info.variateName << "(p, " << info.byteOffset << ", " << info.typeSize
			   << ", " << static_cast<int32_t>(info.bindType) << ", " << info.location << ")";
			fieldInits.push_back(ss.str());
		}
	}
	out << "\t" << blockName << "_t(P* p)";
	if (!fieldInits.empty())
	{
		out << " : ";
		for (size_t i = 0; i < fieldInits.size(); ++i)
		{
			if (i > 0) out << ", ";
			out << fieldInits[i];
		}
	}
	out << " {}\n";
	out << "} " << blockName << ";\n";
	return blockName + "(p)";
}

static void emitCtorBody(std::stringstream& out, const std::string& className,
                          const std::vector<std::string>& initList)
{
	if (initList.empty())
	{
		out << className << "(P* p) { (void)p; }\n";
	}
	else
	{
		out << className << "(P* p) : ";
		for (size_t i = 0; i < initList.size(); ++i)
		{
			if (i > 0) out << ", ";
			out << initList[i];
		}
		out << " {}\n";
	}
}

void generateBindings(std::stringstream& out, const std::vector<uint32_t>& spirv, ShaderLanguage lang)
{
	auto resources = ShaderLanguageConverter::spirvCrossReflectedBindInfo(spirv, lang);

	// Size metadata for cross-stage validation in TypedRasterizerPipeline
	out << "static constexpr size_t pushConstantBlockSize = " << resources.pushConstantSize << ";\n";
	out << "static constexpr size_t uniformBufferBlockSize = " << resources.uniformBufferSize << ";\n";

	// --- ResourceBindings<P>: push constants + UBO (shared between VS and FS) ---
	out << "template<typename P>\nstruct ResourceBindings {\n";
	{
		std::vector<std::string> initList;
		auto s = emitBlockProxy(out, resources.pushConstantName,
		                         ShaderCodeModule::ShaderResources::pushConstantMembers, resources);
		if (!s.empty()) initList.push_back(std::move(s));

		s = emitBlockProxy(out, resources.uniformBufferName,
		                    ShaderCodeModule::ShaderResources::uniformBufferMembers, resources);
		if (!s.empty()) initList.push_back(std::move(s));

		for (auto& info : resources.bindInfoPool)
		{
			if (!isDirectResourceBindType(info.bindType))
				continue;

			out << "::EmbeddedShader::BoundField<P> " << info.variateName << ";\n";
			std::stringstream ss;
			ss << info.variateName << "(p, " << info.byteOffset << ", " << info.typeSize
			   << ", " << static_cast<int32_t>(info.bindType) << ", " << info.binding << ")";
			initList.push_back(ss.str());
		}

		emitCtorBody(out, "ResourceBindings", initList);
	}
	out << "};\n";

	// --- OutputBindings<P>: stage outputs (render targets) ---
	out << "template<typename P>\nstruct OutputBindings {\n";
	{
		std::vector<std::string> initList;
		for (auto& info : resources.bindInfoPool)
		{
			if (info.bindType == ShaderCodeModule::ShaderResources::stageOutputs)
			{
				out << "::EmbeddedShader::BoundField<P> " << info.variateName << ";\n";
				std::stringstream ss;
				ss << info.variateName << "(p, " << info.byteOffset << ", " << info.typeSize
				   << ", " << static_cast<int32_t>(info.bindType) << ", " << info.location << ")";
				initList.push_back(ss.str());
			}
		}
		emitCtorBody(out, "OutputBindings", initList);
	}
	out << "};\n";

	// --- Bindings<P>: combined (for single-stage pipelines like compute) ---
	out << "template<typename P>\nstruct Bindings : ResourceBindings<P>, OutputBindings<P> {\n";
	out << "Bindings(P* p) : ResourceBindings<P>(p), OutputBindings<P>(p) {}\n";
	out << "};\n";
}

int main(int argc, char** argv)
{
	std::cout << "Helicon Shader Compile Scripts\n";
	if (argc < 2)
	{
		std::cout << "NOTE:No input\n";
		return 1;
	}
	std::filesystem::path path = "";
	std::filesystem::path outPath = "";
	std::string ext  = ".h";
	ShaderLanguage inputLanguage = ShaderLanguage::GLSL;
	ShaderStage inputStage = ShaderStage::VertexShader;
	bool stageExplicit = false;

	//main 参数解析
	for (int i = 1; i < argc;)
	{
		if (std::string arg = argv[i]; arg == "-s")
		{
			++i;
			path = argv[i];
		}
		else if (arg == "-o")
		{
			++i;
			outPath = argv[i];
		}
		else if (arg == "-l")
		{
			++i;
			arg = argv[i];
			if (arg == "glsl")
				inputLanguage = ShaderLanguage::GLSL;
			else if (arg == "hlsl")
				inputLanguage = ShaderLanguage::HLSL;
			else
			{
				std::cout << "ERROR:Unrecognized Shader Language.\n";
				return 1;
			}
		}
		else if (arg == "-t")
		{
			++i;
			arg = argv[i];
			if (arg == "vert" || arg == "vertex")
			{
				inputStage = ShaderStage::VertexShader;
				stageExplicit = true;
			}
			else if (arg == "frag" || arg == "fragment")
			{
				inputStage = ShaderStage::FragmentShader;
				stageExplicit = true;
			}
			else if (arg == "comp" || arg == "compute")
			{
				inputStage = ShaderStage::ComputeShader;
				stageExplicit = true;
			}
			else
			{
				std::cout << "ERROR:Unrecognized Shader Stage.\n";
				return 1;
			}
		}
		else if (arg == "-output-file-extension")
		{
			++i;
			ext = argv[i];
		}
		else
		{
			std::cout << "ERROR:Unrecognized Parameter.\n";
			return 1;
		}
		++i;
	}

	if (path.empty())
	{
		std::cout << "ERROR:Cannot enter an empty source file path.\n";
		return 1;
	}

	std::fstream file(path,std::ios::in);
	if (!file.is_open())
	{
		std::cout << "ERROR:Cannot open the source file.\n";
		return 1;
	}

	if (outPath.empty())
	{
		std::cout << "NOTE:No output. Use the source file directory as the default output directory.\n";
		outPath = path.parent_path();
	}

	if (ext.empty())
	{
		std::cout << "NOTE:Invalid file extension. Use the default file extension (.h).\n";
		ext = ".h";
	}
	ext = ext[0] == '.' ? ext : "." + ext;

	std::string code = (std::stringstream{} << file.rdbuf()).str();
	file.close();

	// 如果未显式指定 stage，则根据文件名后缀或内容推断
	if (!stageExplicit)
	{
		auto stem = path.stem().string();
		auto extension = path.extension().string();
		// 检查常见命名: xxx.vert, xxx.frag, xxx.comp, xxx.vert.glsl 等
		if (extension == ".frag" || extension == ".fs" || stem.ends_with(".frag") ||
		    stem.find("frag") != std::string::npos)
			inputStage = ShaderStage::FragmentShader;
		else if (extension == ".comp" || extension == ".cs" || stem.ends_with(".comp") ||
		         stem.find("compute") != std::string::npos)
			inputStage = ShaderStage::ComputeShader;
		else if (extension == ".vert" || extension == ".vs" || stem.ends_with(".vert"))
			inputStage = ShaderStage::VertexShader;
		else
		{
			// 扫描代码中的 gl_FragCoord / gl_Position / gl_GlobalInvocationID 等内置变量
			if (code.find("gl_FragCoord") != std::string::npos ||
			    code.find("gl_SampleID") != std::string::npos ||
			    (code.find("out vec4") != std::string::npos && code.find("gl_Position") == std::string::npos))
				inputStage = ShaderStage::FragmentShader;
			else if (code.find("gl_GlobalInvocationID") != std::string::npos ||
			         code.find("gl_WorkGroupID") != std::string::npos ||
			         code.find("gl_LocalInvocationID") != std::string::npos)
				inputStage = ShaderStage::ComputeShader;
			// else default remains VertexShader
		}
		std::cout << "INFO:Auto-detected shader stage: " << static_cast<int>(inputStage) << "\n";
	}

	auto spirv = ShaderLanguageConverter::glslangSpirvCompiler(code,inputLanguage,inputStage,{ path.parent_path() }, false);
	if (spirv.empty())
	{
		std::cout << "ERROR:Cannot compile SPIR-V.\n";
		return 1;
	}
	std::cout << "SUCCESS:SPIR-V compiled.\n";

	auto irReflections = ShaderLanguageConverter::spirvCrossGetIRReflection(spirv);
	std::cout << "SUCCESS:Obtain reflection information from SPIR-V IR through SPIRV-CROSS.\n";

	// for (auto& irReflection: irReflections)
	// {
	// 	if (irReflection.type == IRReflection::Type::FunctionSignature)
	// 	{
	// 		auto& signature = std::get<FunctionSignature>(irReflection.info);
	// 		std::cout << "Function:" << signature.name.substr(0, signature.name.find('(')) << "\n";
	// 		std::cout << "Return Type:" << signature.returnTypeName << "\n";
	// 		std::cout << "Parameter List:\n";
	// 		for (auto& parameter: signature.parameters)
	// 		{
	// 			std::cout << "\t""Name:" << parameter.name << "\n";
	// 			std::cout << "\t""Type:" << parameter.typeName << "\n\n";
	// 		}
	// 	}
	//
	// 	if (irReflection.type == IRReflection::Type::Struct)
	// 	{
	// 		auto& structInfo = std::get<StructInfo>(irReflection.info);
	// 		std::cout << "Struct: " << structInfo.name << "\n";
	// 		std::cout << "Members:\n";
	// 		for (auto& member: structInfo.members)
	// 		{
	// 			std::cout << "\t""Name:" << member.name << "\n";
	// 			std::cout << "\t""Type:" << member.typeName << "\n\n";
	// 		}
	// 	}
	// }

	std::cout << "INFO:Generate the final C++ shader...\n";
	std::stringstream out;
	out << "#pragma once\n#include <Codegen/VariateProxy.h>\n";

	auto fileName = path.string();
	std::ranges::replace(fileName, '\\', '_');
	std::ranges::replace(fileName, '/', '_');
	std::ranges::replace(fileName, '.', '_');
	std::ranges::replace(fileName, ':', '_');

	// Wrap everything in a per-shader struct to avoid redefinition across .hpp files
	auto nsName = sanitizeToIdentifier(path);
	out << "struct " << nsName << " {\n";

	generateBinary(out, fileName, spirv);

	// 稳定别名：用户可通过 vert_glsl::spirv 引用预编译 SPIR-V 二进制
	out << "static inline auto& spirv = " << fileName << ";\n";

	// Generate BindingKey struct declarations first, collect binding block names
	std::cout << "INFO:Generate binding keys for struct '" << nsName << "'...\n";
	auto bindingBlockNames = generateBindingKeys(out, spirv, inputLanguage);

	// Generate IR reflection (skip structs that overlap with BindingKey structs)
	for (auto& irReflection : irReflections)
	{
		if (irReflection.type == IRReflection::Type::FunctionSignature)
		{
			auto& signature = std::get<FunctionSignature>(irReflection.info);
			if (signature.isEntryPoint) continue;
			buildFunctionSignature(signature, out, fileName);
			out << "\n";
		}

		if (irReflection.type == IRReflection::Type::Struct)
		{
			auto& structInfo = std::get<StructInfo>(irReflection.info);
			// Skip structs already generated as BindingKey structs
			if (bindingBlockNames.contains(structInfo.name)) continue;
			buildStruct(structInfo,out);
			out << "\n";
		}
	}

	// Generate Bindings<P> template for direct member access (TypedRasterizerPipeline / TypedComputePipeline)
	std::cout << "INFO:Generate Bindings<P> for struct '" << nsName << "'...\n";
	generateBindings(out, spirv, inputLanguage);

	out << "}; // struct " << nsName << "\n";

	//std::cout << out.str();

	auto stem = path.stem();
	auto outFilePath = outPath / (stem.string() + ext);
	file.open(outFilePath,std::ios::out);
	file << out.str();
	file.close();

	std::cout << "SUCCESS:The C++ shader has been output to the following directory:\n\t" << outFilePath.string();

	return 0;
}
