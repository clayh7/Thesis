#include "Engine/RenderSystem/Material.hpp"

#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/RenderSystem/Attribute.hpp"
#include "Engine/RenderSystem/Uniform.hpp"
#include "Engine/RenderSystem/Texture.hpp"
#include "Engine/RenderSystem/ShaderProgram.hpp"
#include "Engine/Math/Matrix4f.hpp"


//-------------------------------------------------------------------------------------------------
STATIC char const * Material::DEFAULT_VERT = "Data/Shaders/passthrough.vert";
STATIC char const * Material::DEFAULT_FRAG = "Data/Shaders/passthrough.frag";

STATIC Material const * Material::DEFAULT_MATERIAL = nullptr;
STATIC Material * Material::FONT_MATERIAL = nullptr;


//-------------------------------------------------------------------------------------------------
void Material::InitializeDefaultMaterials( )
{
	DEFAULT_MATERIAL = new Material( );
	FONT_MATERIAL = new Material( "Data/Shaders/font.vert", "Data/Shaders/font.frag" );
	FONT_MATERIAL->SetUniform( "uFontTex", "Data/Fonts/ConsoleFont.png" );
	FONT_MATERIAL->SetUniform( "uColor", Color::WHITE );
}


//-------------------------------------------------------------------------------------------------
void Material::DestroyDefaultMaterials( )
{
	delete DEFAULT_MATERIAL;
	DEFAULT_MATERIAL = nullptr;

	delete FONT_MATERIAL;
	FONT_MATERIAL = nullptr;
}


//-------------------------------------------------------------------------------------------------
Material::Material( )
	: Material( ShaderProgram::CreateOrGetShaderProgram( DEFAULT_VERT, DEFAULT_FRAG ) )
{
	SetUniform( "uColor", Color::WHITE );
}


//-------------------------------------------------------------------------------------------------
Material::Material( std::string const &vsFilePath, std::string const &fsFilePath )
	: Material( ShaderProgram::CreateOrGetShaderProgram( vsFilePath, fsFilePath ) )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Material::Material( ShaderProgram const *program )
	: m_samplerID( 0 )
	, m_program( program )
{
	//Create Sampler
	m_samplerID = g_RenderSystem->CreateSampler( GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT );

	//Attribute and Uniform count
	GLuint progID = m_program->GetShaderProgramID( );

	//Get Attribute information
	int attributeCount;
	glGetProgramiv( progID, GL_ACTIVE_ATTRIBUTES, &attributeCount );
	for ( int aIndex = 0; aIndex < attributeCount; ++aIndex )
	{
		GLsizei length;
		GLint size;
		GLenum type;
		GLchar name[100];
		glGetActiveAttrib( progID, aIndex, sizeof( name ), &length, &size, &type, name );
		GLint loc = glGetAttribLocation( progID, name );
		ASSERT_RECOVERABLE( loc >= 0, "Can't find bind point for Uniform." );

		std::string aName( name );
		std::size_t foundIndex = aName.find( '[' );

		if ( foundIndex != std::string::npos )
		{
			aName = aName.substr( 0, foundIndex );
		}

		size_t attributeHash = std::hash<std::string>{ }( aName );
		switch ( type )
		{
		case GL_INT:
		{
			int * temp = new int[size];
			m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
			break;
		}
		case GL_FLOAT:
		{
			float * temp = new float[size];
			m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
			break;
		}
		case GL_FLOAT_VEC2:
		{
			Vector2f * temp = new Vector2f[size];
			m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
			break;
		}
		case GL_FLOAT_VEC3:
		{
			Vector3f * temp = new Vector3f[size];
			m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
			break;
		}
		case GL_FLOAT_VEC4:
		{
			Vector4f * temp = new Vector4f[size];
			m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
			break;
		}
		case GL_INT_VEC4:
		{
			Vector4i * temp = new Vector4i[size];
			m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
			break;
		}
		//#TODO: add Matrix case
		case GL_SAMPLER_2D:
			unsigned int * temp = new unsigned int( 0 );
			m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
			break;
		}
	}

	//Get Uniform information
	int uniformCount;
	glGetProgramiv( progID, GL_ACTIVE_UNIFORMS, &uniformCount );
	for ( int uIndex = 0; uIndex < uniformCount; ++uIndex )
	{
		GLsizei length;
		GLint size;
		GLenum type;
		GLchar name[100];
		glGetActiveUniform( progID, uIndex, sizeof(name), &length, &size, &type, name );
		GLint loc = glGetUniformLocation( progID, name );
		ASSERT_RECOVERABLE( loc >= 0, "Can't find bind point for Uniform." );

		std::string uName( name );
		std::size_t foundIndex = uName.find( '[' );

		if ( foundIndex != std::string::npos )
		{
			uName = uName.substr( 0, foundIndex );
		}

		switch ( type )
		{
		case GL_INT:
		{
			int * temp = new int[size];
			m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
			break;
		}
		case GL_FLOAT:
		{
			float * temp = new float[size];
			m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
			break;
		}
		case GL_FLOAT_VEC2:
		{
			Vector2f * temp = new Vector2f[size];
			m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
			break;
		}
		case GL_FLOAT_VEC3:
		{
			Vector3f * temp = new Vector3f[size];
			m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
			break;
		}
		case GL_FLOAT_VEC4:
		{
			Vector4f * temp = new Vector4f[size];
			m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
			break;
		}
		case GL_INT_VEC4:
		{
			Vector4i * temp = new Vector4i[size];
			m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
			break;
		}
		case GL_FLOAT_MAT4:
		{
			Matrix4f * temp = new Matrix4f[size];
			m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
			break;
		}
		case GL_SAMPLER_2D:
			m_uniforms[uName] = new Uniform( loc, length, size, type, uName, new unsigned int( 0 ) );
			break;
		}
	}
}


