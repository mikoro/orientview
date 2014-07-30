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
	vec4 tl = texture(textureSampler, textureCoordinate);
    vec4 tr = texture(textureSampler, textureCoordinate + vec2(texelWidth, 0));
    vec4 bl = texture(textureSampler, textureCoordinate + vec2(0, texelHeight));
    vec4 br = texture(textureSampler, textureCoordinate + vec2(texelWidth, texelHeight));
    
    float alpha = fract(textureCoordinate.x * (textureWidth + 0.0f));
    alpha = smoothstep(0.0f, 1.0f, alpha);
    
    vec4 top = mix(tl, tr, alpha);
    vec4 bottom = mix(bl, br, alpha);
    
    alpha = fract(textureCoordinate.y * (textureHeight + 0.0f));
    alpha = smoothstep(0.0f, 1.0f, alpha);
    
    vec4 middle = mix(top, bottom, alpha);
    
	color = middle.rgb;
}
