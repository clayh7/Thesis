#version 410 core
//Deals with Positions

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

in vec3 inPosition;
in vec4 inColor;

out vec4 passColor;


//---------------------------------------------------------------------------------------------
void main( void )
{
	passColor = inColor;
	
	vec4 pos = vec4( inPosition, 1.0f );
	gl_Position = uProj * uView * uModel * pos;
}