//-------------------------------------------------------------------------------------------------
Material::~Material( )
{
	for ( auto deleteUniform : m_uniforms )
	{
		delete deleteUniform.second;
		deleteUniform.second = nullptr;
	}

	for ( auto deleteMe : m_attributes )
	{
		delete deleteMe.second;
		deleteMe.second = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
std::map<size_t, Attribute*> const & Material::GetAttributeList( ) const
{
	return m_attributes;
}


//-------------------------------------------------------------------------------------------------
std::map<std::string, Uniform*> const & Material::GetUniformList( ) const
{
	return m_uniforms;
}


//-------------------------------------------------------------------------------------------------
unsigned int Material::GetGPUProgramID( ) const
{
	return m_program->GetShaderProgramID( );
}


//-------------------------------------------------------------------------------------------------
unsigned int Material::GetSamplerID( ) const
{
	return m_samplerID;
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, int uniformValue )
{
	auto uniformIter = m_uniforms.find( uniformName );
	ASSERT_RECOVERABLE( uniformIter != m_uniforms.end( ), "Uniform not found." );
	ASSERT_RECOVERABLE( uniformIter->second->m_type == GL_INT, "Wrong uniform type." );
	Uniform* uniform = m_uniforms[uniformName];
	*( (int*) uniform->m_data ) = uniformValue;
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, float uniformValue )
{
	auto uniformIter = m_uniforms.find( uniformName );
	ASSERT_RECOVERABLE( uniformIter != m_uniforms.end( ), "Uniform not found." );
	ASSERT_RECOVERABLE( uniformIter->second->m_type == GL_FLOAT, "Wrong uniform type." );
	Uniform* uniform = m_uniforms[uniformName];
	*( (float*) uniform->m_data ) = uniformValue;
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, Vector3f const & uniformValue )
{
	auto uniformIter = m_uniforms.find( uniformName );
	ASSERT_RECOVERABLE( uniformIter != m_uniforms.end( ), "Uniform not found." );
	ASSERT_RECOVERABLE( uniformIter->second->m_type == GL_FLOAT_VEC3, "Wrong uniform type." );
	Uniform* uniform = m_uniforms[uniformName];
	*((Vector3f*)uniform->m_data) = uniformValue;
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, Vector4f const & uniformValue )
{
	auto uniformIter = m_uniforms.find( uniformName );
	ASSERT_RECOVERABLE( uniformIter != m_uniforms.end( ), "Uniform not found." );
	ASSERT_RECOVERABLE( uniformIter->second->m_type == GL_FLOAT_VEC4, "Wrong uniform type." );
	Uniform* uniform = m_uniforms[uniformName];
	*( (Vector4f*) uniform->m_data ) = uniformValue;
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, Vector4i const & uniformValue )
{
	auto uniformIter = m_uniforms.find( uniformName );
	ASSERT_RECOVERABLE( uniformIter != m_uniforms.end( ), "Uniform not found." );
	ASSERT_RECOVERABLE( uniformIter->second->m_type == GL_INT_VEC4, "Wrong uniform type." );
	Uniform* uniform = m_uniforms[uniformName];
	*( (Vector4i*) uniform->m_data ) = uniformValue;
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, Matrix4f const & uniformValue )
{
	auto uniformIter = m_uniforms.find( uniformName );
	ASSERT_RECOVERABLE( uniformIter != m_uniforms.end( ), "Uniform not found." );
	ASSERT_RECOVERABLE( uniformIter->second->m_type == GL_FLOAT_MAT4, "Wrong uniform type." );
	Uniform * uniform = m_uniforms[uniformName];
	*( (Matrix4f*) uniform->m_data ) = uniformValue;
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, Color const & uniformValue )
{
	Vector4f convertedValue = uniformValue.GetVector4f( );
	SetUniform( uniformName, convertedValue );
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, std::string const & uniformValue )
{
	Texture const *generatedTexture = Texture::CreateOrLoadTexture( uniformValue );
	auto uniformIter = m_uniforms.find( uniformName );
	ASSERT_RECOVERABLE( uniformIter != m_uniforms.end( ), "Uniform not found." );
	ASSERT_RECOVERABLE( uniformIter->second->m_type == GL_SAMPLER_2D, "Wrong uniform type." );
	Uniform * uniform = m_uniforms[uniformName];
	*( (unsigned int*) uniform->m_data ) = generatedTexture->m_openglTextureID;
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, unsigned int uniformValue )
{
	auto uniformIter = m_uniforms.find( uniformName );
	ASSERT_RECOVERABLE( uniformIter != m_uniforms.end( ), "Uniform not found." );
	ASSERT_RECOVERABLE( uniformIter->second->m_type == GL_SAMPLER_2D, "Wrong uniform type." );
	Uniform * uniform = m_uniforms[uniformName];
	*( (unsigned int*) uniform->m_data ) = uniformValue;
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, Texture const * uniformValue )
{
	auto uniformIter = m_uniforms.find( uniformName );
	ASSERT_RECOVERABLE( uniformIter != m_uniforms.end( ), "Uniform not found." );
	ASSERT_RECOVERABLE( uniformIter->second->m_type == GL_SAMPLER_2D, "Wrong uniform type." );
	Uniform * uniform = m_uniforms[uniformName];
	*( (unsigned int*) uniform->m_data ) = uniformValue->m_openglTextureID;
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, int * uniformValue )
{
	//Doesn't exist yet, lets make space for it and assign it to the data
	if ( m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( ( Uniform* ) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( int ) * uniformSize;
		int *data = ( int* ) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
			(
				( ( Uniform* ) m_uniforms[uniformName] )->m_bindPoint,
				( ( Uniform* ) m_uniforms[uniformName] )->m_length,
				uniformSize,
				( ( Uniform* ) m_uniforms[uniformName] )->m_type,
				( ( Uniform* ) m_uniforms[uniformName] )->m_name,
				data //Setting new value
				);
	}

	//update the data
	else if ( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( ( Uniform* ) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( int ) * uniformSize;
		memcpy( m_uniforms[uniformName]->m_data, uniformValue, dataSize );
	}

	//That's not real
	else
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, float * uniformValue )
{
	//Doesn't exist yet, lets make space for it and assign it to the data
	if ( m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( ( Uniform* ) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( float ) * uniformSize;
		float *data = ( float* ) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
			(
				( ( Uniform* ) m_uniforms[uniformName] )->m_bindPoint,
				( ( Uniform* ) m_uniforms[uniformName] )->m_length,
				uniformSize,
				( ( Uniform* ) m_uniforms[uniformName] )->m_type,
				( ( Uniform* ) m_uniforms[uniformName] )->m_name,
				data
				);
	}

	//update the data
	else if ( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( ( Uniform* ) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( float ) * uniformSize;
		memcpy( m_uniforms[uniformName]->m_data, uniformValue, dataSize );
	}

	//That's not real
	else
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, Vector2f * uniformValue )
{
	//Doesn't exist yet, lets make space for it and assign it to the data
	if ( m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( ( Uniform* ) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( Vector2f ) * uniformSize;
		Vector2f *data = ( Vector2f* ) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
			(
				( ( Uniform* ) m_uniforms[uniformName] )->m_bindPoint,
				( ( Uniform* ) m_uniforms[uniformName] )->m_length,
				uniformSize,
				( ( Uniform* ) m_uniforms[uniformName] )->m_type,
				( ( Uniform* ) m_uniforms[uniformName] )->m_name,
				data
				);
	}

	//update the data
	else if ( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( ( Uniform* ) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( Vector2f ) * uniformSize;
		memcpy( m_uniforms[uniformName]->m_data, uniformValue, dataSize );
	}

	//That's not real
	else
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, Vector3f * uniformValue )
{
	//Doesn't exist yet, lets make space for it and assign it to the data
	if ( m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( ( Uniform* ) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( Vector3f ) * uniformSize;
		Vector3f *data = ( Vector3f* ) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
			(
				( ( Uniform* ) m_uniforms[uniformName] )->m_bindPoint,
				( ( Uniform* ) m_uniforms[uniformName] )->m_length,
				uniformSize,
				( ( Uniform* ) m_uniforms[uniformName] )->m_type,
				( ( Uniform* ) m_uniforms[uniformName] )->m_name,
				data
				);
	}

	//update the data
	else if ( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( ( Uniform* ) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( Vector3f ) * uniformSize;
		memcpy( m_uniforms[uniformName]->m_data, uniformValue, dataSize );
	}

	//That's not real
	else
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::string const & uniformName, Vector4f * uniformValue )
{
	//Doesn't exist yet, lets make space for it and assign it to the data
	if ( m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( ( Uniform* ) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( Vector4f ) * uniformSize;
		Vector4f *data = ( Vector4f* ) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
			(
				( ( Uniform* ) m_uniforms[uniformName] )->m_bindPoint,
				( ( Uniform* ) m_uniforms[uniformName] )->m_length,
				uniformSize,
				( ( Uniform* ) m_uniforms[uniformName] )->m_type,
				( ( Uniform* ) m_uniforms[uniformName] )->m_name,
				data
				);
	}

	//update the data
	else if ( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( ( Uniform* ) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( Vector4f ) * uniformSize;
		memcpy( m_uniforms[uniformName]->m_data, uniformValue, dataSize );
	}

	//That's not real
	else
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}
}


//-------------------------------------------------------------------------------------------------
//#TODO: Continue Work here
void Material::SetUniform( std::string const & uniformName, Matrix4f * uniformValue )
{
	//Doesn't exist yet, lets make space for it and assign it to the data
	if( m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( (Uniform*) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( Matrix4f ) * uniformSize;
		Matrix4f *data = (Matrix4f*) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
		(
			( (Uniform*) m_uniforms[uniformName] )->m_bindPoint,
			( (Uniform*) m_uniforms[uniformName] )->m_length,
			uniformSize,
			( (Uniform*) m_uniforms[uniformName] )->m_type,
			( (Uniform*) m_uniforms[uniformName] )->m_name,
			data
		);
	}

	//update the data
	else if( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( (Uniform*) m_uniforms[uniformName] )->m_size;
		size_t dataSize = sizeof( Matrix4f ) * uniformSize;
		memcpy( m_uniforms[uniformName]->m_data, uniformValue, dataSize );
	}

	//That's not real
	else
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::vector<Light> const &uniformLights, int lightCount )
{
	SetUniform( "uLightCount", lightCount );
	Vector4f lightColor[16];
	Vector3f lightPosition[16];
	Vector3f lightDirection[16];
	float lightIsDirectional[16];
	float lightDistanceMin[16];
	float lightDistanceMax[16];
	float lightStrengthAtMin[16];
	float lightStrengthAtMax[16];
	float lightInnerAngle[16];
	float lightOuterAngle[16];
	float lightStrengthInside[16];
	float lightStrengthOutside[16];
	for ( unsigned int lightIndex = 0; lightIndex < uniformLights.size( ); ++lightIndex )
	{
		lightColor[lightIndex] = uniformLights[lightIndex].m_lightColor.GetVector4f( );
		lightPosition[lightIndex] = uniformLights[lightIndex].m_position;
		lightDirection[lightIndex] = uniformLights[lightIndex].m_lightDirection;
		lightIsDirectional[lightIndex] = uniformLights[lightIndex].m_isLightDirectional;
		lightDistanceMin[lightIndex] = uniformLights[lightIndex].m_minLightDistance;
		lightDistanceMax[lightIndex] = uniformLights[lightIndex].m_maxLightDistance;
		lightStrengthAtMin[lightIndex] = uniformLights[lightIndex].m_strengthAtMin;
		lightStrengthAtMax[lightIndex] = uniformLights[lightIndex].m_strengthAtMax;
		lightInnerAngle[lightIndex] = uniformLights[lightIndex].m_innerLightCosAngle;
		lightOuterAngle[lightIndex] = uniformLights[lightIndex].m_outerLightCosAngle;
		lightStrengthInside[lightIndex] = uniformLights[lightIndex].m_strengthInside;
		lightStrengthOutside[lightIndex] = uniformLights[lightIndex].m_strengthOutside;
	}

	SetUniform( "uLightColor", lightColor );
	SetUniform( "uLightPosition", lightPosition );
	SetUniform( "uLightDirection", lightDirection );
	SetUniform( "uLightIsDirectional", lightIsDirectional );
	SetUniform( "uLightDistanceMin", lightDistanceMin );
	SetUniform( "uLightDistanceMax", lightDistanceMax );
	SetUniform( "uLightStrengthAtMin", lightStrengthAtMin );
	SetUniform( "uLightStrengthAtMax", lightStrengthAtMax );
	SetUniform( "uLightInnerAngle", lightInnerAngle );
	SetUniform( "uLightOuterAngle", lightOuterAngle );
	SetUniform( "uLightStrengthInside", lightStrengthInside );
	SetUniform( "uLightStrengthOutside", lightStrengthOutside );
}


//-------------------------------------------------------------------------------------------------
void Material::SetUniform( std::vector<Matrix4f> const & uniformMatricies, int matrixCount )
{
	Matrix4f matrix[200];
	for( int matrixIndex = 0; matrixIndex < matrixCount; ++matrixIndex )
	{
		matrix[matrixIndex] = uniformMatricies[matrixIndex];
	}

	SetUniform( "uMatrix", matrix );
}


//-------------------------------------------------------------------------------------------------
void Material::UpdateBindpoints( )
{
	//Attribute and Uniform count
	GLuint progID = m_program->GetShaderProgramID( );

	//Reset all attribute bindpoints
	for ( auto attribute : m_attributes )
	{
		attribute.second->m_bindPoint = -1;
	}

	//Get Attribute information
	int attributeCount;
	glGetProgramiv( progID, GL_ACTIVE_ATTRIBUTES, &attributeCount );
	for ( int aIndex = 0; aIndex < attributeCount; ++aIndex )
	{
		GLsizei length;
		GLint size;
		GLenum type;
		GLchar name[100];
		glGetActiveAttrib( progID, aIndex, sizeof( name ), &length, &size, &type, name );
		GLint loc = glGetAttribLocation( progID, name );
		ASSERT_RECOVERABLE( loc >= 0, "Can't find bind point for Uniform." );

		std::string aName( name );
		std::size_t foundIndex = aName.find( '[' );

		if ( foundIndex != std::string::npos )
		{
			aName = aName.substr( 0, foundIndex );
		}

		size_t attributeHash = std::hash<std::string>{ }( aName );
		auto foundAttribute = m_attributes.find( attributeHash );
		//Update existing bindpoints
		if ( foundAttribute != m_attributes.end( ) )
		{
			foundAttribute->second->m_bindPoint = loc;
		}
		//Create new attributes
		else
		{
			switch ( type )
			{
			case GL_INT:
			{
				int* temp = new int[size];
				m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
				break;
			}
			case GL_FLOAT:
			{
				float* temp = new float[size];
				m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
				break;
			}
			case GL_FLOAT_VEC2:
			{
				Vector2f* temp = new Vector2f[size];
				m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
				break;
			}
			case GL_FLOAT_VEC3:
			{
				Vector3f* temp = new Vector3f[size];
				m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
				break;
			}
			case GL_FLOAT_VEC4:
			{
				Vector4f* temp = new Vector4f[size];
				m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
				break;
			}
			//#TODO: add Matrix case
			case GL_SAMPLER_2D:
			{

				unsigned int* temp = new unsigned int( 0 );
				m_attributes[attributeHash] = new Attribute( loc, length, size, type, aName, temp );
				break;
			}
			}
		}
	}

	//Reset all uniform bindpoints
	for ( auto uniforms : m_uniforms )
	{
		uniforms.second->m_bindPoint = -1;
	}

	//Get Uniform information
	int uniformCount;
	glGetProgramiv( progID, GL_ACTIVE_UNIFORMS, &uniformCount );
	for ( int uIndex = 0; uIndex < uniformCount; ++uIndex )
	{
		GLsizei length;
		GLint size;
		GLenum type;
		GLchar name[100];
		glGetActiveUniform( progID, uIndex, sizeof( name ), &length, &size, &type, name );
		GLint loc = glGetUniformLocation( progID, name );
		ASSERT_RECOVERABLE( loc >= 0, "Can't find bind point for Uniform." );

		std::string uName( name );
		std::size_t foundIndex = uName.find( '[' );

		if ( foundIndex != std::string::npos )
		{
			uName = uName.substr( 0, foundIndex );
		}

		auto foundUniform = m_uniforms.find( uName );
		//Update existing bindpoints
		if ( foundUniform != m_uniforms.end( ) )
		{
			foundUniform->second->m_bindPoint = loc;
		}
		//Create new attributes
		else
		{
			switch ( type )
			{
			case GL_INT:
			{
				int* temp = new int[size];
				m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
				break;
			}
			case GL_FLOAT:
			{
				float* temp = new float[size];
				m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
				break;
			}
			case GL_FLOAT_VEC2:
			{
				Vector2f* temp = new Vector2f[size];
				m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
				break;
			}
			case GL_FLOAT_VEC3:
			{
				Vector3f* temp = new Vector3f[size];
				m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
				break;
			}
			case GL_FLOAT_VEC4:
			{
				Vector4f* temp = new Vector4f[size];
				m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
				break;
			}
			case GL_FLOAT_MAT4:
			{
				Matrix4f* temp = new Matrix4f[size];
				m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
				break;
			}
			case GL_SAMPLER_2D:
			{
				unsigned int * temp = new unsigned int( 0 );
				m_uniforms[uName] = new Uniform( loc, length, size, type, uName, temp );
				break;
			}
			}
		}
	}
}