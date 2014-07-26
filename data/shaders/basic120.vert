#version 120

uniform mat4 vertexMatrix;
uniform mat4 textureMatrix;

attribute vec2 vertexCoord;
attribute vec2 textureCoord;

varying vec2 pass_textureCoord;

void main()
{
	gl_Position = vertexMatrix * vec4(vertexCoord, 0.0, 1.0);
	pass_textureCoord = (textureMatrix * vec4(textureCoord, 0.0, 1.0)).st;
}
