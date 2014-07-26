#version 330

uniform mat4 vertexMatrix;
uniform mat4 textureMatrix;

in vec2 vertexCoord;
in vec2 textureCoord;

out vec2 pass_textureCoord;

void main()
{
	gl_Position = vertexMatrix * vec4(vertexCoord, 0.0, 1.0);
	pass_textureCoord = (textureMatrix * vec4(textureCoord, 0.0, 1.0)).st;
}
