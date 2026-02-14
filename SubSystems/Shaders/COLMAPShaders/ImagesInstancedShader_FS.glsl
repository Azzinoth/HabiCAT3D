in vec3 normal;
in vec3 fragPosition;
in vec2 UV;
in float isHighlighted;
in float isSelected;

@MaterialTextures@
out vec4 out_Color;

uniform vec3 baseColor;
@CameraPosition@

in vec4 instanceColor;

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

void main(void)
{
	vec3 lightDirection = normalize(vec3(0.0, 1.0, 0.2));

	float diffuseFactor = max(dot(normal, lightDirection), 0.15);
	vec3 diffuseColor = diffuseFactor * vec3(2.0, 2.0, 2.0);
	vec3 ambientColor = vec3(0.55f, 0.73f, 0.87f) * 0.8f;
	
	vec3 ColorToUse = baseColor;
	vec4 TexureColor = texture(textures[textureBindings[0]], UV);
	if (TexureColor != vec4(0.0))
		ColorToUse = vec3(TexureColor);
	
	out_Color = vec4(ColorToUse * ambientColor * diffuseColor * 3.3, 1.0f);

	float SaturationFactor = 1.6f;
	float BrightnessFactor = 1.5f;

	if (isHighlighted > 0.0f || isSelected > 0.0f)
	{
		if (isSelected > 0.0f)
		{
			SaturationFactor *= 1.6f;
			BrightnessFactor *= 1.5f;
		}

		vec3 HSV = ConvertRGBToHSV(out_Color.rgb);
		HSV.y *= SaturationFactor;
		HSV.z *= BrightnessFactor;

		HSV.y = clamp(HSV.y, 0.0, 1.0);
		HSV.z = clamp(HSV.z, 0.0, 1.0);

		out_Color.rgb = ConvertHSVToRGB(HSV);
	}

	out_Color.rgb = instanceColor.rgb;
}