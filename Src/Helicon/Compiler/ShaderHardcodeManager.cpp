#include <regex>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <Compiler/ShaderHardcodeManager.h>

#ifndef CABBAGE_ENGINE_DEBUG
#include <Compiler/HardcodeShaders/HardcodeShaders.h>
#endif

namespace EmbeddedShader
{
	bool ShaderHardcodeManager::hardcodeFileOpened = false;

	std::string ShaderHardcodeManager::getItemName(const std::source_location& sourceLocation, ShaderStage inputStage)
	{
		return getItemName(getSourceLocationString(sourceLocation),enumToString(inputStage));
	}

	std::string ShaderHardcodeManager::getItemName(const std::source_location& sourceLocation, ShaderLanguage language)
	{
		return getItemName(getSourceLocationString(sourceLocation),enumToString(language));
	}

	std::string ShaderHardcodeManager::getItemName(const std::string& sourceLocationFormatString, const std::string& prefix)
	{
		return prefix + "_" + sourceLocationFormatString;
	}

#ifdef CABBAGE_ENGINE_DEBUG

	void ShaderHardcodeManager::addTarget(
		const std::string& shaderCode,
		const std::string& targetName, const std::string& itemName)
	{
		createTarget(targetName);

		std::fstream hardcodeShaderFile(hardcodePath / ("HardcodeShaders" + targetName + ".cpp"), std::ios::out | std::ios::in);
		hardcodeShaderFile.seekg(-static_cast<int>(sizeof("};")), std::ios::end);
		hardcodeShaderFile << "{\"" + itemName + "\", R\"(" + shaderCode + ")\"}," << std::endl;
		hardcodeShaderFile << "};";

		debugHardcodeShaders[targetName][itemName] = shaderCode;
	}

	void ShaderHardcodeManager::addTarget(const std::vector<uint32_t>& shaderCode, const std::string& targetName, const std::string& itemName)
	{
		createTarget(targetName);

		std::fstream hardcodeShaderFile(hardcodePath / ("HardcodeShaders" + targetName + ".cpp"), std::ios::out | std::ios::in);
		hardcodeShaderFile.seekg(-static_cast<int>(sizeof("};")), std::ios::end);
		hardcodeShaderFile << "{\"" + itemName + "\", std::vector<uint32_t>{";
		for (uint32_t code : shaderCode)
		{
			hardcodeShaderFile << code << ",";
		}
		hardcodeShaderFile << "}}," << std::endl;
		hardcodeShaderFile << "};";

		debugHardcodeShaders[targetName][itemName] = shaderCode;
	}

	void ShaderHardcodeManager::addTarget(const ShaderCodeModule::ShaderResources& shaderResource,
		const std::string& targetName, const std::string& itemName)
	{
		createTarget(targetName);

		std::fstream hardcodeShaderFile(hardcodePath / ("HardcodeShaders" + targetName + ".cpp"), std::ios::out | std::ios::in);
		hardcodeShaderFile.seekg(-static_cast<int>(sizeof("};")), std::ios::end);
		hardcodeShaderFile << "{\"" + itemName + "\", " + getShaderResourceOutput(shaderResource) + "}," << std::endl;
		hardcodeShaderFile << "};";

		debugHardcodeShaders[targetName][itemName] = shaderResource;
	}
#endif

	std::variant<ShaderCodeModule::ShaderResources,std::variant<std::vector<uint32_t>,std::string>> ShaderHardcodeManager::getHardcodeShader(const std::string& targetName, const std::string& itemName)
	{
#ifdef CABBAGE_ENGINE_DEBUG
		auto target = debugHardcodeShaders.find(targetName);
		if (target == debugHardcodeShaders.end())
		{
			throw std::runtime_error("Target " + targetName + " not found in hardcoded shaders.");
		}

		auto item = target->second.find(itemName);
		if (item == target->second.end())
		{
			throw std::runtime_error("Item " + itemName + " not found in hardcoded shaders for target " + targetName + ".");
		}

		return item->second;
#else
		auto target = HardcodeShaders::hardcodeShaders.find(targetName);
		if (target == HardcodeShaders::hardcodeShaders.end())
		{
			throw std::runtime_error("Target " + targetName + " not found in hardcoded shaders.");
		}

		auto item = target->second->find(itemName);
		if (item == target->second->end())
		{
			throw std::runtime_error("Item " + itemName + " not found in hardcoded shaders for target " + targetName + ".");
		}

		return item->second;
#endif
	}

