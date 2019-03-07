#include "Engine/RenderSystem/Renderer.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <Math.h>
#include <vector>
#pragma comment( lib, "opengl32" ) // Link in the OpenGL32.lib static library
#pragma comment( lib, "Glu32" ) // Link in the OpenGL32.lib static library

#include "Engine/Core/Time.hpp"
#include "Engine/Core/Engine.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Utils/MathUtils.hpp"
#include "Engine/Math/Vector4f.hpp"
#include "Engine/Math/Matrix4f.hpp"
#include "Engine/RenderSystem/Attribute.hpp"
#include "Engine/RenderSystem/BitmapFont.hpp"
#include "Engine/RenderSystem/Camera3D.hpp"
#include "Engine/RenderSystem/Framebuffer.hpp"
#include "Engine/RenderSystem/Glyph.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/RenderSystem/Mesh.hpp"
#include "Engine/RenderSystem/MeshRenderer.hpp"
#include "Engine/RenderSystem/OpenGLExtensions.hpp"
#include "Engine/RenderSystem/ShaderProgram.hpp"
#include "Engine/RenderSystem/Texture.hpp"
#include "Engine/RenderSystem/Uniform.hpp"
#include "Engine/RenderSystem/Vertex.hpp"
#include "Engine/Utils/FileUtils.hpp"
#include "Engine/Utils/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-------------------------------------------------------------------------------------------------
Renderer * g_RenderSystem = nullptr;


//-------------------------------------------------------------------------------------------------
//STATIC char const * Renderer::DEFAULT_FONT = "Data/Fonts/ConsoleFont.fnt";
//STATIC char const * Renderer::DEFAULT_FONT = "Data/Fonts/OCR.fnt";
//STATIC char const * Renderer::DEFAULT_FONT = "Data/Fonts/Basica.fnt";
//STATIC char const * Renderer::DEFAULT_FONT = "Data/Fonts/GalaxyMonkey.fnt";
	STATIC char const * Renderer::DEFAULT_FONT = "Data/Fonts/MainFrame.fnt";
//STATIC char const * Renderer::DEFAULT_FONT = "Data/Fonts/PressStart.fnt";
//STATIC char const * Renderer::DEFAULT_FONT = "Data/Fonts/DesignerBlock.fnt";
//STATIC char const * Renderer::DEFAULT_FONT = "Data/Fonts/ClayFont.png";
//STATIC char const * Renderer::DEFAULT_FONT = "Data/Fonts/RetroFont.png";


//-------------------------------------------------------------------------------------------------
//CHANGE DEBUGGING > WORKING DIRECTORY > $(SolutionDir)Run_$(PlatformName)/
Renderer::Renderer( )
	: m_activeFBO( nullptr )
	, m_activeCamera( nullptr )
	, m_currentRenderState( RenderState::BASIC_3D )
	, m_currentClearColor( Color::BLACK )
	, m_currentTextureID( 0 )
	, m_currentLineWidth( 1.f )
	, m_currentPointSize( 1.f )
{
	Initialize( );

	Mesh::InitializeDefaultMeshes( );
	Material::InitializeDefaultMaterials( );
	BitmapFont::CreateOrGetFont( DEFAULT_FONT );
}


//-------------------------------------------------------------------------------------------------
Renderer::~Renderer( )
{
	BitmapFont::DestroyRegistry( );
	ShaderProgram::DestroyRegistry( );
	Material::DestroyDefaultMaterials( );
	Mesh::DestroyDefaultMeshes( );
	Texture::DestroyRegistry( );
}


