#version 410 core
//Deals with Color

in vec4 passColor;
in vec3 passTangent;
in vec3 passBiTangent;

out vec4 outColor;


//---------------------------------------------------------------------------------------------
vec4 ColorFromVector( vec3 v )
{
	return vec4( ( v + vec3( 1.f ) ) * 0.5f, 1.f );
}


//---------------------------------------------------------------------------------------------
void main( void )
{
	vec3 surface_tangent = normalize( passTangent );
	vec3 surface_biTangent = normalize( passBiTangent );
	vec3 surface_normal = cross( surface_tangent, surface_biTangent );

	outColor = ColorFromVector( surface_normal );
}