	void ShaderHardcodeManager::createTarget(const std::string& name)
	{
		clearOldHardcode();
		// 检查文件是否存在
		std::fstream hardcodeShaderFile;
		if (!hardcodeFileOpened)
		{
			// 如果不存在，则创建文件
			hardcodeShaderFile.open(hardcodePath / "HardcodeShaders.h", std::ios::out | std::ios::trunc);
			hardcodeShaderFile << R"(#pragma once
#include <string>
#include <unordered_map>
#include "../ShaderCodeCompiler.h"
namespace EmbeddedShader
{
	class HardcodeShaders
	{
		friend class ShaderHardcodeManager;
		static std::unordered_map<std::string,std::unordered_map<std::string, std::variant<ShaderCodeModule::ShaderResources,std::variant<std::vector<uint32_t>,std::string>>>*> hardcodeShaders;
	};
})";
			hardcodeShaderFile.close();

			hardcodeShaderFile.open(hardcodePath / "HardcodeShaders.cpp", std::ios::out | std::ios::trunc);
			hardcodeShaderFile << R"(#include"HardcodeShaders.h"
std::unordered_map<std::string,std::unordered_map<std::string, std::variant<EmbeddedShader::ShaderCodeModule::ShaderResources,std::variant<std::vector<uint32_t>,std::string>>>*> EmbeddedShader::HardcodeShaders::hardcodeShaders = {
};)";
			hardcodeShaderFile.close();
			hardcodeFileOpened = true;
		}

		//Declare
		if (auto& ti = targetInfos[name]; !ti.isExistTargetItem)
		{
			// 如果没有声明，添加声明
			hardcodeShaderFile.open(hardcodePath / "HardcodeShaders.h", std::ios::in | std::ios::out);
			hardcodeShaderFile.seekg(-static_cast<int>(sizeof("\t};\n}")), std::ios::end);
			hardcodeShaderFile << "\t\t""static std::unordered_map<std::string, std::variant<ShaderCodeModule::ShaderResources,std::variant<std::vector<uint32_t>,std::string>>> hardcodeShaders" + name + ";" << std::endl;
			hardcodeShaderFile << "\t};\n}";
			hardcodeShaderFile.close();

			hardcodeShaderFile.open(hardcodePath / "HardcodeShaders.cpp", std::ios::in | std::ios::out);
			hardcodeShaderFile.seekg(-static_cast<int>(sizeof("};")), std::ios::end);
			hardcodeShaderFile << "{\"" + name + "\",&hardcodeShaders" + name + "}," << std::endl;
			hardcodeShaderFile << "};";
			hardcodeShaderFile.close();

			ti.isExistTargetItem = true;
		}

		auto& exist = targetInfos[name];
		if (exist.isExistTargetFile)
			return;

		hardcodeShaderFile.open(hardcodePath / ("HardcodeShaders" + name + ".cpp"), std::ios::out | std::ios::trunc);
		hardcodeShaderFile << R"(#include"HardcodeShaders.h"
std::unordered_map<std::string, std::variant<EmbeddedShader::ShaderCodeModule::ShaderResources,std::variant<std::vector<uint32_t>,std::string>>> EmbeddedShader::HardcodeShaders::hardcodeShaders)" + name + R"( = {
};)";
		exist.isExistTargetFile = true;
	}

	void ShaderHardcodeManager::clearOldHardcode()
	{
		if (!isClearOldHardcodeFiles)
		{
			for (auto& entry : std::filesystem::directory_iterator(hardcodePath))
				if (entry.is_regular_file())
					std::filesystem::remove(entry.path());
			isClearOldHardcodeFiles = true;
		}
	}

	std::string ShaderHardcodeManager::getShaderResourceOutput(const ShaderCodeModule::ShaderResources& shaderResources)
	{
		std::stringstream result;
		result << "ShaderCodeModule::ShaderResources{";
		result << shaderResources.pushConstantSize << ",";
		result << "\"" << shaderResources.pushConstantName << "\",";
		result << "{";
		for (const auto& [key, bindInfo]: shaderResources.bindInfoPool)
		{
			result << "{";
			result << "\"" << key << "\",";
			result << "{";
			result << bindInfo.set << ",";
			result << bindInfo.binding << ",";
			result << bindInfo.location << ",";
			result << "\"" << bindInfo.semantic << "\",";
			result << "\"" << bindInfo.variateName << "\",";
			result << "\"" << bindInfo.typeName << "\",";
			result << bindInfo.elementCount << ",";
			result << bindInfo.typeSize << ",";
			result << bindInfo.byteOffset << ",";
			result << "static_cast<EmbeddedShader::ShaderCodeModule::ShaderResources::BindType>(" << bindInfo.bindType << ")";
			result << "}";
			result << "},";
		}
		result << "}";

		result << "}";
		return result.str();
	}

	std::string ShaderHardcodeManager::getSourceLocationString(const std::source_location& sourceLocation)
	{
        std::string fileName = sourceLocation.file_name() + std::string("_line_") + std::to_string(sourceLocation.line()) + std::string("_column_") + std::to_string(sourceLocation.column());
		std::regex pattern(R"(CabbageEngine(.*))");
		std::smatch matches;
		if (std::regex_search(fileName, matches, pattern))
		{
			if (matches.size() > 1)
			{
				fileName = matches[1].str();
			}
			else
			{
				throw std::runtime_error("Failed to resolve source path.");
			}
		}
		std::ranges::replace(fileName, '\\', '_');
		std::ranges::replace(fileName, '/', '_');
		std::ranges::replace(fileName, '.', '_');
		std::ranges::replace(fileName, ':', '_');

		return fileName;
	}
}
