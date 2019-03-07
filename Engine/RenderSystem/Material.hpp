#pragma once

#include <string>
#include <vector>
#include <map>
#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Vector4f.hpp"
#include "Engine/Math/Vector4i.hpp"
#include "Engine/RenderSystem/Color.hpp"
#include "Engine/RenderSystem/Light.hpp"


//-------------------------------------------------------------------------------------------------
class Texture;
class ShaderProgram;
class Color;
class Attribute;
class Matrix4f;
class Uniform;


//-------------------------------------------------------------------------------------------------
class Material
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * DEFAULT_VERT;
	static char const * DEFAULT_FRAG;
	static Material const * DEFAULT_MATERIAL;
	static Material * FONT_MATERIAL; //Treat it as a const variable (not const for some setup)

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void InitializeDefaultMaterials( );
	static void DestroyDefaultMaterials( );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	unsigned int m_samplerID;
	ShaderProgram const * m_program;

	//Uniforms
	std::map<size_t, Attribute*> m_attributes;
	std::map<std::string, Uniform*> m_uniforms;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Material( );
	Material( ShaderProgram const * program );
	Material( std::string const & vsFilePath, std::string const & fsFilePath );
	~Material( );

	//Get Uniforms
	std::map<size_t, Attribute*> const & GetAttributeList( ) const;
	std::map<std::string, Uniform*> const & GetUniformList( ) const;
	unsigned int GetGPUProgramID( ) const;
	unsigned int GetSamplerID( ) const;

	//#TODO: Refactor to use templates
	//Set Uniforms
	void SetUniform( std::string const & uniformName, int uniformValue );
	void SetUniform( std::string const & uniformName, float uniformValue );
	void SetUniform( std::string const & uniformName, Vector3f const & uniformValue );
	void SetUniform( std::string const & uniformName, Vector4f const & uniformValue );
	void SetUniform( std::string const & uniformName, Vector4i const & uniformValue );
	void SetUniform( std::string const & uniformName, Matrix4f const & uniformValue );
	void SetUniform( std::string const & uniformName, Color const & uniformValue );
	void SetUniform( std::string const & uniformName, std::string const & uniformValue );
	void SetUniform( std::string const & uniformName, unsigned int uniformValue );
	void SetUniform( std::string const & uniformName, Texture const * uniformValue );
										 
	void SetUniform( std::string const & uniformName, int * uniformValue );
	void SetUniform( std::string const & uniformName, float * uniformValue );
	void SetUniform( std::string const & uniformName, Vector2f * uniformValue );
	void SetUniform( std::string const & uniformName, Vector3f * uniformValue );
	void SetUniform( std::string const & uniformName, Vector4f * uniformValue );
	void SetUniform( std::string const & uniformName, Matrix4f * uniformValue );

	//Custom Uniform Arrays
	void SetUniform( std::vector<Light> const &uniformLights, int lightCount );
	void SetUniform( std::vector<Matrix4f> const &uniformMatricies, int matrixCount );

	void UpdateBindpoints( );
};