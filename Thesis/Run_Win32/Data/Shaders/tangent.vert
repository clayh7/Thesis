#version 410 core
//Deals with Positions

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

in vec3 inPosition;
in vec4 inColor;
in vec3 inTangent;
in vec3 inBiTangent;

out vec4 passColor;
out vec3 passTangent;
out vec3 passBiTangent;


//---------------------------------------------------------------------------------------------
void main( void )
{
	passColor = inColor;
	passTangent = ( vec4( inTangent, 0.f ) * uModel ).xyz;
	passBiTangent = ( vec4( inBiTangent, 0.f ) * uModel ).xyz;
	
	vec4 pos = vec4( inPosition, 1.0f );
	gl_Position = uProj * uView * uModel * pos;
}