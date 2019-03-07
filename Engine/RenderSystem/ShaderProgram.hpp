#pragma once

#include <map>
#include <string>
#include "Engine/Memory/MemoryAnalytics.hpp"


//-------------------------------------------------------------------------------------------------
class ShaderProgram
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
private:
	static std::map<size_t, ShaderProgram*, std::less<size_t>, UntrackedAllocator<std::pair<size_t, ShaderProgram*>>> s_shaderProgramRegistry;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static ShaderProgram const * CreateOrGetShaderProgram( std::string const & vsFilePath, std::string const & fsFilePath );
	static void DestroyRegistry( );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	unsigned int m_programID;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
private:
	ShaderProgram( std::string const & vsFilePath, std::string const & fsFilePath );

public:
	~ShaderProgram( );
	unsigned int GetShaderProgramID( ) const;
};