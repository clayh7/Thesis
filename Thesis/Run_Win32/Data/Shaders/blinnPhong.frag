#version 410 core
//Deals with Color

uniform sampler2D uDiffuseTex;
uniform sampler2D uNormalTex;
uniform sampler2D uSpecularTex;
uniform sampler2D uAOTex;
uniform sampler2D uEmissiveTex;
uniform float uEmissiveAlpha;

uniform vec4 uAmbientLight;

uniform int uLightCount;
uniform vec4 uLightColor[16];
uniform vec3 uLightPosition[16];
uniform vec3 uLightDirection[16];
uniform float uLightIsDirectional[16]; //1.f = true | 0.f = false
uniform float uLightDistanceMin[16];
uniform float uLightDistanceMax[16];
uniform float uLightStrengthAtMin[16];
uniform float uLightStrengthAtMax[16];
uniform float uLightInnerAngle[16];
uniform float uLightOuterAngle[16];
uniform float uLightStrengthInside[16];
uniform float uLightStrengthOutside[16];

uniform float uSpecularExponent;
uniform float uSpecularIntensity;

uniform vec4 uFogColor;
uniform float uFogMinDistance;
uniform float uFogMaxDistance;

uniform vec3 uCameraPosition;

in vec2 passUV0;
in vec3 passPosition;
in vec3 passTangent;
in vec3 passBiTangent;

out vec4 outColor;

/*
//---------------------------------------------------------------------------------------------
//Keep color between 0 and 1
vec3 ClampColor( vec3 color )
{
	return clamp( color, vec3( 0.0f ), vec3( 1.0f ) );
}


//---------------------------------------------------------------------------------------------
vec4 ColorFromVector( vec3 v )
{
	return vec4( ( v + vec3( 1.f ) ) * 0.5f, 1.f );
}


//---------------------------------------------------------------------------------------------
vec3 CalcAmbientLight( )
{
	return uAmbientLight.rgb * uAmbientLight.a;
}


//---------------------------------------------------------------------------------------------
float CalcAttenuation( int lightIndex )
{
	vec3 vectorToLight = uLightPosition[lightIndex] - passPosition[lightIndex];

	//Distance based Attenuation
	float distanceToLight = length( vectorToLight );
	float att = smoothstep( uLightDistanceMin[lightIndex], uLightDistanceMax[lightIndex], distanceToLight );
	att = uLightStrengthAtMin[lightIndex] * ( 1.f - att ) + uLightStrengthAtMax[lightIndex] * att;

	//Angle based Attenuation
	float angleToLight = dot( vectorToLight, -uLightDirection[lightIndex] );
	float att2 = smoothstep( uLightInnerAngle[lightIndex], uLightOuterAngle[lightIndex], angleToLight );
	att2 = uLightStrengthInside[lightIndex] * ( 1.f - att2 ) + uLightStrengthOutside[lightIndex] * att2;

	return att * att2;
}


//---------------------------------------------------------------------------------------------
vec3 CalcDot3Light( vec3 normal )
{
	vec3 dot3Light = vec3( 0.f );
	for( int lightIndex = 0; lightIndex < uLightCount; ++lightIndex )
	{
		float attenuation = CalcAttenuation( lightIndex );
		vec3 vectorToLightDirectional = -uLightDirection[lightIndex];
		vec3 vectorToLightPoint = uLightPosition[lightIndex] - passPosition;
		vec3 vectorToLight = vectorToLightDirectional * uLightIsDirectional[lightIndex] + vectorToLightPoint * ( 1.f - uLightIsDirectional[lightIndex] ); //No branching
		float amount = max( dot( vectorToLight, normal ), 0.f );
		vec3 light = uLightColor[lightIndex].rgb * uLightColor[lightIndex].a;
		dot3Light += light * amount * attenuation;
	}

	return dot3Light;
}


//---------------------------------------------------------------------------------------------
vec3 CalcEmissiveLight( )
{
	vec4 emissiveColor = texture( uEmissiveTex, passUV0 );
	return emissiveColor.rgb * emissiveColor.a;
}


//---------------------------------------------------------------------------------------------
vec3 CalcSpecularLight( vec3 normal )
{
	float specular = texture( uSpecularTex, passUV0 ).r;
	vec3 specLight = vec3( 0.f );
	for( int lightIndex = 0; lightIndex < uLightCount; ++lightIndex )
	{
		float attenuation = CalcAttenuation( lightIndex );
		vec3 vectorToLight = normalize( uLightPosition[lightIndex] - passPosition );
		vec3 vectorToEye = normalize( uCameraPosition - passPosition );
		vec3 halfVector = normalize( vectorToLight + vectorToEye );
		float amount = max( dot( halfVector, normal ), 0.f );
		amount = pow( amount, uSpecularExponent ) * attenuation * uSpecularIntensity;
		vec3 light = uLightColor[lightIndex].rgb * uLightColor[lightIndex].a;
		specLight += light * specular * amount;
	}

	return specLight;
}


//---------------------------------------------------------------------------------------------
float CalculateFogFactor( void )
{
	vec3 vectorToCamera = uCameraPosition - passPosition;
	float distanceFromPoint = length( vectorToCamera ); //Get Length of vectorToCamera
	float fog = ( ( distanceFromPoint - uFogMinDistance ) / ( uFogMaxDistance - uFogMinDistance ) ); //Amount / Range

	return clamp( fog, 0.f, 1.f ); //Return percentage to fog
}


//---------------------------------------------------------------------------------------------
void main( void )
{
	//Calculate Tangent - Bitangent - Normal
	vec3 surface_tangent = normalize( passTangent );
	vec3 surface_biTangent = normalize( passBiTangent );
	vec3 surface_normal = cross( surface_tangent, surface_biTangent );

	mat3 tbn = mat3( surface_tangent, surface_biTangent, surface_normal );
	tbn = transpose( tbn );

	//Maps normal to a range (-1 to 1, -1 to 1, 0 to -1)
	vec3 normal = texture( uNormalTex, passUV0 ).xyz;
	normal = normal * vec3( 2.0f, 2.0f, 1.0f) - vec3(1.0f, 1.0f, 0.0f);
	normal = normalize( normal ); //re-fix the normals (float error)
	normal = normal * tbn;

	vec3 diffuse = texture( uDiffuseTex, passUV0 ).rgb;
	float aoFactor = texture( uAOTex, passUV0 ).x;

	vec3 ambientLight = CalcAmbientLight( );
	vec3 dot3Light = CalcDot3Light( normal );
	vec3 emissiveLight = CalcEmissiveLight( );
	vec3 specularLight = CalcSpecularLight( normal );

	vec3 surfaceLight = ClampColor( ( ambientLight + dot3Light ) * aoFactor + emissiveLight );
	vec3 finalColor = ClampColor( ( diffuse * surfaceLight ) + specularLight );

	float fogAmount = CalculateFogFactor( );
	outColor = vec4( finalColor * ( 1 - fogAmount ) + uFogColor.rgb * ( fogAmount ), 1.f ); //Fading to Fog
	
	//vec3 vectorToLight = uLightPosition[0] - passPosition;
	//float dis = length(vectorToLight);
	//if(dis > uLightDistanceMin[0])
	//	outColor = vec4(vec3(0.f),1.f);
	//else
	//	outColor = vec4(1.f);
	//outColor = vec4(vec3(max(1.f - dis/uLightDistanceMin[0], 0.f)), 1.f);
}
*/

