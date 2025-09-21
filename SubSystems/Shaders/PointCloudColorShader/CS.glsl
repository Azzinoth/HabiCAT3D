#pragma once

static const char* const PointCloudRecoloringShader_CS = R"(

uniform float LayerAbsoluteMin;
uniform float LayerAbsoluteMax;
uniform float LayerMinValue;
uniform float LayerMaxValue;
uniform float SelectedRangeMin;
uniform float SelectedRangeMax;

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct PointData
{
    vec3 position;
    uint color;
};

layout (std430, binding = 1) buffer ParticleBuffer
{
    PointData particles[];
};

layout (std430, binding = 2) readonly buffer LayerValueBuffer
{
    float LayerValues[];
};

layout (std430, binding = 3) readonly buffer ColormapBuffer
{
    float TurboColormap[];
};

// Copyright 2019 Google LLC.
// SPDX-License-Identifier: Apache-2.0

// Author: Anton Mikhailov

// The look-up tables contains 256 entries. Each entry is a an sRGB triplet.
// From : https://gist.github.com/mikhailov-work/6a308c20e494d9e0ccc29036b28faa7a#file-turbo_colormap-c
vec3 GetTurboColormapValue(float factor)
{
    int index = int(255 * factor);
    return vec3(TurboColormap[index * 3], TurboColormap[index * 3 + 1], TurboColormap[index * 3 + 2]);
}

vec3 ConvertRGBToHSV(vec3 RGBColor)
{
	vec4 ConversionConstants = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 BlueGreenMix = mix(vec4(RGBColor.bg, ConversionConstants.wz), vec4(RGBColor.gb, ConversionConstants.xy), step(RGBColor.b, RGBColor.g));
	vec4 MaxColorMix = mix(vec4(BlueGreenMix.xyw, RGBColor.r), vec4(RGBColor.r, BlueGreenMix.yzx), step(BlueGreenMix.x, RGBColor.r));

	float Chroma = MaxColorMix.x - min(MaxColorMix.w, MaxColorMix.y);
	float Epsilon = 1.0e-10;

	float Hue = abs(MaxColorMix.z + (MaxColorMix.w - MaxColorMix.y) / (6.0 * Chroma + Epsilon));
	float Saturation = Chroma / (MaxColorMix.x + Epsilon);
	float Value = MaxColorMix.x;

	return vec3(Hue, Saturation, Value);
}

vec3 ConvertHSVToRGB(vec3 HSVColor)
{
    vec4 RGBConstants = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);

    vec3 HueShifts = abs(fract(HSVColor.xxx + RGBConstants.xyz) * 6.0 - RGBConstants.www);
    return HSVColor.z * mix(RGBConstants.xxx, clamp(HueShifts - RGBConstants.xxx, 0.0, 1.0), HSVColor.y);
}

void main()
{
	float NormalizedValue = (LayerValues[gl_GlobalInvocationID.x] - LayerMinValue) / (LayerMaxValue - LayerMinValue);
	NormalizedValue = clamp(NormalizedValue, 0, 1);

	vec3 ColorToUse = GetTurboColormapValue(NormalizedValue);


	float UnselectedAreaSaturationFactor = 0.3f;
	float UnselectedAreaBrightnessFactor = 0.2f;

	float NormalizedAbsoluteValue = (LayerValues[gl_GlobalInvocationID.x] - LayerAbsoluteMin) / (LayerAbsoluteMax - LayerAbsoluteMin);
	NormalizedAbsoluteValue = clamp(NormalizedAbsoluteValue, 0, 1);

	if (SelectedRangeMin != 0.0 || SelectedRangeMax != 0.0)
	{
		if (NormalizedAbsoluteValue >= SelectedRangeMin &&
			NormalizedAbsoluteValue <= SelectedRangeMax)
		{

		}
		else
		{
			// Convert RGB to HSV
			vec3 HSV = ConvertRGBToHSV(ColorToUse);

			// Adjust saturation and brightness (value)
			HSV.y *= UnselectedAreaSaturationFactor;
			HSV.z *= UnselectedAreaBrightnessFactor;

			// Clamp the saturation and brightness components
			HSV.y = clamp(HSV.y, 0.0, 1.0);
			HSV.z = clamp(HSV.z, 0.0, 1.0);

			// Convert back to RGB
			ColorToUse = ConvertHSVToRGB(HSV);
		}
	}






	uint R = uint(ColorToUse.x * 255);
	uint G = uint(ColorToUse.y * 255);
	uint B = uint(ColorToUse.z * 255);
	uint A = 255;

	particles[gl_GlobalInvocationID.x].color = (R << 0) | (G << 8) | (B << 16) | (A << 24);
}
)";