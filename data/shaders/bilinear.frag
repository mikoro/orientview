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
	// round up to the nearest texel center (this avoids hardware bilinear)
	float tx = textureCoordinate.x * textureWidth;
	tx = ceil(tx + 0.5f) - 0.5f;

	float ty = textureCoordinate.y * textureHeight;
	ty = ceil(ty + 0.5f) - 0.5f;

	vec2 snappedTextureCoordinate = vec2(tx / textureWidth, ty / textureHeight);

	// take color samples from four nearest texel centers
	vec4 tl = texture(textureSampler, snappedTextureCoordinate);
	vec4 tr = texture(textureSampler, snappedTextureCoordinate + vec2(texelWidth, 0));
	vec4 bl = texture(textureSampler, snappedTextureCoordinate + vec2(0, texelHeight));
	vec4 br = texture(textureSampler, snappedTextureCoordinate + vec2(texelWidth, texelHeight));

	float alphaX = fract(textureCoordinate.x * textureWidth);
	float alphaY = fract(textureCoordinate.y * textureHeight);
	
	// remap alpha 0.5 1.0 0.5 -> 0.0 0.5 1.0
	// the snapping makes this necessary
	alphaX += (alphaX > 0.5f) ? -0.5f : 0.5f;
	alphaY += (alphaY > 0.5f) ? -0.5f : 0.5f;
	
	// better looking color gradients at the edges
	alphaX = smoothstep(0.0f, 1.0f, alphaX);
	alphaY = smoothstep(0.0f, 1.0f, alphaY);
	
	vec4 top = mix(tl, tr, alphaX);
	vec4 bottom = mix(bl, br, alphaX);
	color = mix(top, bottom, alphaY).rgb;
}