//---------------------------------------------------------------------------------------------
float CalculateFogFactor( void )
{
	vec3 vectorToCamera = uCameraPosition - passPosition;
	float distanceFromPoint = length( vectorToCamera ); //Get Length of vectorToCamera
	float fog = ( ( distanceFromPoint - uFogMinDistance ) / ( uFogMaxDistance - uFogMinDistance ) ); //Amount / Range

	return clamp( fog, 0.f, 1.f ); //Return percentage to fog
}

//------------------------------------------------------------------------
// Used for passing information back from my lighting equation
struct surface_light_t
{
   vec3 surface;
   vec3 specular;
};

//------------------------------------------------------------------------
vec4 VectorAsColor( vec3 vec ) 
{
	return vec4( (vec + vec3(1.0f)) * vec3(.5f), 1.0f );
}

//------------------------------------------------------------------------
surface_light_t CalculateSurfaceLight( vec3 surface_position, vec3 surface_normal,
   vec3 dir_to_eye, 
   float distance_to_eye,
   vec3 light_position, 
   vec3 light_direction,
   vec3 light_color,
   float light_dir_factor,
   float spec_power,
   float spec_intensity,
   float near_distance, float far_distance,
   float near_power, float far_power, 
   float inner_angle, float outer_angle,
   float inner_power, float outer_power ) 
{
   // Calculate some intermediate values, like the half vector
   vec3 vector_to_light = light_position - surface_position;
   float distance_to_light = length(vector_to_light);

   // so if light_dir_factor is 1 - our vector to light is the back along the lights direction
   // otherwise, it's the actual vector TO the light
   vec3 dir_to_light = mix( vector_to_light / distance_to_light, -light_direction, light_dir_factor ); 

   // Distance for "directional lights" is the planar distance, instead of the full vector distance
   distance_to_light = mix( distance_to_light, dot( vector_to_light, -light_direction ), light_dir_factor ); 

   vec3 half_vector = normalize(dir_to_light + dir_to_eye);
   float angle = dot( light_direction, -dir_to_light ); 

   // Calculate falloff due to cone angle & distance
   float distance_attenuation = mix( near_power, far_power, 
	  smoothstep( near_distance, far_distance, distance_to_light ) );

   float angle_attenuation = mix( inner_power, outer_power, 
	  smoothstep( inner_angle, outer_angle, angle ) );

   float attenuation = angle_attenuation * distance_attenuation;


   // Dot3 Lighting
   float dot3_factor = max( dot( surface_normal, dir_to_light ), 0.0f ) * attenuation;
   vec3 dot3_color = light_color * dot3_factor;


   // Specular Lighting
   float spec_factor = max( dot( surface_normal, half_vector ), 0.0f );
   spec_factor = pow( spec_factor, spec_power ) * spec_intensity * attenuation;
   vec3 spec_color = light_color * spec_factor;

   // Return it
   surface_light_t ret;
   ret.surface = dot3_color;
   ret.specular = spec_color;

   return ret;
}