//-------------------------------------------------------------------------------------------------
void Renderer::Initialize( )
{
	//---------------------------------------------------------------------------------------------
	// Manually Set BASIC_3D Render State
	// This has to be done because we assume m_currentRenderState = Active
	//---------------------------------------------------------------------------------------------
	
	//Set up blending
	glEnable( GL_BLEND );

	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glCullFace( GL_BACK );
	glClearColor( 0.f, 0.f, 0.f, 1.f );

	//Set up lines
	glEnable( GL_LINE_SMOOTH );
	SetLineWidth( m_currentLineWidth );

	//Set up points
	SetPointSize( m_currentPointSize );

	//---------------------------------------------------------------------------------------------
	// BINDING OPENGL FUNCTIONS
	//---------------------------------------------------------------------------------------------

	glGenBuffers = ( PFNGLGENBUFFERSPROC )wglGetProcAddress( "glGenBuffers" );
	glBindBuffer = ( PFNGLBINDBUFFERPROC )wglGetProcAddress( "glBindBuffer" );
	glBufferData = ( PFNGLBUFFERDATAPROC )wglGetProcAddress( "glBufferData" );
	glDeleteBuffers = ( PFNGLDELETEBUFFERSPROC )wglGetProcAddress( "glDeleteBuffers" );
	glGenerateMipmap = ( PFNGLGENERATEMIPMAPPROC )wglGetProcAddress( "glGenerateMipmap" );

	glCreateShader = ( PFNGLCREATESHADERPROC ) wglGetProcAddress( "glCreateShader" );
	glShaderSource = ( PFNGLSHADERSOURCEPROC ) wglGetProcAddress( "glShaderSource" );
	glCompileShader = ( PFNGLCOMPILESHADERPROC ) wglGetProcAddress( "glCompileShader" );
	glGetShaderiv = ( PFNGLGETSHADERIVPROC ) wglGetProcAddress( "glGetShaderiv" );
	glDeleteShader = ( PFNGLDELETESHADERPROC ) wglGetProcAddress( "glDeleteShader" );
	glGetShaderInfoLog = ( PFNGLGETSHADERINFOLOGPROC ) wglGetProcAddress( "glGetShaderInfoLog" );

	glCreateProgram = ( PFNGLCREATEPROGRAMPROC ) wglGetProcAddress( "glCreateProgram" );
	glAttachShader = ( PFNGLATTACHSHADERPROC ) wglGetProcAddress( "glAttachShader" );
	glLinkProgram = ( PFNGLLINKPROGRAMPROC ) wglGetProcAddress( "glLinkProgram" );
	glGetProgramiv = ( PFNGLGETPROGRAMIVPROC ) wglGetProcAddress( "glGetProgramiv" );
	glDetachShader = ( PFNGLDETACHSHADERPROC ) wglGetProcAddress( "glDetachShader" );
	glDeleteProgram = ( PFNGLDELETEPROGRAMPROC ) wglGetProcAddress( "glDeleteProgram" );
	glGetProgramInfoLog = ( PFNGLGETPROGRAMINFOLOGPROC ) wglGetProcAddress( "glGetProgramInfoLog" );

	glGenVertexArrays = ( PFNGLGENVERTEXARRAYSPROC ) wglGetProcAddress( "glGenVertexArrays" );
	glDeleteVertexArrays = ( PFNGLDELETEVERTEXARRAYSPROC ) wglGetProcAddress( "glDeleteVertexArrays" );
	glBindVertexArray = ( PFNGLBINDVERTEXARRAYPROC ) wglGetProcAddress( "glBindVertexArray" );
	glGetAttribLocation = ( PFNGLGETATTRIBLOCATIONPROC ) wglGetProcAddress( "glGetAttribLocation" );
	glEnableVertexAttribArray = ( PFNGLENABLEVERTEXATTRIBARRAYPROC ) wglGetProcAddress( "glEnableVertexAttribArray" );
	glVertexAttribPointer = ( PFNGLVERTEXATTRIBPOINTERPROC ) wglGetProcAddress( "glVertexAttribPointer" );
	glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC) wglGetProcAddress( "glVertexAttribIPointer" );

	glUseProgram = ( PFNGLUSEPROGRAMPROC ) wglGetProcAddress( "glUseProgram" );

	//---------------------------------------------------------------------------------------------

	glGetUniformLocation = ( PFNGLGETUNIFORMLOCATIONPROC ) wglGetProcAddress( "glGetUniformLocation" );
	glGetActiveUniform = ( PFNGLGETACTIVEUNIFORMPROC ) wglGetProcAddress( "glGetActiveUniform" );
	glGetActiveAttrib = ( PFNGLGETACTIVEATTRIBPROC ) wglGetProcAddress( "glGetActiveAttrib" );

	glUniform1fv = ( PFNGLUNIFORM1FVPROC ) wglGetProcAddress( "glUniform1fv" );
	glUniform2fv = ( PFNGLUNIFORM2FVPROC ) wglGetProcAddress( "glUniform2fv" );
	glUniform3fv = ( PFNGLUNIFORM3FVPROC ) wglGetProcAddress( "glUniform3fv" );
	glUniform4fv = ( PFNGLUNIFORM4FVPROC ) wglGetProcAddress( "glUniform4fv" );

	glUniform1iv = ( PFNGLUNIFORM1IVPROC ) wglGetProcAddress( "glUniform1iv" );
	glUniform2iv = ( PFNGLUNIFORM2IVPROC ) wglGetProcAddress( "glUniform2iv" );
	glUniform3iv = ( PFNGLUNIFORM3IVPROC ) wglGetProcAddress( "glUniform3iv" );
	glUniform4iv = ( PFNGLUNIFORM4IVPROC ) wglGetProcAddress( "glUniform4iv" );

	glUniformMatrix4fv = ( PFNGLUNIFORMMATRIX4FVPROC ) wglGetProcAddress( "glUniformMatrix4fv" );

	//---------------------------------------------------------------------------------------------

	glActiveTexture = ( PFNGLACTIVETEXTUREPROC ) wglGetProcAddress( "glActiveTexture" );
	glGenSamplers = ( PFNGLGENSAMPLERSPROC ) wglGetProcAddress( "glGenSamplers" );
	glBindSampler = ( PFNGLBINDSAMPLERPROC ) wglGetProcAddress( "glBindSampler" );
	glSamplerParameteri = ( PFNGLSAMPLERPARAMETERIPROC ) wglGetProcAddress( "glSamplerParameteri" );

	//---------------------------------------------------------------------------------------------

	glGenFramebuffers = ( PFNGLGENFRAMEBUFFERSPROC ) wglGetProcAddress( "glGenFramebuffers" );
	glBindFramebuffer = ( PFNGLBINDFRAMEBUFFERPROC ) wglGetProcAddress( "glBindFramebuffer" );
	glFramebufferTexture = ( PFNGLFRAMEBUFFERTEXTUREPROC ) wglGetProcAddress( "glFramebufferTexture" );
	glCheckFramebufferStatus = ( PFNGLCHECKFRAMEBUFFERSTATUSPROC ) wglGetProcAddress( "glCheckFramebufferStatus" );
	glDeleteFramebuffers = ( PFNGLDELETEFRAMEBUFFERSPROC ) wglGetProcAddress( "glDeleteFramebuffers" );
	glDrawBuffers = ( PFNGLDRAWBUFFERSPROC ) wglGetProcAddress( "glDrawBuffers" );
	glBlitFramebuffer = ( PFNGLBLITFRAMEBUFFERPROC ) wglGetProcAddress( "glBlitFramebuffer" );
	
	//-------------------------------------------------------------------------------------------------
	glDisableVertexAttribArray = ( PFNGLDISABLEVERTEXATTRIBARRAYPROC ) wglGetProcAddress( "glDisableVertexAttribArray" );
	//#TODO: Assert that these exist
}


