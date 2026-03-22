#pragma once

#include <string>

namespace EmbeddedShader
{

struct BindingKey
{
	std::string bindingName;
	std::string getAstName() const { return bindingName; }
};

} // namespace EmbeddedShader
