#version 410 core
//Deals with Color

uniform vec4 uColor;

in vec4 passColor;

out vec4 outColor;


//---------------------------------------------------------------------------------------------
void main( void )
{
	outColor = passColor * uColor;
}