//-------------------------------------------------------------------------------------------------
void Renderer::ClearScreen( const Color & clearColor )
{
	//According to Taylor, setting the clear color takes a long time if done every frame
	if( clearColor != m_currentClearColor )
	{
		m_currentClearColor = clearColor;
		glClearColor( clearColor.GetRFloat( ), clearColor.GetGFloat( ), clearColor.GetBFloat( ), clearColor.GetAFloat( ) );
	}
	glClearDepth( 1.f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


//-------------------------------------------------------------------------------------------------
GLuint Renderer::CreateSampler( GLenum const &min_filter, GLenum const &mag_filter, GLenum const &u_wrap, GLenum const &v_wrap )
{
	GLuint samepler_id;
	glGenSamplers( 1, &samepler_id );
	glSamplerParameteri( samepler_id, GL_TEXTURE_MIN_FILTER, min_filter );	//how does it shrink
	glSamplerParameteri( samepler_id, GL_TEXTURE_MAG_FILTER, mag_filter );	//how does it stretch
	glSamplerParameteri( samepler_id, GL_TEXTURE_WRAP_S, u_wrap );			//if u is < 0 or > 1, how does it behave
	glSamplerParameteri( samepler_id, GL_TEXTURE_WRAP_T, v_wrap );			//same, but for v

	return samepler_id;
}


//-------------------------------------------------------------------------------------------------
GLuint Renderer::CreateRenderBuffer( void const *data, size_t count, size_t elem_size, GLenum const &usage /*= GL_STATIC_DRAW*/ )
{
	GLuint buffer;
	glGenBuffers( 1, &buffer ); //Create

	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, count * elem_size, data, usage ); //Fill

	glBindBuffer( GL_ARRAY_BUFFER, NULL ); //Unbind

	return buffer; //Return generate ID
}


//-------------------------------------------------------------------------------------------------
GLuint Renderer::CreateShader( std::string const &filename, GLenum shader_type )
{
	std::vector<unsigned char> buffer;
	bool success = LoadBinaryFileToBuffer( filename, buffer );
	ASSERT_RECOVERABLE(  success, "File not found." );
	//buffer.push_back(NULL);

	GLuint shaderID = glCreateShader( shader_type );
	ASSERT_RECOVERABLE(  shaderID != NULL, "Shader failed to load." );

	GLint src_length = buffer.size( );
	GLchar* bufferStart = (GLchar*) &buffer[0];
	glShaderSource( shaderID, 1, &bufferStart, &src_length );

	glCompileShader( shaderID );

	//Check for errors
	GLint status;
	glGetShaderiv( shaderID, GL_COMPILE_STATUS, &status );
	if( status == GL_FALSE )
	{
		GLint length;
		glGetShaderiv( shaderID, GL_INFO_LOG_LENGTH, &length );

		std::string bufferError;
		bufferError.resize( length + 1 );
		glGetShaderInfoLog( shaderID, length, &length, &bufferError[0] );

		OutputDebugStringA( "\n===================================================================================================\n" );
		OutputDebugStringA( "ERROR COMPILING SHADER\n" );

		char fullPath[_MAX_PATH];
		_fullpath( fullPath, &filename[0], _MAX_PATH );
		std::string parseBufferError = bufferError;
		bool cont = true;
		while( cont )
		{
			int checkIndex = parseBufferError.substr( 1 ).find( "ERROR:" );
			std::string errorLine;

			//Last error
			if( checkIndex == -1 )
			{
				cont = false;
				errorLine = parseBufferError;
			}
			else
			{
				errorLine = parseBufferError.substr( 0, checkIndex );
			}
			int startIndex = parseBufferError.find( "0:" ) + 2; //Make start after "0:"
			parseBufferError = parseBufferError.substr( startIndex );
			int endIndex = parseBufferError.find( ":" );
			std::string lineNumber = parseBufferError.substr( 0, endIndex );

			//Prepare next parseBuffer
			if( cont )
			{
				int nextIndex = parseBufferError.find( "ERROR:" );
				parseBufferError = parseBufferError.substr( nextIndex );
			}

			std::string debugString = Stringf( "%s(%s): %s\n", fullPath, &lineNumber[0], &errorLine[0] );
			OutputDebugStringA( &debugString[0] );
		}
		glDeleteShader( shaderID );

		std::string const openGLVersion = GetOpenGLVersion( );
		std::string const openGLSLVersion = GetGLSLVersion( );
		ShaderError( fullPath, bufferError, openGLVersion, openGLSLVersion );
		return 0;
	}

	return shaderID;
}


//-------------------------------------------------------------------------------------------------
GLuint Renderer::CreateAndLinkProgram( GLuint vs, GLuint fs, std::string const &debugFilepath )
{
	//NOTE: debugFilepath is only used if there is a link error
	GLuint programID = glCreateProgram( );
	ASSERT_RECOVERABLE(  programID != NULL, "No Program!" );

	glAttachShader( programID, vs );
	glAttachShader( programID, fs );

	glLinkProgram( programID );

	//GetProgrami - will get integer
	//GetProgramiv - will get (multiple) integers
	GLint status;
	char fullPath[_MAX_PATH];
	_fullpath( fullPath, &debugFilepath[0], _MAX_PATH );
	glGetProgramiv( programID, GL_LINK_STATUS, &status );
	if(status == GL_FALSE ) //#TODO: Make this a function
	{
		//#TODO: Maybe make the logging of an error its own function
		GLint logLength;
		glGetProgramiv( programID, GL_INFO_LOG_LENGTH, &logLength );

		std::string bufferError;
		bufferError.resize( logLength + 1 );
		glGetProgramInfoLog( programID, logLength, &logLength, &bufferError[0] );

		std::string debugString = Stringf( "%s: \n%s\n", fullPath, &bufferError[0] );
		OutputDebugStringA( "\n===================================================================================================\n" );
		OutputDebugStringA( "ERROR LINKING PROGRAM\n" );
		OutputDebugStringA( &debugString[0] );

		glDeleteProgram( programID );

		std::string const openGLVersion = GetOpenGLVersion( );
		std::string const openGLSLVersion = GetGLSLVersion( );
		ShaderError( fullPath, bufferError, openGLVersion, openGLSLVersion );
		return 0;
	}
	else
	{
		//Frees up shaders id, 
		glDetachShader( programID, vs );
		glDetachShader( programID, fs );
	}

	return programID;
}


//-------------------------------------------------------------------------------------------------
void Renderer::UpdateRenderBuffer( GLuint bufferID, void const *data, size_t count, size_t elem_size, GLenum const &usage /*= GL_STATIC_DRAW*/ )
{
	glBindBuffer( GL_ARRAY_BUFFER, bufferID );
	glBufferData( GL_ARRAY_BUFFER, count * elem_size, data, usage ); //Fill

	glBindBuffer( GL_ARRAY_BUFFER, NULL ); //Unbind
}


//-------------------------------------------------------------------------------------------------
void Renderer::CreateOrUpdateRenderBuffer( GLuint * bufferID, void const *data, size_t count, size_t elem_size, GLenum const &usage /*= GL_STATIC_DRAW */ )
{
	if( *bufferID == NULL )
	{
		*bufferID = CreateRenderBuffer( data, count, elem_size, usage );
	}
	else
	{
		UpdateRenderBuffer( *bufferID, data, count, elem_size, usage );
	}
}


//-------------------------------------------------------------------------------------------------
void Renderer::BindMeshToVAO( GLuint vaoID, Mesh const *mesh, Material const *material )
{
	unsigned int vboID = mesh->GetVBOID( );
	unsigned int iboID = mesh->GetIBOID( );
	std::vector< VertexDefinition > const & layout = mesh->GetLayout( );
	std::map< size_t, Attribute* > const & attributes = material->GetAttributeList( );

	glBindVertexArray( vaoID );
	glBindBuffer( GL_ARRAY_BUFFER, vboID );

	//Bind Properties
	for( VertexDefinition vertexLayout : layout )
	{
		//Map Types
		GLenum type = GetVertexDataType( vertexLayout.m_type );

		//Map Boolean
		GLboolean normalize = vertexLayout.m_normalized ? GL_TRUE : GL_FALSE;
		size_t attributeHash = std::hash<std::string>{ }( vertexLayout.m_name );
		auto foundAttribute = attributes.find( attributeHash );
		if ( foundAttribute != attributes.end( ) )
		{
			int bindPoint = foundAttribute->second->GetBindPoint( );
			BindShaderProgramProperty( bindPoint, vertexLayout.m_count, type, normalize, vertexLayout.m_stride, vertexLayout.m_offset );
		}
		else
		{
			continue;
			//#TODO: Make sure I don't want to handle this
			//ASSERT_OR_DIE( found != attributes.end( ), Stringf( "Attribute %s doesn't exist.", &vertexLayout.m_name[0] ) );
		}
	}

	//Unbind VBO
	glBindBuffer( GL_ARRAY_BUFFER, NULL );

	if ( iboID != NULL )
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, iboID );
	}

	//Unbinds the vertex array object
	glBindVertexArray( NULL );
}


