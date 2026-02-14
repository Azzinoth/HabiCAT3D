@In_Position@
@In_UV@
@In_Normal@
@In_Instance_Data@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

#define MAX_MULTIPLE_CELL_INTERACTION_COUNT 256

uniform int HighlightedCellIndex[MAX_MULTIPLE_CELL_INTERACTION_COUNT];
uniform int SelectedCellIndex[MAX_MULTIPLE_CELL_INTERACTION_COUNT];

out vec3 normal;
out vec3 fragPosition;
out vec2 UV;
out float isHighlighted;
out float isSelected;

layout(std430, binding = 0) buffer PerInstanceColor {
    vec4 Colors[];
};
out vec4 instanceColor;

void main(void)
{
	isHighlighted = 0.0f;
	for (int i = 0; i < MAX_MULTIPLE_CELL_INTERACTION_COUNT; i++)
	{
		if (HighlightedCellIndex[i] == gl_InstanceID)
		{
			isHighlighted = 1.0f;
			break;
		}
	}

	isSelected = 0.0f;
	for (int i = 0; i < MAX_MULTIPLE_CELL_INTERACTION_COUNT; i++)
	{
		if (SelectedCellIndex[i] == gl_InstanceID)
		{
			isSelected = 1.0f;
			break;
		}
	}
	 
	UV = FETexCoord;
	normal = normalize(mat3(transpose(inverse(FEInstanceData))) * FENormal);
	fragPosition = vec3(FEInstanceData * vec4(FEPosition, 1.0));

	instanceColor = Colors[gl_InstanceID];

	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * FEInstanceData * vec4(FEPosition, 1.0);
}