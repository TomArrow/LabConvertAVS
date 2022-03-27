#include "LabConvert.h"
#include <cmath>

// The color conversion code/logic is lifted and adapted from https://bottosson.github.io/posts/oklab/

inline void ConvertFromOkLChab::PixelFromOkLChabToSRGB(float& l, float& c, float& hab, float& r, float& g, float& b) {

    float L, A, B;
    float l0, m, s;

    // LChab to LAB
    L = l;
    A = c * std::cosf(hab);
    B = c * std::sinf(hab);

    // LAB to XYZ
    l0 = L + 0.3963377774f * A + 0.2158037573f * B;
    m = L - 0.1055613458f * A - 0.0638541728f * B;
    s = L - 0.0894841775f * A - 1.2914855480f * B;

    l0 = l0 * l0 * l0;
    m = m * m * m;
    s = s * s * s;

    r = +4.0767416621f * l0 - 3.3077115913f * m + 0.2309699292f * s;
    g = -1.2684380046f * l0 + 2.6097574011f * m - 0.3413193965f * s;
    b = -0.0041960863f * l0 - 0.7034186147f * m + 1.7076147010f * s;

}


ConvertFromOkLChab::ConvertFromOkLChab(PClip _child, IScriptEnvironment* env) :
    GenericVideoFilter(_child) {
    if (!vi.IsPlanar() || !vi.IsYUV() || vi.BitsPerComponent() != 32) {
        env->ThrowError("ConvertFromOkLChab: 32 bit float YUV444PS only!");
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


PVideoFrame __stdcall ConvertFromOkLChab::GetFrame(int n, IScriptEnvironment* env) {

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

            PixelFromOkLChabToSRGB(srcpLocal[0][x], srcpLocal[1][x], srcpLocal[2][x], dstpLocal[0][x], dstpLocal[1][x], dstpLocal[2][x]);
        }

    }

    return dst;
}