//-------------------------------------------------------------------------------------------------
void Renderer::BindShaderProgramProperty( int bindPoint, GLint count, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset )
{
	//Position to bind
	if ( bindPoint >= 0 )
	{
		if( type == GL_UNSIGNED_INT )
		{
			glEnableVertexAttribArray( bindPoint );
			glVertexAttribIPointer( bindPoint, count, type, stride, (GLvoid*) offset );
		}
		else
		{
			glEnableVertexAttribArray( bindPoint );
			glVertexAttribPointer(
				bindPoint,			//Bind point to Shader
				count,				//Number of data elements passed
				type,				//type of data
				normalize,			//if we're passing in normals (it'll now normalize for us)
				stride,				//Size of the stride between elements
				(GLvoid*)offset		//Pointer to data's location in class
				);
		}
	}
	else
	{
		//#TODO: Make Assert and continue
		//ASSERT_OR_DIE( posBind >= 0, "No bind point." );
	}
}


//-------------------------------------------------------------------------------------------------
void Renderer::BindTextureSampler( GLuint samplerID, GLuint textureID, unsigned int textureIndexPort, int bindLocation )
{
	if ( textureID != NULL )
	{
		glActiveTexture( GL_TEXTURE0 + textureIndexPort );
		glBindTexture( GL_TEXTURE_2D, textureID );
		glBindSampler( textureIndexPort, samplerID );
		glUniform1iv( bindLocation, 1, ( GLint* ) &textureIndexPort );
	}
}


