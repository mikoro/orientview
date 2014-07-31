#version 330

uniform mat4 vertexMatrix;

in vec3 vertexPosition;
in vec2 vertexTextureCoordinate;

out vec2 textureCoordinate;

void main()
{
	gl_Position = vertexMatrix * vec4(vertexPosition, 1.0);
	textureCoordinate = vertexTextureCoordinate;
}
