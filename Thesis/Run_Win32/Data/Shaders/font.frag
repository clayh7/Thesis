#version 410 core
//Deals with Color

uniform sampler2D uFontTex;
uniform vec4 uColor;

in vec4 passColor;
in vec2 passUV0;

out vec4 outColor;


//---------------------------------------------------------------------------------------------
void main( void )
{
	outColor = texture( uFontTex, passUV0 ) * uColor;
}