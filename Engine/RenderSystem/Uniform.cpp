#include "Engine/RenderSystem/Uniform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "ThirdParty/OpenGL/glext.h"
#include "ThirdParty/OpenGL/wglext.h"

#include "Engine/Math/Vector2f.hpp"
#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Vector4f.hpp"
#include "Engine/Math/Matrix4f.hpp"


//-------------------------------------------------------------------------------------------------
Uniform::Uniform( Uniform const & copy, void * const data )
	: m_bindPoint( copy.m_bindPoint )
	, m_length( copy.m_length )
	, m_size( copy.m_size )
	, m_type( copy.m_type )
	, m_name( copy.m_name )
	, m_data( data )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Uniform::Uniform( int bindPoint, int length, int size, unsigned int type, std::string const & name, void * const data )
	: m_bindPoint( bindPoint )
	, m_length( length )
	, m_size( size )
	, m_type( type )
	, m_name( name )
	, m_data( data )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Uniform::~Uniform( )
{
	switch ( m_type )
	{
	case GL_INT:
	{
		int * deleteData = ( int* ) m_data;
		delete[] deleteData;
		break;
	}
	case GL_FLOAT:
	{
		float * deleteData = ( float* ) m_data;
		delete[] deleteData;
		break;
	}
	case GL_FLOAT_VEC2:
	{
		Vector2f * deleteData = ( Vector2f* ) m_data;
		delete[] deleteData;
		break;
	}
	case GL_FLOAT_VEC3:
	{
		Vector3f * deleteData = ( Vector3f* ) m_data;
		delete[] deleteData;
		break;
	}
	case GL_FLOAT_VEC4:
	{
		Vector4f * deleteData = ( Vector4f* ) m_data;
		delete[] deleteData;
		break;
	}
	case GL_FLOAT_MAT4:
	{
		Matrix4f * deleteData = ( Matrix4f* ) m_data;
		delete[] deleteData;
		break;
	}
	case GL_SAMPLER_2D:
	{
		size_t* deleteData = (size_t*) m_data;
		delete[] deleteData;
		break;
	}
	}
}
