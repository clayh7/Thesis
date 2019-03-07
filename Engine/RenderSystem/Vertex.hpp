#pragma once

#include "Engine/Math/Vector2f.hpp"
#include "Engine/Math/Vector3f.hpp"
#include "Engine/Math/Vector4i.hpp"
#include "Engine/RenderSystem/Color.hpp"


//-------------------------------------------------------------------------------------------------
enum eVertexType
{
	eVertexType_PC,
	eVertexType_PCU,
	eVertexType_PCUN,
	eVertexType_PCUTB,
	eVertexType_PCUTBWI,
	eVertexType_SPRITE,
};


//-------------------------------------------------------------------------------------------------
class Vertex_Master
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Vector3f m_position;
	Color m_color;
	Vector2f m_uv0;
	Vector2f m_uv1;
	Vector3f m_tangent;
	Vector3f m_bitangent;
	Vector3f m_normal;
	Vector4f m_boneWeights;
	Vector4i m_boneIndicies;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vertex_Master
	(
		Vector3f const & position = Vector3f::ZERO,
		Color const & color = Color::WHITE,
		Vector2f const & uv0 = Vector2f::ZERO,
		Vector2f const & uv1 = Vector2f::ZERO,
		Vector3f const & tangent = Vector3f( 1.f, 0.f, 0.f ),
		Vector3f const & bitangent = Vector3f( 0.f, 1.f, 0.f ),
		Vector3f const & normal = Vector3f( 0.f, 0.f, 1.f ),
		Vector4f const & boneWeights = Vector4f( 1.f, 0.f, 0.f, 0.f ),
		Vector4i const & boneIndicies = Vector4i( 0, 0, 0, 0 )
	);

	bool Equals( Vertex_Master const & check );
};


//-------------------------------------------------------------------------------------------------
class Vertex_SPRITE
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Vector3f m_position;
	Color m_color;
	Vector2f m_uv;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vertex_SPRITE
	(
		Vector3f const & position = Vector3f::ZERO,
		Color const & color = Color::WHITE,
		Vector2f const & uv = Vector2f::ZERO
	);
};


//-------------------------------------------------------------------------------------------------
class Vertex_PCUTBWI
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Vector3f m_position;
	Color m_color;
	Vector2f m_uv;
	Vector3f m_tangent;
	Vector3f m_bitangent;
	Vector4f m_boneWeights;
	Vector4i m_boneIndicies;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vertex_PCUTBWI
	(
		Vector3f const & position = Vector3f::ZERO,
		Color const & color = Color::WHITE,
		Vector2f const & uv = Vector2f::ZERO,
		Vector3f const & tangent = Vector3f( 1.f, 0.f, 0.f ),
		Vector3f const & bitangent = Vector3f( 0.f, 1.f, 0.f ),
		Vector4f const & boneWeights = Vector4f( 1.f, 0.f, 0.f, 0.f ),
		Vector4i const & boneIndicies = Vector4i( 0, 0, 0, 0 )
	);
};


//-------------------------------------------------------------------------------------------------
class Vertex_PCUTB
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Vector3f m_position;
	Color m_color;
	Vector2f m_uv;
	Vector3f m_tangent;
	Vector3f m_bitangent;

	//-------------------------------------------------------------------------------------------------
	// Functions
	//-------------------------------------------------------------------------------------------------
public:
	Vertex_PCUTB
	(
		Vector3f const & position = Vector3f::ZERO,
		Color const & color = Color::WHITE,
		Vector2f const & uv = Vector2f::ZERO,
		Vector3f const & tangent = Vector3f(1.f,0.f,0.f),
		Vector3f const & bitangent = Vector3f(0.f,1.f,0.f)
	);
};


//-------------------------------------------------------------------------------------------------
class Vertex_PCUN
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Vector3f m_position;
	Color m_color;
	Vector2f m_uv;
	Vector3f m_normal;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vertex_PCUN
	(
		Vector3f const & position = Vector3f::ZERO,
		Color const & color = Color::WHITE,
		Vector2f const & uv = Vector2f::ZERO,
		Vector3f const & normal = Vector3f( 0.f, 0.f, 1.f )
	);
};


//-------------------------------------------------------------------------------------------------
class Vertex_PCU
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Vector3f m_position;
	Color m_color;
	Vector2f m_uv;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vertex_PCU
	(
		Vector3f const & position = Vector3f::ZERO,
		Color const & color = Color::WHITE,
		Vector2f const & uv = Vector2f::ZERO
	);
};


//-------------------------------------------------------------------------------------------------
class Vertex_PC
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Vector3f m_position;
	Color m_color;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Vertex_PC
	(
		Vector3f const & position = Vector3f::ZERO,
		Color const & color = Color::WHITE
	);
};