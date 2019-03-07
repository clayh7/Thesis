#version 410 core
//Deals with Positions

uniform mat4 uModel;	//Related to the mesh you're drawing, updated every frame
uniform mat4 uView;		//View of the camera
uniform mat4 uProj;		//Perspective / Orthographic

in vec2 inUV0;
in vec3 inPosition;
in vec3 inTangent;
in vec3 inBiTangent;

out vec2 passUV0;
out vec3 passPosition;
out vec3 passTangent;
out vec3 passBiTangent;

void main( void )
{
	passUV0 = inUV0;

	vec4 pos = vec4( inPosition, 1.0f );
	passPosition = ( uModel * pos ).xyz; //Get world position
	passTangent = ( vec4( inTangent, 0.f ) * uModel ).xyz; //Transform tangent into local space
	passBiTangent = ( vec4( inBiTangent, 0.f ) * uModel ).xyz; //Transform tangent into local space

	//Matrix Math
	pos = uProj * uView * uModel * pos;
	
	//Set Positions
	gl_Position = pos;
}