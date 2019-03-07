#version 410 core
//Deals with Color

uniform sampler2D uTexDiffuse;

in vec4 passColor;
in vec2 passUV0;

out vec4 outColor;


//---------------------------------------------------------------------------------------------
void main( void )
{
	outColor = texture( uTexDiffuse, passUV0 ) * passColor;
}