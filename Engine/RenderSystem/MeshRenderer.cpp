#include "Engine/RenderSystem/MeshRenderer.hpp"

#include "Engine/DebugSystem/BProfiler.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"
#include "Engine/Math/Matrix4f.hpp"
#include "Engine/RenderSystem/Material.hpp"
#include "Engine/RenderSystem/Mesh.hpp"
#include "Engine/RenderSystem/Renderer.hpp"
#include "Engine/RenderSystem/Uniform.hpp"
#include "Engine/RenderSystem/Attribute.hpp"
#include "Engine/RenderSystem/Texture.hpp"
#include "Engine/RenderSystem/Light.hpp"


//-------------------------------------------------------------------------------------------------
MeshRenderer::MeshRenderer( Transform const & transform /*= Transform( )*/, RenderState const & renderState /*= RenderState::BASIC_3D*/ )
	: m_transform( transform )
	, m_renderState( renderState )
	, m_vaoID( NULL )
	, m_mesh( nullptr )
	, m_material( nullptr )
{
	CreateVAO( );
}


//-------------------------------------------------------------------------------------------------
MeshRenderer::MeshRenderer( eMeshShape const & meshShape, Transform const &transform /*= Transform( )*/, RenderState const & renderState /*= RenderState::BASIC_3D*/ )
	: m_transform( transform )
	, m_renderState( renderState )
	, m_vaoID( NULL )
	, m_mesh( Mesh::GetMeshShape( meshShape ) )
	, m_material( Material::DEFAULT_MATERIAL )
{
	CreateVAO( );

	g_RenderSystem->BindMeshToVAO( m_vaoID, m_mesh, m_material );

	//Clear Matricies
	SetUniform( "uModel", GetModelMatrix( ) );
	SetUniform( "uView", Matrix4f::IDENTITY );
	SetUniform( "uProj", Matrix4f::IDENTITY );
}


//-------------------------------------------------------------------------------------------------
MeshRenderer::MeshRenderer( Mesh const * mesh, Transform const & transform /*= Transform( )*/, RenderState const & renderState /*= RenderState::BASIC_3D */ )
	: m_transform( transform )
	, m_renderState( renderState )
	, m_vaoID( NULL )
	, m_mesh( mesh )
	, m_material( Material::DEFAULT_MATERIAL )
{
	CreateVAO( );

	g_RenderSystem->BindMeshToVAO( m_vaoID, m_mesh, m_material );

	//Clear Matricies
	SetUniform( "uModel", GetModelMatrix( ) );
	SetUniform( "uView", Matrix4f::IDENTITY );
	SetUniform( "uProj", Matrix4f::IDENTITY );
}


//-------------------------------------------------------------------------------------------------
MeshRenderer::MeshRenderer( Mesh const * mesh, Material const * material, Transform const & transform /*= Transform() */, RenderState const & renderState /*= RenderState::BASIC_3D*/ )
	: m_transform( transform )
	, m_renderState( renderState )
	, m_vaoID( NULL )
	, m_mesh( mesh )
	, m_material( material )
{
	CreateVAO( );

	g_RenderSystem->BindMeshToVAO( m_vaoID, m_mesh, m_material );

	//Clear Matricies
	SetUniform( "uModel", GetModelMatrix() );
	SetUniform( "uView", Matrix4f::IDENTITY );
	SetUniform( "uProj", Matrix4f::IDENTITY );
}


