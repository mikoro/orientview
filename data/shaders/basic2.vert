#version 330

uniform mat4 matrix;

in vec4 position;
in vec4 color;

out vec4 pass_color;

void main()
{
	pass_color = color;
	gl_Position = matrix * position;
}