//-------------------------------------------------------------------------------------------------
GLenum Renderer::GetVertexDataType( VertexDataType const & type ) const
{
	switch( type )
	{
	case VertexDataType_FLOAT:
		return GL_FLOAT;
	case VertexDataType_UBYTE:
		return GL_UNSIGNED_BYTE;
	case VertexDataType_INT:
		return GL_INT;
	case VertexDataType_UINT:
		return GL_UNSIGNED_INT;
	default:
		ERROR_AND_DIE( "Vertex type not handled" );
	}
}


//-------------------------------------------------------------------------------------------------
void Renderer::SetShaderProgramUniforms( GLuint samplerID, std::map<std::string, Uniform*> const & uniformList )
{
	GLuint textureID = NULL;
	unsigned int textureIndexPort = 0;
	for( auto uniformIter = uniformList.begin( ); uniformIter != uniformList.end( ); ++uniformIter )
	{
		Uniform* currentUniform = uniformIter->second;
		switch( currentUniform->m_type )
		{
		case GL_INT:
			glUniform1iv( currentUniform->m_bindPoint, currentUniform->m_size, (GLint*) currentUniform->m_data );
			break;
		case GL_FLOAT:
			glUniform1fv( currentUniform->m_bindPoint, currentUniform->m_size, (GLfloat*) currentUniform->m_data );
			break;
		case GL_FLOAT_VEC2:
			glUniform2fv( currentUniform->m_bindPoint, currentUniform->m_size, (GLfloat*) currentUniform->m_data );
			break;
		case GL_FLOAT_VEC3:
			glUniform3fv( currentUniform->m_bindPoint, currentUniform->m_size, (GLfloat*) currentUniform->m_data );
			break;
		case GL_FLOAT_VEC4:
			glUniform4fv( currentUniform->m_bindPoint, currentUniform->m_size, (GLfloat*) currentUniform->m_data );
			break;
		case GL_FLOAT_MAT4:
			glUniformMatrix4fv( currentUniform->m_bindPoint, currentUniform->m_size, GL_FALSE, ( GLfloat* ) &( (Matrix4f*) currentUniform->m_data )->m_data[0] );
			break;
		case GL_SAMPLER_2D:
			textureID = *( (unsigned int*) currentUniform->m_data );
			if( textureID != NULL )
			{
				std::string uniformName = currentUniform->m_name;
				BindTextureSampler( samplerID, textureID, textureIndexPort, currentUniform->m_bindPoint );
			}
			++textureIndexPort; //#TODO: Increment even if NULL, is this correct?
								//#TODO: Probably do a GetPort() and retrieve a list where I set the number for each texture/uniforms name
			break;
		default:
			ERROR_AND_DIE( "Uniform type case not handled" );
			break;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void Renderer::SetMatrixUniforms( MeshRenderer * setMeshRenderer )
{
	//Get View Projection Matrixes
	Matrix4f cameraView = m_activeCamera->GetViewMatrix( );
	Matrix4f cameraProj = m_activeCamera->GetProjectionMatrix( );

	//Set View Projection Matrixes
	setMeshRenderer->SetUniform( "uView", cameraView );
	setMeshRenderer->SetUniform( "uProj", cameraProj );
}


//-------------------------------------------------------------------------------------------------
void Renderer::MeshRender( MeshRenderer const * meshRenderer )
{
	//Set Render State
	ApplyRenderState( meshRenderer->GetRenderState( ) );
	
	//Set shader program to bind everything to
	unsigned int gpuProgramID = meshRenderer->GetGPUProgramID( );
	glUseProgram( gpuProgramID );
	GLuint samplerID = meshRenderer->GetSamplerID( );
	SetShaderProgramUniforms( samplerID, meshRenderer->GetMaterialUniformList( ) );
	SetShaderProgramUniforms( samplerID, meshRenderer->GetUniformList( ) );

	//Reset active Texture to 0 ("unbinding")
	glActiveTexture( GL_TEXTURE0 + 0 ); //#TODO: Remove when everything uses shaders to draw?

	//Attach Vertex Array Object and Shader Program
	unsigned vaoID = meshRenderer->GetVAOID( );
	glBindVertexArray( vaoID );

	//Draw Vertexes
	for ( DrawInstruction instruction : meshRenderer->GetDrawInstructions( ) )
	{
		if ( !instruction.m_useIndexBuffer )
		{
			glDrawArrays( instruction.GetPrimitiveType(), instruction.m_startIndex, instruction.m_count );
		}
		else
		{
			glDrawElements( instruction.GetPrimitiveType(), instruction.m_count, GL_UNSIGNED_INT, ( GLvoid* ) ( instruction.m_startIndex * sizeof( unsigned int ) ) );
		}
		g_EngineSystem->IncrementDrawCalls( );
	}

	//Remove and Clear
	glUseProgram( NULL );
	glBindVertexArray( NULL );
}


//-------------------------------------------------------------------------------------------------
void Renderer::DeleteFramebuffer( Framebuffer *fbo )
{
	if ( m_activeFBO == fbo )
	{
		BindFramebuffer( nullptr );
	}
	glDeleteFramebuffers( 1, &( fbo->m_fboHandle ) );
	delete fbo;
}


//-------------------------------------------------------------------------------------------------
//Will only have one FBO bound at a time
void Renderer::BindFramebuffer( Framebuffer *fbo )
{
	if ( m_activeFBO == fbo )
	{
		return;
	}

	m_activeFBO = fbo;
	if ( fbo == nullptr )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, NULL );
		Vector2i windowDimensions = GetWindowDimensions( );
		glViewport( 0, 0, windowDimensions.x, windowDimensions.y );
	}
	else
	{
		glBindFramebuffer( GL_FRAMEBUFFER, fbo->m_fboHandle );
		glViewport( 0, 0, fbo->m_pixelWidth, fbo->m_pixelHeight );

		GLenum renderTargets[32];
		unsigned int colorCount = fbo->m_colorTargets.size( );
		for ( unsigned int i = 0; i < colorCount; ++i )
		{
			renderTargets[i] = GL_COLOR_ATTACHMENT0 + i;
		}
		glDrawBuffers( colorCount, renderTargets );
	}
}


//-------------------------------------------------------------------------------------------------
//Only use for debug purposes
void Renderer::FramebufferCopyToBack( Framebuffer *fbo )
{
	if ( fbo == nullptr )
	{
		return;
	}

	GLuint fboh = fbo->m_fboHandle;
	glBindFramebuffer( GL_READ_FRAMEBUFFER, fboh );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, NULL );

	unsigned int readWidth = fbo->m_pixelWidth;
	unsigned int readHeight = fbo->m_pixelHeight;

	Vector2i windowDimensions = GetWindowDimensions( );
	unsigned int drawWidth = (unsigned int) windowDimensions.x;
	unsigned int drawHeight = (unsigned int) windowDimensions.y;

	// if your fbo isnt the same size as your back buffer
	glBlitFramebuffer(
		0, 0,
		readWidth, readHeight,
		0, 0,
		drawWidth, drawHeight,
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST
	);
}


