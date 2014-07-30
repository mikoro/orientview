#version 330

uniform mat4 vertexMatrix;
uniform float textureWidth;
uniform float textureHeight;
uniform float texelWidth;
uniform float texelHeight;

in vec3 vertexPosition;
in vec2 vertexTextureCoordinate;

out vec2 textureCoordinate;

void main()
{
	gl_Position = vertexMatrix * vec4(vertexPosition, 1.0);
	textureCoordinate = vertexTextureCoordinate;
}
