#include "texture_loader.h"

#include <cstring>
#include <sstream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_DXT_IMPLEMENTATION
#include <stb_dxt.h>

bool upload_texture_data(const void *input_data,
                         uint32_t width,
                         uint32_t height,
                         ImageFormat format,
                         TextureLoadResult &result,
                         std::string &error_message)
{
    HardwareImageCreateInfo create_info;
    create_info.width = width;
    create_info.height = height;
    create_info.format = format;
    create_info.usage = ImageUsage::SampledImage;
    create_info.arrayLayers = 1;
    create_info.mipLevels = 1;

    result.texture = HardwareImage(create_info);
    if (!result.texture)
    {
        error_message = "Failed to create HardwareImage.";
        return false;
    }

    HardwareExecutor executor;
    executor << result.texture.copyFrom(input_data) << executor.commit();

    result.descriptor_id = result.texture.storeDescriptor();
    result.width = width;
    result.height = height;
    result.success = true;
    return true;
}

std::vector<uint8_t> compress_rgba_to_bc1(const unsigned char *rgba_data, int width, int height)
{
    const uint32_t block_count_x = static_cast<uint32_t>((width + 3) / 4);
    const uint32_t block_count_y = static_cast<uint32_t>((height + 3) / 4);
    std::vector<uint8_t> compressed_data(block_count_x * block_count_y * 8);

    for (uint32_t by = 0; by < block_count_y; ++by)
    {
        for (uint32_t bx = 0; bx < block_count_x; ++bx)
        {
            uint8_t block[64];

            for (int py = 0; py < 4; ++py)
            {
                for (int px = 0; px < 4; ++px)
                {
                    const int x = static_cast<int>(bx * 4 + px);
                    const int y = static_cast<int>(by * 4 + py);
                    const int dst_index = (py * 4 + px) * 4;

                    if (x < width && y < height)
                    {
                        const int src_index = (y * width + x) * 4;
                        std::memcpy(&block[dst_index], &rgba_data[src_index], 4);
                    }
                    else
                    {
                        block[dst_index + 0] = 0;
                        block[dst_index + 1] = 0;
                        block[dst_index + 2] = 0;
                        block[dst_index + 3] = 255;
                    }
                }
            }

            uint8_t *out_block = &compressed_data[(by * block_count_x + bx) * 8];
            stb_compress_dxt_block(out_block, block, 0, STB_DXT_NORMAL);
        }
    }

    return compressed_data;
}

TextureLoadResult load_texture_from_file(const std::filesystem::path &texture_path,
                                         const TextureLoadOptions &options,
                                         std::string &error_message)
{
    TextureLoadResult result;

    if (!std::filesystem::exists(texture_path))
    {
        error_message = "Texture file does not exist: " + texture_path.string();
        return result;
    }

    stbi_set_flip_vertically_on_load(options.flip_vertically ? 1 : 0);

    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc *pixels = stbi_load(texture_path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr)
    {
        std::ostringstream oss;
        oss << "Failed to load texture: " << texture_path.string();
        if (const char *reason = stbi_failure_reason())
        {
            oss << ", reason: " << reason;
        }
        error_message = oss.str();
        return result;
    }

    const auto free_pixels = [&] {
        stbi_image_free(pixels);
    };

    if (width <= 0 || height <= 0)
    {
        free_pixels();
        error_message = "Invalid texture dimensions for file: " + texture_path.string();
        return result;
    }

    switch (options.encoding)
    {
    case TextureEncoding::RGBA8_SRGB:
    {
        const bool ok = upload_texture_data(pixels,
                                            static_cast<uint32_t>(width),
                                            static_cast<uint32_t>(height),
                                            ImageFormat::RGBA8_SRGB,
                                            result,
                                            error_message);
        free_pixels();
        return ok ? result : TextureLoadResult{};
    }
    case TextureEncoding::BC1_RGB_UNORM:
    case TextureEncoding::BC1_RGB_SRGB:
    {
        auto compressed_data = compress_rgba_to_bc1(pixels, width, height);
        free_pixels();

        const ImageFormat bc1_format = (options.encoding == TextureEncoding::BC1_RGB_SRGB)
                                     ? ImageFormat::BC1_RGB_SRGB
                                     : ImageFormat::BC1_RGB_UNORM;

        const bool ok = upload_texture_data(compressed_data.data(),
                                            static_cast<uint32_t>(width),
                                            static_cast<uint32_t>(height),
                                            bc1_format,
                                            result,
                                            error_message);
        return ok ? result : TextureLoadResult{};
    }
    }

    free_pixels();
    error_message = "Unsupported texture encoding.";
    return result;
}

TextureLoadResult load_texture_rgba8_srgb(const std::filesystem::path &texture_path,
                                          bool flip_vertically,
                                          std::string &error_message)
{
    TextureLoadOptions options;
    options.encoding = TextureEncoding::RGBA8_SRGB;
    options.flip_vertically = flip_vertically;
    return load_texture_from_file(texture_path, options, error_message);
}

TextureLoadResult load_texture_bc1(const std::filesystem::path &texture_path,
                                   bool use_srgb,
                                   bool flip_vertically,
                                   std::string &error_message)
{
    TextureLoadOptions options;
    options.encoding = use_srgb ? TextureEncoding::BC1_RGB_SRGB : TextureEncoding::BC1_RGB_UNORM;
    options.flip_vertically = flip_vertically;
    return load_texture_from_file(texture_path, options, error_message);
}
