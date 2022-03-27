#pragma once
#include <windows.h>
#include "avisynth.h"

class ConvertToLChab : public GenericVideoFilter {
    inline void PixelFromSRGBToLChab(float& r, float& g, float& b, float& l, float& c, float& hab);
    VideoInfo viOut;
    inline float PivotXyz(float n);
    const float Epsilon = 0.008856f; // Intent is 216/24389
    const float Kappa = 903.3f; // Intent is 24389/27
    const float WhiteReference[3] = { 95.047f,100.000f, 108.883f };
public:
    ConvertToLChab(PClip _child, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    const VideoInfo& __stdcall GetVideoInfo() { return viOut; }
};

class ConvertFromLChab : public GenericVideoFilter {
    inline void PixelFromLChabToSRGB(float& l, float& c, float& hab,float& r, float& g, float& b);
    VideoInfo viOut;
    const float Epsilon = 0.008856f; // Intent is 216/24389
    const float Kappa = 903.3f; // Intent is 24389/27
    const float WhiteReference[3] = { 95.047f,100.000f, 108.883f };
public:
    ConvertFromLChab(PClip _child, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    const VideoInfo& __stdcall GetVideoInfo() { return viOut; }
};


// Oklab variants
class ConvertToOkLChab : public GenericVideoFilter {
    inline void PixelFromSRGBToOkLChab(float& r, float& g, float& b, float& l, float& c, float& hab);
    VideoInfo viOut;
public:
    ConvertToOkLChab(PClip _child, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    const VideoInfo& __stdcall GetVideoInfo() { return viOut; }
};

class ConvertFromOkLChab : public GenericVideoFilter {
    inline void PixelFromOkLChabToSRGB(float& l, float& c, float& hab, float& r, float& g, float& b);
    VideoInfo viOut;
public:
    ConvertFromOkLChab(PClip _child, IScriptEnvironment* env);
    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
    const VideoInfo& __stdcall GetVideoInfo() { return viOut; }
};