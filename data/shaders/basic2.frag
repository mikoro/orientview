#version 330 core

varying vec4 pass_color;

void main()
{
	gl_FragColor = pass_color;
}
