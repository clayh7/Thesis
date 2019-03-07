#include "Engine/RenderSystem/ShaderProgram.hpp"

#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"


//-------------------------------------------------------------------------------------------------
STATIC std::map<size_t, ShaderProgram*, std::less<size_t>, UntrackedAllocator<std::pair<size_t, ShaderProgram*>>> ShaderProgram::s_shaderProgramRegistry;


//-------------------------------------------------------------------------------------------------
STATIC ShaderProgram const * ShaderProgram::CreateOrGetShaderProgram( const std::string & vsFilePath, const std::string & fsFilePath )
{
	std::string totalString;
	totalString.append( vsFilePath );
	totalString.append( fsFilePath );
	size_t nameHash = std::hash<std::string>{ }( totalString );

	auto found = s_shaderProgramRegistry.find( nameHash );
	if( found != s_shaderProgramRegistry.end( ) )
	{
		return found->second;
	}
	else
	{
		s_shaderProgramRegistry[nameHash] = new ShaderProgram( vsFilePath, fsFilePath );
		return s_shaderProgramRegistry[nameHash];
	}
}


//-------------------------------------------------------------------------------------------------
STATIC void ShaderProgram::DestroyRegistry( )
{
	for( auto shaderIndex = s_shaderProgramRegistry.begin( ); shaderIndex != s_shaderProgramRegistry.end( ); ++shaderIndex )
	{
		delete shaderIndex->second;
		shaderIndex->second = nullptr;
	}
}


//-------------------------------------------------------------------------------------------------
ShaderProgram::ShaderProgram( std::string const &vsFilePath, std::string const &fsFilePath )
{
	//Build Vert and Frag Shader
	unsigned int vertexShaderID = g_RenderSystem->CreateShader( vsFilePath, GL_VERTEX_SHADER );
	unsigned int fragmentShaderID = g_RenderSystem->CreateShader( fsFilePath, GL_FRAGMENT_SHADER );
	ASSERT_RECOVERABLE( vertexShaderID != NULL && fragmentShaderID != NULL, "Vertex or Fragment File not loaded correctly." );

	//Build Program
	m_programID = g_RenderSystem->CreateAndLinkProgram( vertexShaderID, fragmentShaderID, fsFilePath );
	ASSERT_RECOVERABLE( m_programID != NULL, "Program did not link." );

	//Release Vert and Frag Shader
	glDeleteShader( vertexShaderID );
	glDeleteShader( fragmentShaderID );
}


//-------------------------------------------------------------------------------------------------
ShaderProgram::~ShaderProgram( )
{
	glDeleteProgram( m_programID );
}


//-------------------------------------------------------------------------------------------------
unsigned int ShaderProgram::GetShaderProgramID( ) const
{
	return m_programID;
}