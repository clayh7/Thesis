#version 410 core

uniform sampler2D uDiffuseTex;

in vec2 passUV0;
//in float passDepth;

out vec4 outColor;


//---------------------------------------------------------------------------------------------
vec4 ColorFromVector( vec3 v )
{
	return vec4( ( v + vec3( 1.f ) ) * 0.5f, 1.f );
}


//---------------------------------------------------------------------------------------------
vec2 GetScreenLocation( vec2 uv )
{
	float aspect = 16.f/9.f;
	return vec2( uv.x * aspect, uv.y );
}


//---------------------------------------------------------------------------------------------
vec2 GetUVLocation( vec2 screenPos )
{
	float aspect = 16.f/9.f;
	return vec2( screenPos.x / aspect, screenPos.y );
}


//---------------------------------------------------------------------------------------------
void main() 
{ 
	outColor = texture( uDiffuseTex, passUV0 );
}