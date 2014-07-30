#version 120

uniform mat4 vertexMatrix;

attribute vec3 vertexPosition;
attribute vec2 vertexTextureCoordinate;

varying vec2 textureCoordinate;

void main()
{
	gl_Position = vertexMatrix * vec4(vertexPosition, 1.0);
	textureCoordinate = vertexTextureCoordinate;
}
