#pragma once

static const char* const CustomMesh_VS = R"(
@In_Position@
@In_Normal@

layout (location = 7) in float FirstLayerData;
layout (location = 8) in float AdditionalLayerData;

layout (location = 9) in vec4 BulkLayerData_0;
layout (location = 10) in vec4 BulkLayerData_1;
layout (location = 11) in vec4 BulkLayerData_2;
layout (location = 12) in vec4 BulkLayerData_3;
layout (location = 13) in vec4 BulkLayerData_4;
layout (location = 14) in vec4 BulkLayerData_5;

@In_Color@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

uniform int HeatMapType;
uniform int HaveColor;

uniform int InterpolationActive;

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

	vec4 BulkLayers[6];
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

    if (InterpolationActive == 1)
    {
        vs_out.BulkLayers[0] = BulkLayerData_0;
        vs_out.BulkLayers[1] = BulkLayerData_1;
        vs_out.BulkLayers[2] = BulkLayerData_2;
        vs_out.BulkLayers[3] = BulkLayerData_3;
        vs_out.BulkLayers[4] = BulkLayerData_4;
        vs_out.BulkLayers[5] = BulkLayerData_5;
    }
}
)";