//#include "texture_scenario.h"
//
//#include <array>
//#include <cmath>
//#include <filesystem>
//#include <memory>
//#include <mutex>
//#include <sstream>
//#include <string>
//#include <utility>
//#include <vector>
//
//#include "Codegen/BuiltinVariate.h"
//#include "Codegen/CustomLibrary.h"
//#include "Codegen/TypeAlias.h"
//#include "texture_data.h"
//#include "../scenario_registry.h"
//
//#ifndef HELICON_STRINGIZE_
//#define HELICON_STRINGIZE_(X) #X
//#endif
//#ifndef GLSL
//#define GLSL(path) HELICON_STRINGIZE_(path.hpp)
//#endif
//
//#include GLSL(texture_vert.glsl)
//#include GLSL(texture_frag.glsl)