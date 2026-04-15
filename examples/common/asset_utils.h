#pragma once

#include <filesystem>
#include <string>
#include <string_view>

std::filesystem::path project_root_path();
std::filesystem::path examples_root_path();
std::filesystem::path resolve_examples_asset(std::string_view relative_path);
std::string read_text_file(const std::filesystem::path &file_path);
