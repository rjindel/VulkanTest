#version 450

layout (location = 0) in vec4 i_Position;

layout (binding = 1) uniform UBO
{
	vec4 u_Colour;
} ubo;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(location = 0) out vec4 v_Color;

void main()
{
	gl_Position = i_Position;
	v_Color = ubo.u_Colour;
}