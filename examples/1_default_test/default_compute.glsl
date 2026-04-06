#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable
#include "compute_header.glsl"

layout(local_size_x = 8, local_size_y = 8) in;

layout(set = 2, binding = 0, rgba16f) uniform image2D inputImageRGBA16[];

layout(set = 3, binding = 0) uniform GlobalUniformParam
{
    uint imageID;
} globalParams;

void main()
{
    uint imageID = globalParams.imageID;
    imageID = nonuniformEXT(imageID);
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imageExtent = imageSize(inputImageRGBA16[imageID]);
    if (pixelCoord.x >= imageExtent.x || pixelCoord.y >= imageExtent.y)
    {
        return;
    }

    vec4 color = imageLoad(inputImageRGBA16[imageID], pixelCoord);
    imageStore(inputImageRGBA16[imageID], pixelCoord, vec4(acesFilmicToneMapCurve(color.xyz), color.w));
}
