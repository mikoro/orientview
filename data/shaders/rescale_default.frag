#version 120

uniform sampler2D textureSampler;

varying vec2 textureCoordinate;

void main()
{
	gl_FragColor = texture2D(textureSampler, textureCoordinate);
}
