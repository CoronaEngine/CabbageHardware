
#include "ShaderCodeCompiler.h"

#include <slang-com-helper.h>
#include <slang-com-ptr.h>
#include <slang.h>

#include <Codegen/AST/Parser.hpp>

#include "ShaderHardcodeManager.h"
#include "ShaderLanguageConverter.h"
#include <shared_mutex>


namespace EmbeddedShader
{
    std::shared_mutex threadMutex;

    std::string enumToString(ShaderLanguage language) {
        switch (language)
        {
            case ShaderLanguage::GLSL:
                return "GLSL";
            case ShaderLanguage::HLSL:
                return "HLSL";
            case ShaderLanguage::SpirV:
                return "SpirV";
            case ShaderLanguage::Slang:
                return "Slang";
            default:break;
        }
        return "Unknown";
    }

    std::string enumToString(ShaderStage stage)
    {
        switch (stage)
        {
            case ShaderStage::VertexShader:
                return "VertexShader";
            case ShaderStage::FragmentShader:
                return "FragmentShader";
            case ShaderStage::ComputeShader:
                return "ComputeShader";
            default:break;
        }
        return "Unknown";
    }

    ShaderCodeCompiler::ShaderCodeCompiler(const std::string& shaderCode, ShaderStage inputStage,
        ShaderLanguage language, CompilerOption option, const std::source_location& sourceLocation)
    {
        sourceLocationStr = ShaderHardcodeManager::getSourceLocationString(sourceLocation);
        stage = enumToString(inputStage);
        compile(shaderCode,inputStage,language,option);
    }

    ShaderCodeModule ShaderCodeCompiler::getShaderCode(ShaderLanguage language, bool bindless) const
    {
        std::shared_lock<std::shared_mutex> lock(threadMutex);

        std::string bindlessStr = bindless ? "_Bindless" : "";
        ShaderCodeModule result;
        auto languageStr = enumToString(language);
        result.shaderCode = std::get<1>(ShaderHardcodeManager::getHardcodeShader(stage, ShaderHardcodeManager::getItemName(sourceLocationStr,  languageStr + bindlessStr)));
        result.shaderResources = std::get<0>(ShaderHardcodeManager::getHardcodeShader(stage, ShaderHardcodeManager::getItemName(sourceLocationStr, languageStr + "_Reflection" + bindlessStr)));
        return result;
    }