//------------------------------------------------------------------------
void main()
{
   // Get surface information from textures
   vec4 diffuse = texture( uDiffuseTex, passUV0 );
	vec3 normal = texture( uNormalTex, passUV0 ).xyz;
	float ao = texture( uAOTex, passUV0 ).r;
   vec4 emissive = texture( uEmissiveTex, passUV0 );
   float spec_intensity = texture( uSpecularTex, passUV0 ).r;

   spec_intensity = spec_intensity * uSpecularIntensity;

   // Ambient is "World Light"
   // Emissive is Light emitted by the surface itself.
   vec3 ambient_color = uAmbientLight.rgb * uAmbientLight.a;
   vec3 emissive_color = emissive.rgb * emissive.a * uEmissiveAlpha;

   // Create the TBN matrix from passes tangent and bitangent
	vec3 surface_tan = normalize(passTangent);
	vec3 surface_bitangent = normalize(passBiTangent);
	vec3 surface_normal = cross( surface_tan, surface_bitangent );
	mat3 tbn = mat3( surface_tan, surface_bitangent, surface_normal );
	tbn = transpose( tbn );

	// Move the normal to world space
   normal = (normal * vec3(2.0f, 2.0f, 1.0f)) - vec3(1.0f, 1.0f, 0.0f);
   normal = normalize(normal);
	normal = normal * tbn;
	
   vec3 vector_to_eye = uCameraPosition - passPosition;
   float distance_to_eye = length(vector_to_eye);
   vec3 dir_to_eye = vector_to_eye / distance_to_eye;

   vec3 dot3_light = vec3(0.0f);
   vec3 spec_light = vec3(0.0f);

   // Go through each light, and add the light factor.
   for (int i = 0; i < uLightCount; ++i) {
	  surface_light_t light = CalculateSurfaceLight( passPosition, 
		 normal,
		 dir_to_eye,
		 distance_to_eye, 
		 uLightPosition[i],
		 uLightDirection[i],
		 uLightColor[i].rgb,
		 uLightIsDirectional[i],
		 uSpecularExponent,
		 spec_intensity,
		 uLightDistanceMin[i], uLightDistanceMax[i],
		 uLightStrengthAtMin[i], uLightStrengthAtMax[i],
		 uLightInnerAngle[i], uLightOuterAngle[i],
		 uLightStrengthInside[i], uLightStrengthOutside[i] );

	  dot3_light += light.surface;
	  spec_light += light.specular;
   }

   // The clamp is optional - but if you remove it, it allows your lights to "wash out" your surface
   // which *might* be an effect you want?
   vec3 surface_light = ao * (ambient_color + dot3_light) + emissive_color;

   surface_light = clamp( surface_light, vec3(0.0f), vec3(1.0f) );

   //vec3 reflect_eye = reflect( -vector_to_eye, normal );
   //vec3 reflect_color = texture( gTexSky, reflect_eye ).rgb;
   //vec4 surface_color = mix( diffuse, vec4(reflect_color, diffuse.a), spec_intensity ); 

   // Final lighting equation - add it all together, and clamp again to put us in our valid light range
   vec4 final_color = diffuse * vec4(surface_light, 1.0f) + vec4( spec_light, 0.0f );
   final_color = clamp( final_color, vec4(0.0f), vec4(1.0f) );

   float fogAmount = CalculateFogFactor( );
	final_color = final_color * ( 1 - fogAmount ) + uFogColor * ( fogAmount ); //Fading to Fog

   outColor = final_color;
   //outNormal = VectorAsColor(normal).xyz;
}