//-------------------------------------------------------------------------------------------------
void Renderer::ApplyRenderState( RenderState const &renderState )
{
	SetCullFace( renderState.m_backfaceCullingEnabled );
	SetDepthWrite( renderState.m_depthWritingEnabled );
	SetDepthTest( renderState.m_depthTestingEnabled );
	SetBlending( renderState.m_blendingMode );
	SetDrawMode( renderState.m_drawMode );
	SetWindingOrder( renderState.m_windingClockwise );
	SetLineWidth( renderState.m_lineWidth );
}


//-------------------------------------------------------------------------------------------------
GLenum Renderer::GetOpenGLDrawMode( const ePrimitiveType& drawMode ) const
{
	if ( drawMode == ePrimitiveType_LINES )
		return GL_LINES;
	if ( drawMode == ePrimitiveType_POINTS )
		return GL_POINTS;
	if ( drawMode == ePrimitiveType_QUADS )
		return GL_QUADS;
	if ( drawMode == ePrimitiveType_TRIANGLES )
		return GL_TRIANGLES;
	if ( drawMode == ePrimitiveType_TRIANGLE_FAN )
		return GL_TRIANGLE_FAN;

	//Default, shouldn't happen
	return GL_POINTS;
}


//-------------------------------------------------------------------------------------------------
void Renderer::SetCullFace( bool enable )
{
	//No redundant actions
	if ( m_currentRenderState.m_backfaceCullingEnabled == enable )
		return;
	m_currentRenderState.m_backfaceCullingEnabled = enable;

	if ( enable )
		glEnable( GL_CULL_FACE );
	else
		glDisable( GL_CULL_FACE );
}


