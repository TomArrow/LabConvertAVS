#include "LabConvert.h"
#include <cmath>

// The color conversion code/logic is lifted and adapted from https://bottosson.github.io/posts/oklab/

// 
// We are going straight from linear sRGB to cone response 
// But for reference, here is the normal XYZ to cone response matrix:
//  M 1  = (   +0.8189330101 +0.0329845436 +0.0482003018   +0.3618667424 +0.9293118715 +0.2643662691   -0.1288597137 +0.0361456387 +0.6338517070   ) 
//
inline void ConvertToOkLChab::PixelFromSRGBToOkLChab(float& r, float& g, float& b, float& l, float& c, float& hab) {

    float l0, m, s;
    float L, A, B;
    // sRGB straight to cone response:
    l0 = 0.4122214708f * r + 0.5363325363f * g + 0.0514459929f * b;
    m = 0.2119034982f * r + 0.6806995451f * g + 0.1073969566f * b;
    s = 0.0883024619f * r + 0.2817188376f * g + 0.6299787005f * b;

    // Cubic roots
    l0 = std::cbrtf(l0);
    m = std::cbrtf(m);
    s = std::cbrtf(s);

    // XYZ to LAB
    L = 0.2104542553f * l0 + 0.7936177850f * m - 0.0040720468f * s;
    A = 1.9779984951f * l0 - 2.4285922050f * m + 0.4505937099f * s;
    B = 0.0259040371f * l0 + 0.7827717662f * m - 0.8086757660f * s;

    l = L;
    //c = std::sqrtf(std::powf(A, 2) + std::powf(B, 2));
    c = std::sqrtf(A*A + B*B);
    hab = std::atan2(B, A);
}


ConvertToOkLChab::ConvertToOkLChab(PClip _child, IScriptEnvironment* env) :
    GenericVideoFilter(_child) {
    if (!vi.IsPlanar() || !vi.IsRGB() || vi.BitsPerComponent() != 32) {
        env->ThrowError("ConvertToOkLChab: 32 bit float RGBPS only!");
    }

    // Create output VideoInfo
    memset(&viOut, 0, sizeof(VideoInfo));
    viOut.width = vi.width;
    viOut.height = vi.height;
    viOut.fps_numerator = vi.fps_numerator;
    viOut.fps_denominator = vi.fps_denominator;
    viOut.num_frames = vi.num_frames;
    viOut.pixel_type = VideoInfo::CS_YUV444PS;
    viOut.sample_type = SAMPLE_FLOAT;
    viOut.nchannels = vi.nchannels;
    viOut.audio_samples_per_second = vi.audio_samples_per_second;
    viOut.num_audio_samples = vi.num_audio_samples;
}


PVideoFrame __stdcall ConvertToOkLChab::GetFrame(int n, IScriptEnvironment* env) {

    PVideoFrame src = child->GetFrame(n, env);

    PVideoFrame dst = env->NewVideoFrame(viOut);

    float* srcp[3];
    float* dstp[3];
    int src_pitch[3], dst_pitch[3], row_size[3], height[3];
    //int p, x, y;

    int planes[] = { PLANAR_R, PLANAR_G, PLANAR_B };
    int labPlanes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    for (int p = 0; p < 3; p++) {
        srcp[p] = (float*)src->GetReadPtr(planes[p]);
        dstp[p] = (float*)dst->GetWritePtr(labPlanes[p]);

        src_pitch[p] = src->GetPitch(planes[p]);
        dst_pitch[p] = dst->GetPitch(labPlanes[p]);
        row_size[p] = dst->GetRowSize(labPlanes[p]) / sizeof(float);
        height[p] = dst->GetHeight(labPlanes[p]);
    }

    if (height[0] != height[1] || height[1] != height[2]) {
        env->ThrowError("LabConvert: Heights of all RGB planes must be same!");
    }
    if (row_size[0] != row_size[1] || row_size[1] != row_size[2]) {
        env->ThrowError("LabConvert: Rowsize (width) of all RGB planes must be same!");
    }

//#pragma omp parallel for 
    for (int y = 0; y < height[0]; y++) {
        float* srcpLocal[3];
        float* dstpLocal[3];
        for (int p = 0; p < 3; p++) {
            srcpLocal[p] = (float*)((char*)srcp[p] + src_pitch[p] * y);
            dstpLocal[p] = (float*)((char*)dstp[p] + dst_pitch[p] * y);
        }
        int rowSize0 = row_size[0];


        for (int x = 0; x < rowSize0; x++) {

            PixelFromSRGBToOkLChab(srcpLocal[0][x], srcpLocal[1][x], srcpLocal[2][x], dstpLocal[0][x], dstpLocal[1][x], dstpLocal[2][x]);
        }

    }

    return dst;
}