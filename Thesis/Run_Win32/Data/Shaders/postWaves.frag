#version 410 core

uniform sampler2D uDiffuseTex;
uniform float uTime;

in vec2 passUV0;

out vec4 outColor;


//---------------------------------------------------------------------------------------------
vec4 ColorFromVector( vec3 v )
{
	return vec4( ( v + vec3( 1.f ) ) * 0.5f, 1.f );
}


//---------------------------------------------------------------------------------------------
void main()
{
	vec4 diffuse = texture( uDiffuseTex, passUV0 );

	vec2 offset = passUV0 - vec2( .5f );
	offset.x *= 16.f/9.f;
	float d = length( offset );
	vec2 normalizedOffset = offset / d;

	float waveAmplitude = 0.03f;
	float speedOfWave = -3.f;
	float widthOfWave = 25.f;
	float factor = waveAmplitude * cos( uTime * speedOfWave + d * widthOfWave );
	vec2 offset2 = normalizedOffset * factor;

	outColor = texture( uDiffuseTex, passUV0 + offset2 );
}