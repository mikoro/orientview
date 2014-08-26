#version 120

uniform mat4 vertexMatrix;

attribute vec2 vertexPosition;
attribute vec2 vertexTextureCoordinate;
attribute vec4 vertexColor;

varying vec2 textureCoordinate;
varying vec4 color;

void main()
{
	gl_Position = vertexMatrix * vec4(vertexPosition, 0.0, 1.0);
	textureCoordinate = vertexTextureCoordinate;
	color = vertexColor;
}