//-------------------------------------------------------------------------------------------------
void Renderer::SetDepthTest( bool enable )
{
	//No redundant actions
	if ( m_currentRenderState.m_depthTestingEnabled == enable )
		return;
	m_currentRenderState.m_depthTestingEnabled = enable;

	if ( enable )
		glEnable( GL_DEPTH_TEST );
	else
		glDisable( GL_DEPTH_TEST );
}


//-------------------------------------------------------------------------------------------------
void Renderer::SetDepthWrite( bool enable )
{
	//No redundant actions
	if ( m_currentRenderState.m_depthWritingEnabled == enable )
		return;
	m_currentRenderState.m_depthWritingEnabled = enable;

	if ( enable )
	{
		glDepthMask( GL_TRUE );
	}
	else
	{
		glDepthMask( GL_FALSE );
	}
}


//-------------------------------------------------------------------------------------------------
void Renderer::EnableAlphaTesting( float threshold /*= 0.5f */ )
{
	glAlphaFunc( GL_GREATER, threshold );
	glEnable( GL_ALPHA_TEST );
}


//-------------------------------------------------------------------------------------------------
void Renderer::DisableAlphaTesting( )
{
	glDisable( GL_ALPHA_TEST );
}


//-------------------------------------------------------------------------------------------------
void Renderer::SetBlending( const eBlending& setBlendingMode )
{
	//No redundant actions
	if ( m_currentRenderState.m_blendingMode == setBlendingMode )
		return;
	m_currentRenderState.m_blendingMode = setBlendingMode;

	if ( setBlendingMode == eBlending_NORMAL )
	{
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	}
	else if ( setBlendingMode == eBlending_ADDITIVE )
	{
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	}
	else if ( setBlendingMode == eBlending_SUBTRACTIVE )
	{
		glBlendFunc( GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA );
	}
	else if ( setBlendingMode == eBlending_INVERTED )
	{
		glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ZERO );
	}
}


