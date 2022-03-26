#include "LabConvert.h"
#include <cmath>


inline float ConvertToLChab::PivotXyz(float n)
{
    return n > Epsilon ? std::cbrtf(n) : (Kappa * n + 16.0f) / 116.0f;
}

inline void ConvertToLChab::PixelFromSRGBToLChab(float& r, float& g, float& b, float& l, float& c, float& hab) {

    float x, y, z;
    float L, A, B;
    //sRGBToXYZ
    // sRGB to XYZ matrix:
    // 
    // 0.4124564  0.3575761  0.1804375
    // 0.2126729  0.7151522  0.0721750
    // 0.0193339  0.1191920  0.9503041
    //
    x = r * 0.4124564f + g * 0.3575761f + b * 0.1804375f;
    y = r * 0.2126729f + g * 0.7151522f + b * 0.0721750f;
    z = r * 0.0193339f + g * 0.1191920f + b * 0.9503041f;

    // XYZ to LAB
    x = PivotXyz(x / WhiteReference[0]);
    y = PivotXyz(y / WhiteReference[1]);
    z = PivotXyz(z / WhiteReference[2]);
    L = 116.0f * y - 16.0f;
    A = 500.0f * (x - y);
    B = 200.0f * (y - z);

    l = L;
    //c = std::sqrtf(std::powf(A, 2) + std::powf(B, 2));
    c = std::sqrtf(A*A + B*B);
    hab = std::atan2(B, A);
}


ConvertToLChab::ConvertToLChab(PClip _child, IScriptEnvironment* env) :
    GenericVideoFilter(_child) {
    if (!vi.IsPlanar() || !vi.IsRGB() || vi.BitsPerComponent() != 32) {
        env->ThrowError("ConvertToLChab: 32 bit float RGBPS only!");
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


PVideoFrame __stdcall ConvertToLChab::GetFrame(int n, IScriptEnvironment* env) {

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

            PixelFromSRGBToLChab(srcpLocal[0][x], srcpLocal[1][x], srcpLocal[2][x], dstpLocal[0][x], dstpLocal[1][x], dstpLocal[2][x]);
        }

    }

    return dst;
}