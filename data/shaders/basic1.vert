#version 110

attribute vec4 position;
attribute vec4 color;
uniform mat4 matrix;
varying vec4 pass_color;

void main()
{
	pass_color = color;
	gl_Position = matrix * position;
}