//-------------------------------------------------------------------------------------------------
MeshRenderer::~MeshRenderer( )
{
	glDeleteVertexArrays( 1, &m_vaoID );

	for ( auto deleteMe : m_uniforms )
	{
		delete deleteMe.second;
		deleteMe.second = nullptr;
	}
	m_uniforms.clear( );
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::Update( bool onScreen /*= false*/ )
{
	SetUniform( "uModel", GetModelMatrix( ) );
	if ( !onScreen )
	{
		g_RenderSystem->SetMatrixUniforms( this );
	}
	else
	{
		SetUniform( "uView", Matrix4f::IDENTITY );
		SetUniform( "uProj", Matrix4f::IDENTITY );
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::Render( ) const
{
	g_RenderSystem->MeshRender( this );
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::CreateVAO( )
{
	//Create VAO
	glGenVertexArrays( 1, &m_vaoID );
	ASSERT_RECOVERABLE( m_vaoID != NULL, "VAO didn't generate." );;
}


//-------------------------------------------------------------------------------------------------
RenderState MeshRenderer::GetRenderState( ) const
{
	return m_renderState;
}


//-------------------------------------------------------------------------------------------------
//#TODO: Could pre-calculate for each time the model is transformed
Matrix4f MeshRenderer::GetModelMatrix( ) const
{
	return m_transform.GetModelMatrix( );
}


//-------------------------------------------------------------------------------------------------
Matrix4f MeshRenderer::GetViewMatrix( ) const
{
	auto found = m_uniforms.find( "uView" );
	return *((Matrix4f *) ( ( *found->second ).m_data ));
}


//-------------------------------------------------------------------------------------------------
Matrix4f MeshRenderer::GetProjectionMatrix( ) const
{
	auto found = m_uniforms.find( "uProj" );
	return *( (Matrix4f *) ( ( *found->second ).m_data ) );
}


//-------------------------------------------------------------------------------------------------
unsigned int MeshRenderer::GetGPUProgramID( ) const
{
	return m_material->GetGPUProgramID( );
}


//-------------------------------------------------------------------------------------------------
unsigned int MeshRenderer::GetSamplerID( ) const
{
	return m_material->GetSamplerID( );
}


//-------------------------------------------------------------------------------------------------
unsigned int MeshRenderer::GetVAOID( ) const
{
	return m_vaoID;
}


//-------------------------------------------------------------------------------------------------
unsigned int MeshRenderer::GetIBOID( ) const
{
	return m_mesh->GetIBOID( );
}


//-------------------------------------------------------------------------------------------------
std::vector<DrawInstruction> const & MeshRenderer::GetDrawInstructions( ) const
{
	return m_mesh->GetDrawInstructions();
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetLineWidth( float lineWidth )
{
	m_renderState.m_lineWidth = lineWidth;
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetPosition( Vector3f const &pos )
{
	m_transform.m_position = pos;
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetPosition( float xPos, float yPos, float zPos )
{
	m_transform.m_position = Vector3f( xPos, yPos, zPos );
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetTransform( Transform const & transform )
{
	m_transform = transform;
	SetUniform( "uModel", GetModelMatrix( ) );
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetMesh( Mesh const * mesh )
{
	m_mesh = mesh;
	RebindMeshAndMaterialToVAO( );
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetMaterial( Material const * material )
{
	m_material = material;
	RebindMeshAndMaterialToVAO( );
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetMeshAndMaterial( Mesh const * mesh, Material const * material )
{
	m_mesh = mesh;
	m_material = material;
	RebindMeshAndMaterialToVAO( );
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::RebindMeshAndMaterialToVAO( )
{
	if( m_mesh && m_material )
	{
		if( g_ProfilerSystem )
		{
			g_RenderSystem->BindMeshToVAO( m_vaoID, m_mesh, m_material );
			UpdateUniformBindpoints( );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::UpdateUniformBindpoints( )
{
	std::map<std::string, Uniform*> const & uniformList = m_material->GetUniformList( );

	//Reset all uniform bindpoints
	for ( auto uniform : m_uniforms )
	{
		auto foundUniform = uniformList.find( uniform.second->m_name );
		if ( foundUniform != uniformList.end( ) )
		{
			uniform.second->m_bindPoint = foundUniform->second->m_bindPoint;
		}
		else
		{
			uniform.second->m_bindPoint = -1;
		}
	}
}


//-------------------------------------------------------------------------------------------------
std::map<std::string, Uniform*> const & MeshRenderer::GetUniformList( ) const
{
	return m_uniforms;
}


//-------------------------------------------------------------------------------------------------
std::map<std::string, Uniform*> const & MeshRenderer::GetMaterialUniformList( ) const
{
	return m_material->GetUniformList( );
}


//-------------------------------------------------------------------------------------------------
std::map<size_t, Attribute*> const & MeshRenderer::GetMaterialAttributeList( ) const
{
	return m_material->GetAttributeList( );
}


//-------------------------------------------------------------------------------------------------
Vector3f MeshRenderer::GetPosition( )
{
	return m_transform.m_position;
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform( std::string const & uniformName, uint32_t uniformValue )
{
	//Get Uniform Map
	std::map<std::string, Uniform*> const & matUniformList = m_material->GetUniformList( );

	//Find uniform on material
	auto foundMatUniform = matUniformList.find( uniformName );
	if( foundMatUniform == matUniformList.end( ) )
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}

	//Find uniform on meshrenderer
	auto foundUniform = m_uniforms.find( uniformName );
	if( foundUniform == m_uniforms.end( ) )
	{
		size_t * newData = new size_t[foundMatUniform->second->m_size];
		newData[0] = ( uniformValue );
		Uniform * addUniform = new Uniform( *foundMatUniform->second, newData );
		m_uniforms.insert( std::pair<std::string, Uniform*>( uniformName, addUniform ) );
	}
	else
	{
		//Update data
		*( (size_t*) m_uniforms[uniformName]->m_data ) = uniformValue;
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform( std::string const & uniformName, int uniformValue )
{
	//Get Uniform Map
	std::map<std::string, Uniform*> const & matUniformList = m_material->GetUniformList( );

	//Find uniform on material
	auto foundMatUniform = matUniformList.find( uniformName );
	if( foundMatUniform == matUniformList.end( ) )
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}

	//Find uniform on meshrenderer
	auto foundUniform = m_uniforms.find( uniformName );
	if( foundUniform == m_uniforms.end( ) )
	{
		int * newData = new int[foundMatUniform->second->m_size];
		newData[0] = ( uniformValue );
		Uniform * addUniform = new Uniform( *foundMatUniform->second, newData );
		m_uniforms.insert( std::pair<std::string, Uniform*>( uniformName, addUniform ) );
	}
	else
	{
		//Update data
		*( (int*) m_uniforms[uniformName]->m_data ) = uniformValue;
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform( std::string const & uniformName, float uniformValue )
{
	//Get Uniform Map
	std::map<std::string, Uniform*> const & matUniformList = m_material->GetUniformList( );

	//Find uniform on material
	auto foundMatUniform = matUniformList.find( uniformName );
	if( foundMatUniform == matUniformList.end( ) )
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}

	//Find uniform on meshrenderer
	auto foundUniform = m_uniforms.find( uniformName );
	if( foundUniform == m_uniforms.end( ) )
	{
		Vector2f * newData = new Vector2f[foundMatUniform->second->m_size];
		newData[0] = ( uniformValue );
		Uniform * addUniform = new Uniform( *foundMatUniform->second, newData );
		m_uniforms.insert( std::pair<std::string, Uniform*>( uniformName, addUniform ) );
	}
	else
	{
		//Update data
		*( (Vector2f*) m_uniforms[uniformName]->m_data ) = uniformValue;
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform( std::string const & uniformName, Vector2f const &uniformValue )
{
	//Get Uniform Map
	std::map<std::string, Uniform*> const & matUniformList = m_material->GetUniformList( );

	//Find uniform on material
	auto foundMatUniform = matUniformList.find( uniformName );
	if( foundMatUniform == matUniformList.end( ) )
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}

	//Find uniform on meshrenderer
	auto foundUniform = m_uniforms.find( uniformName );
	if( foundUniform == m_uniforms.end( ) )
	{
		Vector2f * newData = new Vector2f[foundMatUniform->second->m_size];
		newData[0] = ( uniformValue );
		Uniform * addUniform = new Uniform( *foundMatUniform->second, newData );
		m_uniforms.insert( std::pair<std::string, Uniform*>( uniformName, addUniform ) );
	}
	else
	{
		//Update data
		*( (Vector2f*) m_uniforms[uniformName]->m_data ) = uniformValue;
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform( std::string const & uniformName, Vector3f const &uniformValue )
{
	//Get Uniform Map
	std::map<std::string, Uniform*> const & matUniformList = m_material->GetUniformList( );

	//Find uniform on material
	auto foundMatUniform = matUniformList.find( uniformName );
	if( foundMatUniform == matUniformList.end( ) )
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}

	//Find uniform on meshrenderer
	auto foundUniform = m_uniforms.find( uniformName );
	if( foundUniform == m_uniforms.end( ) )
	{
		Vector3f * newData = new Vector3f[foundMatUniform->second->m_size];
		newData[0] = ( uniformValue );
		Uniform * addUniform = new Uniform( *foundMatUniform->second, newData );
		m_uniforms.insert( std::pair<std::string, Uniform*>( uniformName, addUniform ) );
	}
	else
	{
		//Update data
		*( (Vector3f*) m_uniforms[uniformName]->m_data ) = uniformValue;
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform( std::string const & uniformName, Vector4f const &uniformValue )
{
	//Get Uniform Map
	std::map<std::string, Uniform*> const & matUniformList = m_material->GetUniformList( );

	//Find uniform on material
	auto foundMatUniform = matUniformList.find( uniformName );
	if( foundMatUniform == matUniformList.end( ) )
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}

	//Find uniform on meshrenderer
	auto foundUniform = m_uniforms.find( uniformName );
	if( foundUniform == m_uniforms.end( ) )
	{
		Vector4f * newData = new Vector4f[foundMatUniform->second->m_size];
		newData[0] = ( uniformValue );
		Uniform * addUniform = new Uniform( *foundMatUniform->second, newData );
		m_uniforms.insert( std::pair<std::string, Uniform*>( uniformName, addUniform ) );
	}
	else
	{
		//Update data
		*( (Vector4f*) m_uniforms[uniformName]->m_data ) = uniformValue;
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform( std::string const & uniformName, Vector4i const & uniformValue )
{
	//Get Uniform Map
	std::map<std::string, Uniform*> const & matUniformList = m_material->GetUniformList( );

	//Find uniform on material
	auto foundMatUniform = matUniformList.find( uniformName );
	if( foundMatUniform == matUniformList.end( ) )
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}

	//Find uniform on meshrenderer
	auto foundUniform = m_uniforms.find( uniformName );
	if( foundUniform == m_uniforms.end( ) )
	{
		Vector4i * newData = new Vector4i[foundMatUniform->second->m_size];
		newData[0] = ( uniformValue );
		Uniform * addUniform = new Uniform( *foundMatUniform->second, newData );
		m_uniforms.insert( std::pair<std::string, Uniform*>( uniformName, addUniform ) );
	}
	else
	{
		//Update data
		*( (Vector4i*) m_uniforms[uniformName]->m_data ) = uniformValue;
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform( std::string const & uniformName, Matrix4f const & uniformValue )
{
	//Get Uniform Map
	std::map<std::string, Uniform*> const & matUniformList = m_material->GetUniformList( );

	//Find uniform on material
	auto foundMatUniform = matUniformList.find( uniformName );
	if( foundMatUniform == matUniformList.end( ) )
	{
		//ERROR_AND_DIE( "Uniform doesn't exist on Material" );
		return;
	}

	//Find uniform on meshrenderer
	auto foundUniform = m_uniforms.find( uniformName );
	if( foundUniform == m_uniforms.end( ) )
	{
		Matrix4f * newData = new Matrix4f[foundMatUniform->second->m_size];
		newData[0] = ( uniformValue );
		Uniform * addUniform = new Uniform( *foundMatUniform->second, newData );
		m_uniforms.insert( std::pair<std::string, Uniform*>( uniformName, addUniform ) );
	}
	else
	{
		//Update data
		*( (Matrix4f*) m_uniforms[uniformName]->m_data ) = uniformValue;
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform( std::string const & uniformName, Color const & uniformValue )
{
	//Get Uniform Map
	std::map<std::string, Uniform*> const & matUniformList = m_material->GetUniformList( );

	//Find uniform on material
	auto foundMatUniform = matUniformList.find( uniformName );
	if( foundMatUniform == matUniformList.end( ) )
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}

	//Find uniform on meshrenderer
	auto foundUniform = m_uniforms.find( uniformName );
	if( foundUniform == m_uniforms.end( ) )
	{
		Vector4f * newData = new Vector4f[foundMatUniform->second->m_size];
		newData[0] = ( uniformValue.GetVector4f( ) );
		Uniform * addUniform = new Uniform( *foundMatUniform->second, newData );
		m_uniforms.insert( std::pair<std::string, Uniform*>( uniformName, addUniform ) );
	}
	else
	{
		//Update data
		*( (Vector4f*) m_uniforms[uniformName]->m_data ) = uniformValue.GetVector4f( );
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform( std::string const & uniformName, std::string const & uniformValue )
{
	//Get Uniform Map
	std::map<std::string, Uniform*> const & matUniformList = m_material->GetUniformList( );
	Texture const * generatedTexture = Texture::CreateOrLoadTexture( uniformValue );

	//Find uniform on material
	auto foundMatUniform = matUniformList.find( uniformName );
	if( foundMatUniform == matUniformList.end( ) )
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}

	//Find uniform on meshrenderer
	auto foundUniform = m_uniforms.find( uniformName );
	if( foundUniform == m_uniforms.end( ) )
	{
		size_t * newData = new size_t[foundMatUniform->second->m_size];
		newData[0] = ( generatedTexture->m_openglTextureID );
		Uniform * addUniform = new Uniform( *foundMatUniform->second, newData );
		m_uniforms.insert( std::pair<std::string, Uniform*>( uniformName, addUniform ) );
	}
	else
	{
		//Update data
		*( (size_t*) m_uniforms[uniformName]->m_data ) = generatedTexture->m_openglTextureID;
	}
}


//-------------------------------------------------------------------------------------------------
void MeshRenderer::SetUniform( std::string const & uniformName, Texture const * uniformValue )
{
	//Get Uniform Map
	std::map<std::string, Uniform*> const & matUniformList = m_material->GetUniformList( );

	//Find uniform on material
	auto foundMatUniform = matUniformList.find( uniformName );
	if( foundMatUniform == matUniformList.end( ) )
	{
		ERROR_AND_DIE( "Uniform doesn't exist on Material" );
	}

	//Find uniform on meshrenderer
	auto foundUniform = m_uniforms.find( uniformName );
	if( foundUniform == m_uniforms.end( ) )
	{
		size_t * newData = new size_t[foundMatUniform->second->m_size];
		newData[0] = ( uniformValue->m_openglTextureID );
		Uniform * addUniform = new Uniform( *foundMatUniform->second, newData );
		m_uniforms.insert( std::pair<std::string, Uniform*>( uniformName, addUniform ) );
	}
	else
	{
		//Update data
		*( (size_t*) m_uniforms[uniformName]->m_data ) = uniformValue->m_openglTextureID;
	}
}


//-------------------------------------------------------------------------------------------------
//#TODO: Redo map copy here (it's slow)
void MeshRenderer::SetUniform( std::string const & uniformName, int * uniformValue )
{
	std::map<std::string, Uniform*> uniformList = m_material->GetUniformList( );

	//Doesn't exist yet, lets make space for it and assign it to the data
	if ( uniformList[uniformName] != nullptr && m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( ( Uniform* ) uniformList[uniformName] )->m_size;
		size_t dataSize = sizeof( int ) * uniformSize;
		int *data = ( int* ) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
			(
				( ( Uniform* ) uniformList[uniformName] )->m_bindPoint,
				( ( Uniform* ) uniformList[uniformName] )->m_length,
				uniformSize,
				( ( Uniform* ) uniformList[uniformName] )->m_type,
				( ( Uniform* ) uniformList[uniformName] )->m_name,
				data //Setting new value
			);
	}
	
	//update the data
	else if ( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( ( Uniform* ) uniformList[uniformName] )->m_size;
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
void MeshRenderer::SetUniform( std::string const &uniformName, float *uniformValue )
{
	std::map<std::string, Uniform*> uniformList = m_material->GetUniformList( );

	//Doesn't exist yet, lets make space for it and assign it to the data
	if ( uniformList[uniformName] != nullptr && m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( ( Uniform* ) uniformList[uniformName] )->m_size;
		size_t dataSize = sizeof( float ) * uniformSize;
		float *data = ( float* ) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
			(
				( ( Uniform* ) uniformList[uniformName] )->m_bindPoint,
				( ( Uniform* ) uniformList[uniformName] )->m_length,
				uniformSize,
				( ( Uniform* ) uniformList[uniformName] )->m_type,
				( ( Uniform* ) uniformList[uniformName] )->m_name,
				data
			);
	}

	//update the data
	else if ( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( ( Uniform* ) uniformList[uniformName] )->m_size;
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
void MeshRenderer::SetUniform( std::string const &uniformName, Vector2f *uniformValue )
{
	std::map<std::string, Uniform*> uniformList = m_material->GetUniformList( );

	//Doesn't exist yet, lets make space for it and assign it to the data
	if ( uniformList[uniformName] != nullptr && m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( ( Uniform* ) uniformList[uniformName] )->m_size;
		size_t dataSize = sizeof( Vector2f ) * uniformSize;
		Vector2f *data = ( Vector2f* ) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
			(
				( ( Uniform* ) uniformList[uniformName] )->m_bindPoint,
				( ( Uniform* ) uniformList[uniformName] )->m_length,
				uniformSize,
				( ( Uniform* ) uniformList[uniformName] )->m_type,
				( ( Uniform* ) uniformList[uniformName] )->m_name,
				data
				);
	}

	//update the data
	else if ( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( ( Uniform* ) uniformList[uniformName] )->m_size;
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
void MeshRenderer::SetUniform( std::string const &uniformName, Vector3f *uniformValue )
{
	std::map<std::string, Uniform*> uniformList = m_material->GetUniformList( );

	//Doesn't exist yet, lets make space for it and assign it to the data
	if ( uniformList[uniformName] != nullptr && m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( ( Uniform* ) uniformList[uniformName] )->m_size;
		size_t dataSize = sizeof( Vector3f ) * uniformSize;
		Vector3f *data = ( Vector3f* ) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
			(
				( ( Uniform* ) uniformList[uniformName] )->m_bindPoint,
				( ( Uniform* ) uniformList[uniformName] )->m_length,
				uniformSize,
				( ( Uniform* ) uniformList[uniformName] )->m_type,
				( ( Uniform* ) uniformList[uniformName] )->m_name,
				data
			);
	}

	//update the data
	else if ( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( ( Uniform* ) uniformList[uniformName] )->m_size;
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
void MeshRenderer::SetUniform( std::string const & uniformName, Vector4f * uniformValue )
{
	std::map<std::string, Uniform*> uniformList = m_material->GetUniformList( );

	//Doesn't exist yet, lets make space for it and assign it to the data
	if ( uniformList[uniformName] != nullptr && m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( ( Uniform* ) uniformList[uniformName] )->m_size;
		size_t dataSize = sizeof( Vector4f ) * uniformSize;
		Vector4f *data = ( Vector4f* ) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
			(
				( ( Uniform* ) uniformList[uniformName] )->m_bindPoint,
				( ( Uniform* ) uniformList[uniformName] )->m_length,
				uniformSize,
				( ( Uniform* ) uniformList[uniformName] )->m_type,
				( ( Uniform* ) uniformList[uniformName] )->m_name,
				data
			);
	}

	//update the data
	else if ( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( ( Uniform* ) uniformList[uniformName] )->m_size;
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
void MeshRenderer::SetUniform( std::string const &uniformName, Matrix4f * uniformValue )
{
	std::map<std::string, Uniform*> uniformList = m_material->GetUniformList( );

	//Doesn't exist yet, lets make space for it and assign it to the data
	if( uniformList[uniformName] != nullptr && m_uniforms[uniformName] == nullptr )
	{
		int uniformSize = ( (Uniform*) uniformList[uniformName] )->m_size;
		size_t dataSize = sizeof( Matrix4f ) * uniformSize;
		Matrix4f *data = (Matrix4f*) malloc( dataSize );
		memcpy( data, uniformValue, dataSize );

		m_uniforms[uniformName] = new Uniform
		(
			( (Uniform*) uniformList[uniformName] )->m_bindPoint,
			( (Uniform*) uniformList[uniformName] )->m_length,
			uniformSize,
			( (Uniform*) uniformList[uniformName] )->m_type,
			( (Uniform*) uniformList[uniformName] )->m_name,
			data
		);
	}

	//update the data
	else if( m_uniforms[uniformName] != nullptr )
	{
		int uniformSize = ( (Uniform*) uniformList[uniformName] )->m_size;
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
void MeshRenderer::SetUniform( std::vector<Light> const &uniformLights, int lightCount )
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
		lightColor[lightIndex] = uniformLights[lightIndex].m_lightColor.GetVector4f();
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