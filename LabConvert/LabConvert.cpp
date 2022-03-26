#include "LabConvert.h"


// Most of the LAB color conversion code is lifted and adapted from ColorMinePortable


// sRGB to XYZ matrix:
// 
// 0.4124564  0.3575761  0.1804375
// 0.2126729  0.7151522  0.0721750
// 0.0193339  0.1191920  0.9503041
//
// XYZ to sRGB matrix:
// 
// 3.2404542 -1.5371385 -0.4985314
// -0.9692660  1.8760108  0.0415560
// 0.0556434 - 0.2040259  1.0572252
//



AVSValue __cdecl Create_ConvertToLChab(AVSValue args, void* user_data, IScriptEnvironment* env) {

    return new ConvertToLChab(args[0].AsClip(), env);
}

AVSValue __cdecl Create_ConvertFromLChab(AVSValue args, void* user_data, IScriptEnvironment* env) {

    return new ConvertFromLChab(args[0].AsClip(), env);
}

const AVS_Linkage* AVS_linkage = 0;

extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment * env, const AVS_Linkage* const vectors) {
    AVS_linkage = vectors;
    env->AddFunction("ConvertToLChab", "c", Create_ConvertToLChab, 0);
    env->AddFunction("ConvertFromLChab", "c", Create_ConvertFromLChab, 0);
    return "Color matrix transform";
}