    void ShaderCodeCompiler::compile(const std::string& shaderCode, ShaderStage inputStage, ShaderLanguage language,
        CompilerOption option) const
    {
#if CABBAGE_ENGINE_DEBUG
        std::string bindlessStr = Ast::Parser::getBindless() ? "_Bindless" : "";
        std::vector<uint32_t> codeSpirV = {};
#ifdef WIN32
        std::vector<uint32_t> codeDXIL = {};
        std::vector<uint32_t> codeDXBC = {};
#endif

        if (Ast::Parser::getBindless())
            option.compileDXBC = false; // DXBC不支持sm6.6 bindless

        if (language != ShaderLanguage::Slang && language != ShaderLanguage::HLSL && !option.compileHLSL)
        {
            //slang可以直接编译到dxil和dxbc，但其他情况需要依赖HLSL，所以如果不需要HLSL就直接跳过
            option.compileDXIL = false;
            option.compileDXBC = false;
        }

        std::string codeGLSL;
        std::string codeHLSL;
        std::string codeSlang;
        std::vector<ShaderCodeModule::ShaderResources> reflections;

        switch (language)
        {
            case ShaderLanguage::Slang:
            {
                codeSlang = shaderCode;

                std::vector<std::string> outputs;
                std::vector<std::vector<uint32_t>> binaryOutputs;
                std::vector<ShaderLanguage> binaryLanguages;
                std::vector<ShaderLanguage> languages;
                if (option.compileSpirV)
                    binaryLanguages.push_back(ShaderLanguage::SpirV);
#ifdef WIN32
                if (option.compileDXIL)
                    binaryLanguages.push_back(ShaderLanguage::DXIL);
                if (option.compileDXBC)
                    binaryLanguages.push_back(ShaderLanguage::DXBC);
#endif
                if (option.compileGLSL)
                    languages.push_back(ShaderLanguage::GLSL);
                if (option.compileHLSL)
                    languages.push_back(ShaderLanguage::HLSL);


                reflections = ShaderLanguageConverter::slangCompiler(codeSlang, binaryLanguages, languages, binaryOutputs, outputs, true);
                size_t index = 0;
                if (option.compileSpirV)
                    codeSpirV = binaryOutputs[index++];
#ifdef WIN32
                if (option.compileDXIL)
                    codeDXIL = binaryOutputs[index++];
                if (option.compileDXBC)
                    codeDXBC = binaryOutputs[index++];
#endif

                index = 0;
                if (option.compileGLSL)
                    codeGLSL = outputs[index++];
                if (option.compileHLSL)
                    codeHLSL = outputs[index++];
                break;
            }
            case ShaderLanguage::GLSL:
                codeGLSL = shaderCode;
                if (option.compileSpirV)
                    codeSpirV = ShaderLanguageConverter::glslangSpirvCompiler(codeGLSL, language, inputStage);
                if (option.compileGLSL)
                    codeGLSL = ShaderLanguageConverter::spirvCrossConverter(codeSpirV, ShaderLanguage::GLSL);
                if (option.compileHLSL)
                    codeHLSL = ShaderLanguageConverter::spirvCrossConverter(codeSpirV, ShaderLanguage::HLSL);
#ifdef WIN32
                // codeDXIL = ShaderLanguageConverter::dxilCompiler(codeHLSL, inputStage);
                // if (!bindless)
                //     codeDXBC = ShaderLanguageConverter::dxbcCompiler(codeHLSL, inputStage);
#endif
                break;
            case ShaderLanguage::HLSL:
                codeHLSL = shaderCode;
                if (option.compileSpirV)
                    codeSpirV = ShaderLanguageConverter::glslangSpirvCompiler(codeHLSL, language, inputStage);
                if (option.compileGLSL)
                    codeGLSL = ShaderLanguageConverter::spirvCrossConverter(codeSpirV, ShaderLanguage::GLSL);
                if (option.compileHLSL)
                    codeHLSL = ShaderLanguageConverter::spirvCrossConverter(codeSpirV, ShaderLanguage::HLSL);
#ifdef WIN32
                if (option.compileDXIL)
                    codeDXIL = ShaderLanguageConverter::dxilCompiler(codeHLSL, inputStage);
                if (option.compileDXBC)
                    codeDXBC = ShaderLanguageConverter::dxbcCompiler(codeHLSL, inputStage);
#endif
                break;
                //case ShaderLanguage::SpirV:
                //    codeSpirV = shaderCode;
                //    codeGLSL = ShaderLanguageConverter::spirvCrossConverter(codeSpirV, ShaderLanguage::GLSL);
                //    codeHLSL = ShaderLanguageConverter::spirvCrossConverter(codeSpirV, ShaderLanguage::HLSL);
                //    break;
            default:
                break;
        }

        if (option.spvLinkBinary && !option.spvLinkBinary->empty() && !codeSpirV.empty())
        {
            std::vector<std::vector<uint32_t>> src = *option.spvLinkBinary;
            src.push_back(codeSpirV);
            codeSpirV = ShaderLanguageConverter::spirvLinker(src);
        }
        
        //auto functionSignatures = ShaderLanguageConverter::spirvCrossGetFunctionSignatures(codeSpirV);


        // support mutil-thread
        {
            std::unique_lock<std::shared_mutex> lock(threadMutex);

        size_t index = 0;
        if (!codeSpirV.empty())
        {
            ShaderHardcodeManager::addTarget(codeSpirV, stage,ShaderHardcodeManager::getItemName(sourceLocationStr, "SpirV" + bindlessStr));
            if (!reflections.empty()) ShaderHardcodeManager::addTarget(reflections[index++], stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "SpirV_Reflection" + bindlessStr));
            else
            {
                auto shaderResource = ShaderLanguageConverter::spirvCrossReflectedBindInfo(codeSpirV, ShaderLanguage::HLSL);
                ShaderHardcodeManager::addTarget(shaderResource, stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "SpirV_Reflection" + bindlessStr));
            }
        }
        if (!codeGLSL.empty())
        {
            ShaderHardcodeManager::addTarget(codeGLSL, stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "GLSL" + bindlessStr));
            if (!reflections.empty()) ShaderHardcodeManager::addTarget(reflections[index++], stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "GLSL_Reflection" + bindlessStr));
            else
            {
                auto shaderResource = ShaderLanguageConverter::spirvCrossReflectedBindInfo(codeSpirV, ShaderLanguage::HLSL);
                ShaderHardcodeManager::addTarget(shaderResource, stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "GLSL_Reflection" + bindlessStr));
            }
        }
        if (!codeHLSL.empty())
        {
            ShaderHardcodeManager::addTarget(codeHLSL, stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "HLSL" + bindlessStr));
            if (!reflections.empty()) ShaderHardcodeManager::addTarget(reflections[index++], stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "HLSL_Reflection" + bindlessStr));
            else
            {
                auto shaderResource = ShaderLanguageConverter::spirvCrossReflectedBindInfo(codeSpirV, ShaderLanguage::HLSL);
                ShaderHardcodeManager::addTarget(shaderResource, stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "HLSL_Reflection" + bindlessStr));
            }
        }
        if (!codeSlang.empty())
        {
            ShaderHardcodeManager::addTarget(codeSlang, stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "Slang" + bindlessStr));
            if (!reflections.empty()) ShaderHardcodeManager::addTarget(reflections[0], stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "Slang_Reflection" + bindlessStr));
            else
            {
                auto shaderResource = ShaderLanguageConverter::spirvCrossReflectedBindInfo(codeSpirV, ShaderLanguage::HLSL);
                ShaderHardcodeManager::addTarget(shaderResource, stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "Slang_Reflection" + bindlessStr));
            }
        }
#ifdef WIN32
        if (!codeDXIL.empty())
        {
            ShaderHardcodeManager::addTarget(codeDXIL, stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "DXIL" + bindlessStr));
            if (!reflections.empty()) ShaderHardcodeManager::addTarget(reflections[index++], stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "DXIL_Reflection" + bindlessStr));
            else
            {
                auto shaderResource = ShaderLanguageConverter::spirvCrossReflectedBindInfo(codeSpirV, ShaderLanguage::HLSL);
                ShaderHardcodeManager::addTarget(shaderResource, stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "DXIL_Reflection" + bindlessStr));
            }
        }
        if (!codeDXBC.empty())
        {
            ShaderHardcodeManager::addTarget(codeDXBC, stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "DXBC"));
            if (!reflections.empty()) ShaderHardcodeManager::addTarget(reflections[index++], stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "DXBC_Reflection" + bindlessStr));
            else
            {
                auto shaderResource = ShaderLanguageConverter::spirvCrossReflectedBindInfo(codeSpirV, ShaderLanguage::HLSL);
                ShaderHardcodeManager::addTarget(shaderResource, stage, ShaderHardcodeManager::getItemName(sourceLocationStr, "DXBC_Reflection" + bindlessStr));
            }
        }
#endif
        }
#endif
    }
}