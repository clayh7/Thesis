#version 410 core

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

in vec3 inPosition;
in vec2 inUV0;

out vec2 passUV0;
out float passDepth;

void main( void )
{
	passUV0 = vec2( inUV0.x, 1.f - inUV0.y );

	vec4 pos = vec4( inPosition, 1.0f );
	pos = uProj * uView * uModel * pos;

	gl_Position = pos;

	//---------------------------------------------------------------------------------------------
	//float near_z = 0.1f;
	//float far_z = 100.f;
	// For calculating W Buffering Parameters - C++ Side
	//float q = 1.0f / (far_z - near_z);
	//vec2 w_buffer_params = vec2( 2.0f * q, -(far_z + near_z) * q );

	///////////////////////////////////////////////////////////////////////////////////////
	// Vertex Shader Side - w_buffer_params is passed up to gWBufferParams

	// Transform into Clip Space
	//vec4 clip_space_position = uProj * uView * uModel * pos;

	// clip_space_position.w is the Z distance from camera at this point
	// Dotting it with the w_buffer_params will map it from [near, far] to [-1, 1].
	//float linear_z = dot( w_buffer_params, vec2( clip_space_position.w, 1.0f ) );

	// Last, set it as our Z, multiplied by clip_space_position.w (which was divided out by the 
	// hardware again when it goes to NDC space.  This causes it to be output to our depth
	//clip_space_position.z = linear_z * clip_space_position.w;
	//passDepth = ( linear_z + 1.f ) / 2.f;

	// Finally, output it.
	//gl_Position = clip_space_position;
}