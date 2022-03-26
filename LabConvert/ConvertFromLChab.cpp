#include "LabConvert.h"
#include <cmath>



inline void ConvertFromLChab::PixelFromLChabToSRGB(float& l, float& c, float& hab, float& r, float& g, float& b) {

    float L, A, B;
    float x, y, z;
    float x2, y2, z2;

    // LChab to LAB
    L = l;
    A = c * std::cosf(hab);
    B = c * std::sinf(hab);

    // LAB to XYZ
    y = (L + 16.0f) / 116.0f;
    x = A / 500.0f + y;
    z = y - B / 200.0f;
    float x3 = x * x * x;
    float z3 = z * z * z;
    x2 = WhiteReference[0] * (x3 > Epsilon ? x3 : (x - 16.0f / 116.0f) / 7.787f);
    y2 = WhiteReference[1] * (L > (Kappa * Epsilon) ? std::powf(((L + 16.0f) / 116.0f), 3.0f) : L / Kappa);
    z2 = WhiteReference[2] * (z3 > Epsilon ? z3 : (z - 16.0f / 116.0f) / 7.787f);

    //
    // XYZ to sRGB matrix:
    // 
    // 3.2404542 -1.5371385 -0.4985314
    // -0.9692660  1.8760108  0.0415560
    // 0.0556434 - 0.2040259  1.0572252
    //
    r = x2 * 3.2404542f + y2 * -1.5371385f + z2 * -0.4985314f;
    g = x2 * -0.9692660f + y2 * 1.8760108f + z2 * 0.0415560f;
    b = x2 * 0.0556434f + y2 * -0.2040259f + z2 * 1.0572252f;
}


ConvertFromLChab::ConvertFromLChab(PClip _child, IScriptEnvironment* env) :
    GenericVideoFilter(_child) {
    if (!vi.IsPlanar() || !vi.IsYUV() || vi.BitsPerComponent() != 32) {
        env->ThrowError("ConvertFromLChab: 32 bit float YUV444PS only!");
    }

    // Create output VideoInfo
    memset(&viOut, 0, sizeof(VideoInfo));
    viOut.width = vi.width;
    viOut.height = vi.height;
    viOut.fps_numerator = vi.fps_numerator;
    viOut.fps_denominator = vi.fps_denominator;
    viOut.num_frames = vi.num_frames;
    viOut.pixel_type = VideoInfo::CS_RGBPS;
    viOut.sample_type = SAMPLE_FLOAT;
    viOut.nchannels = vi.nchannels;
    viOut.audio_samples_per_second = vi.audio_samples_per_second;
    viOut.num_audio_samples = vi.num_audio_samples;
}


PVideoFrame __stdcall ConvertFromLChab::GetFrame(int n, IScriptEnvironment* env) {

    PVideoFrame src = child->GetFrame(n, env);

    PVideoFrame dst = env->NewVideoFrame(viOut);

    float* srcp[3];
    float* dstp[3];
    int src_pitch[3], dst_pitch[3], row_size[3], height[3];
    //int p, x, y;

    int planes[] = { PLANAR_R, PLANAR_G, PLANAR_B };
    int labPlanes[] = { PLANAR_Y, PLANAR_U, PLANAR_V };

    for (int p = 0; p < 3; p++) {
        srcp[p] = (float*)src->GetReadPtr(labPlanes[p]);
        dstp[p] = (float*)dst->GetWritePtr(planes[p]);

        src_pitch[p] = src->GetPitch(labPlanes[p]);
        dst_pitch[p] = dst->GetPitch(planes[p]);
        row_size[p] = dst->GetRowSize(planes[p]) / sizeof(float);
        height[p] = dst->GetHeight(planes[p]);
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

            PixelFromLChabToSRGB(srcpLocal[0][x], srcpLocal[1][x], srcpLocal[2][x], dstpLocal[0][x], dstpLocal[1][x], dstpLocal[2][x]);
        }

    }

    return dst;
}