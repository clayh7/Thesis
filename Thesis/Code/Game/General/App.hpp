#pragma once

#include <vector>
#include "Engine/Math/Vector2i.hpp"


//-------------------------------------------------------------------------------------------------
class App;
class Framebuffer;
class Mesh;
class Material;
class MeshRenderer;
class Vector2f;
class Vector3f;


//-------------------------------------------------------------------------------------------------
extern App* g_AppSystem;


//-------------------------------------------------------------------------------------------------
class App
{
private:
	Framebuffer * m_fbo;
	Mesh const * m_screenMesh;
	Material * m_screenMaterial;
	MeshRenderer * m_screenEffect;

public:
	App( );
	~App( );
	void Update( );
	void UpdateInputs( );
	void Render( ) const;

	void StartClientServer( );
};