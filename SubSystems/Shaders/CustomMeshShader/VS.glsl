#pragma once

static const char* const CustomMesh_VS = R"(
@In_Position@
@In_Normal@

layout (location = 7) in float FirstLayerData;
layout (location = 8) in float AdditionalLayerData;

@In_Color@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

uniform int HeatMapType;
uniform int HaveColor;

out VS_OUT
{
	vec2 UV;
	vec3 worldPosition;
	vec4 viewPosition;
	mat3 TBN;
	vec3 vertexNormal;
	float materialIndex;

	vec3 color;

	float FirstLayer;
	float AdditionalLayer;
} vs_out;

void main(void)
{
	//gl_Position = ProjectionMatrix * vec4(vPos, 1.0);

	vec4 worldPosition = FEWorldMatrix * vec4(FEPosition, 1.0);
	vs_out.worldPosition = worldPosition.xyz;
	vs_out.viewPosition = FEViewMatrix * worldPosition;
	gl_Position = FEProjectionMatrix * vs_out.viewPosition;

	vs_out.vertexNormal = normalize(vec3(FEWorldMatrix * vec4(FENormal, 0.0)));

	if (HaveColor == 1)
		vs_out.color = FEColor;

	vs_out.FirstLayer = FirstLayerData;
	vs_out.AdditionalLayer = AdditionalLayerData;
}
)";