#version 330

uniform sampler2D textureSampler;

in vec2 textureCoordinate;

out vec3 color;

void main()
{
	color = texture(textureSampler, textureCoordinate).rgb;
}
