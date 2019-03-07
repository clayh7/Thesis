#include "Engine/RenderSystem/SpriteRenderSystem/Particle.hpp"


//-------------------------------------------------------------------------------------------------
Particle::Particle( )
	: sprite( nullptr )
	, velocity( Vector2f::ZERO )
	, acceleration( Vector2f::ZERO )
	, age( 0.f )
	, maxAge( 0.f )
{
}


//-------------------------------------------------------------------------------------------------
Particle::~Particle( )
{
	delete sprite;
	sprite = nullptr;
}


//-------------------------------------------------------------------------------------------------
void Particle::SetPosition( Vector2f const & position )
{
	sprite->SetPosition( position );
}