//-------------------------------------------------------------------------------------------------
void Renderer::SetDrawMode( eDrawMode const &drawMode )
{
	//No redundant actions
	if ( m_currentRenderState.m_drawMode == drawMode )
		return;
	m_currentRenderState.m_drawMode = drawMode;

	if ( drawMode == eDrawMode_FULL )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}
	else if ( drawMode == eDrawMode_LINE )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	}
	else if ( drawMode == eDrawMode_POINT )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
	}
}


//-------------------------------------------------------------------------------------------------
void Renderer::SetWindingOrder( bool clockwise )
{
	//No redundant actions
	if ( m_currentRenderState.m_windingClockwise == clockwise )
		return;
	m_currentRenderState.m_windingClockwise = clockwise;

	if ( clockwise )
	{
		glCullFace( GL_FRONT );
	}
	else
	{
		glCullFace( GL_BACK );
	}
}


//-------------------------------------------------------------------------------------------------
void Renderer::SetLineWidth( float size )
{
	//#TODO: Make sure this works, No redundant changes
	//It was having problems because I think this has to be set everytime when the VAO is bound
// 	if ( m_currentLineWidth == size )
// 		return;

	m_currentLineWidth = size;
	glLineWidth( m_currentLineWidth );
}


//-------------------------------------------------------------------------------------------------
void Renderer::SetPointSize( float size )
{
	//No redundant changes
	if ( m_currentPointSize == size )
		return;

	m_currentPointSize = size;
	glPointSize( m_currentPointSize );
}


//-------------------------------------------------------------------------------------------------
std::string Renderer::GetOpenGLVersion( ) const
{
	char* openGLVersion = ( char* ) glGetString( GL_VERSION );
	return std::string( openGLVersion );
}


//-------------------------------------------------------------------------------------------------
std::string Renderer::GetGLSLVersion( ) const
{
	char* openGLSLVersion = ( char* ) glGetString( GL_SHADING_LANGUAGE_VERSION );
	return std::string( openGLSLVersion );
}


//-------------------------------------------------------------------------------------------------
BitmapFont const * Renderer::GetDefaultFont( ) const
{
	return BitmapFont::CreateOrGetFont( DEFAULT_FONT );
}


//-------------------------------------------------------------------------------------------------
Framebuffer * Renderer::GetActiveFBO( ) const
{
	return m_activeFBO;
}

//-------------------------------------------------------------------------------------------------
//#TODO: Implement default white texture?
//unsigned char plainWhiteTexel[3] = { 255, 255, 255 }; //A single white opaque texel
//m_plainWhiteTexture = CreateTextureFromData( "PlainWhite", plainWhiteTexel, IntVector2::ONE, 3 );