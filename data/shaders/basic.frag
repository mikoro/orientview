#version 330

uniform sampler2D textureSampler;

in vec2 pass_textureCoord;

out vec3 out_color;

void main()
{
	out_color = texture(textureSampler, pass_textureCoord).rgb;
	out_color += vec3(0.5, 0.5, 0.5);
}
