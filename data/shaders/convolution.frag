#version 330

uniform sampler2D textureSampler;
uniform float textureWidth;
uniform float textureHeight;
uniform float texelWidth;
uniform float texelHeight;

in vec2 textureCoordinate;

out vec3 color;

void main()
{
	vec4 colors[9];
	int index = 0;
	
	for(int y = -1; y <= 1; y++)
    {
        for(int x = -1; x <= 1; x++)
        {
			colors[index++] = texture(textureSampler, textureCoordinate + vec2(texelWidth * float(x), texelHeight * float(y)));
        }
    }
    
    // blur
    // 1 2 1
	// 2 1 2
	// 1 2 1
	float sum1 = 13.0f;
	vec4 color1 = colors[0] * 1.0f + colors[1] * 2.0f + colors[2] * 1.0f +
				  colors[3] * 2.0f + colors[4] * 1.0f + colors[5] * 2.0f +
				  colors[6] * 1.0f + colors[7] * 2.0f + colors[8] * 1.0f;
	// sharpen
	// -1 -1 -1
	// -1  9 -1
	// -1 -1 -1
	float sum2 = 1.0f;
	vec4 color2 = colors[0] * -1.0f + colors[1] * -1.0f + colors[2] * -1.0f +
				  colors[3] * -1.0f + colors[4] *  9.0f + colors[5] * -1.0f +
				  colors[6] * -1.0f + colors[7] * -1.0f + colors[8] * -1.0f;
	
	color = color1.rgb / sum1;
}
