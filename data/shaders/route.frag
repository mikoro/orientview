#version 120

uniform vec4 borderColor;
uniform float borderRelativeWidth;

varying vec2 textureCoordinate;
varying vec4 color;

void main()
{
	//if (1.0 - abs(textureCoordinate.x) < borderRelativeWidth)
	//	gl_FragColor = borderColor;
	//else
	//	gl_FragColor = color;
	
	gl_FragColor = color;
	gl_FragColor.a = 0.5;
}
