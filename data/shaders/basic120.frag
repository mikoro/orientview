#version 120

uniform sampler2D textureSampler;

varying vec2 pass_textureCoord;

void main()
{
	gl_FragColor = texture2D(textureSampler, pass_textureCoord);
}
