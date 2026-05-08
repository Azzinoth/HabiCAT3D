@WorldMatrix@

uniform vec3 Center;
uniform float Radius;

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0) buffer DataBuffer
{
    vec4 data[];
};

layout(std430, binding = 1) buffer Triangles
{
    vec4 TriangleCentroids[];
};

layout(std430, binding = 2) buffer AnnotationBuffer
{
	int AnnotationID;
};

void main()
{
	vec3 WorldPosition = TriangleCentroids[gl_GlobalInvocationID.x].xyz;
	WorldPosition = (FEWorldMatrix * vec4(WorldPosition, 1.0)).xyz;
	
	if (distance(Center, WorldPosition) < Radius)
	{
		data[gl_GlobalInvocationID.x][0] = float(AnnotationID);
	
	//	if (bDeletionOccurred == 0)
	//		atomicAdd(bDeletionOccurred, 1);
	}
}