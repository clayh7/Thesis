#include "Engine/RenderSystem/Vertex.hpp"
#include "Engine/DebugSystem/ErrorWarningAssert.hpp"


//-------------------------------------------------------------------------------------------------
Vertex_Master::Vertex_Master( Vector3f const & position /*= Vector3f::ZERO*/,
	Color const & color /*= Color::WHITE*/,
	Vector2f const & uv0 /*= Vector2f::ZERO*/,
	Vector2f const & uv1 /*= Vector2f::ZERO*/,
	Vector3f const & tangent /*= Vector3f( 1.f, 0.f, 0.f )*/,
	Vector3f const & bitangent /*= Vector3f( 0.f, 1.f, 0.f )*/,
	Vector3f const & normal /*= Vector3f( 0.f, 0.f, 1.f )*/,
	Vector4f const & boneWeights /*= Vector4f( 1.f, 0.f, 0.f, 0.f )*/,
	Vector4i const & boneIndicies /*= Vector4i( 0, 0, 0, 0 ) */ )
	: m_position( position )
	, m_color( color )
	, m_uv0( uv0 )
	, m_uv1( uv1 )
	, m_tangent( tangent )
	, m_bitangent( bitangent )
	, m_normal( normal )
	, m_boneWeights( boneWeights )
	, m_boneIndicies( boneIndicies )
{
}


//-------------------------------------------------------------------------------------------------
bool Vertex_Master::Equals( Vertex_Master const & check )
{
	return ( m_uv0 == check.m_uv0 )
		&& ( m_position == check.m_position )
		&& ( m_color == check.m_color )
		&& ( m_uv1 == check.m_uv1 )
		&& ( m_tangent == check.m_tangent )
		&& ( m_bitangent == check.m_bitangent )
		&& ( m_normal == check.m_normal )
		&& ( m_boneWeights == check.m_boneWeights )
		&& ( m_boneIndicies == check.m_boneIndicies );

	//bool checkAmount = (sizeof( Vertex_Master ) == 68);
	//ASSERT_OR_DIE( checkAmount, "Update Vertex_Master Equals function." );
}


//-------------------------------------------------------------------------------------------------
Vertex_SPRITE::Vertex_SPRITE( Vector3f const & position /*= Vector3f::ZERO*/, Color const & color /*= Color::WHITE*/, Vector2f const & uv /*= Vector2f::ZERO */ )
	: m_position( position )
	, m_color( color )
	, m_uv( uv )
{

}


//-------------------------------------------------------------------------------------------------
Vertex_PCUTBWI::Vertex_PCUTBWI( Vector3f const & position /*= Vector3f::ZERO*/, Color const & color /*= Color::WHITE*/, Vector2f const & uv /*= Vector2f::ZERO*/, Vector3f const & tangent /*= Vector3f( 1.f, 0.f, 0.f )*/, Vector3f const & bitangent /*= Vector3f( 0.f, 1.f, 0.f )*/, Vector4f const & boneWeights /*= Vector4f( 1.f, 0.f, 0.f, 0.f )*/, Vector4i const & boneIndicies /*= Vector4i( 0, 0, 0, 0 ) */ )
	: m_position( position )
	, m_color( color )
	, m_uv( uv )
	, m_tangent( tangent )
	, m_bitangent( bitangent )
	, m_boneWeights( boneWeights )
	, m_boneIndicies( boneIndicies )
{
}


//-------------------------------------------------------------------------------------------------
Vertex_PCUTB::Vertex_PCUTB( Vector3f const & position /*= Vector3::ZERO*/, Color const & color /*= Rgba::WHITE*/, Vector2f const & uv /*= Vector2::ZERO*/, Vector3f const & tangent /*= Vector3(1.f,0.f,0.f)*/, Vector3f const &biTangent /*= Vector3(0.f,1.f,0.f) */ )
	: m_position( position )
	, m_color( color )
	, m_uv( uv )
	, m_tangent( tangent )
	, m_bitangent( biTangent )
{
}


//-------------------------------------------------------------------------------------------------
Vertex_PCUN::Vertex_PCUN( const Vector3f& position /*= Vector3f::ZERO*/, const Color& color /*= Color::WHITE*/, const Vector2f& uv /*= Vector2f::ZERO*/, Vector3f const &normal /*= Vector3f( 0.f, 0.f, 1.f ) */ )
	: m_position( position )
	, m_color( color )
	, m_uv( uv )
	, m_normal( normal )
{
}


//-------------------------------------------------------------------------------------------------
Vertex_PCU::Vertex_PCU( const Vector3f& position /*= Vector3::ZERO*/, const Color& color /*= Rgba::WHITE */, const Vector2f& uv )
: m_position( position )
, m_color( color )
, m_uv( uv )
{
}


//-------------------------------------------------------------------------------------------------
Vertex_PC::Vertex_PC( const Vector3f& position /*= Vector3::ZERO*/, const Color& color /*= Rgba::WHITE */ )
: m_position( position )
, m_color( color )
{
}
