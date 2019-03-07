#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEmitter.hpp"

#include "Engine/Utils/MathUtils.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Particle.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteGameRenderer.hpp"


//-------------------------------------------------------------------------------------------------
ParticleEmitterResource::ParticleEmitterResource( std::string const & spriteName )
	: spriteID( spriteName )
	, initialSpawn( 0 )
	, spawnRate( 0.f )
	, offsetPosition( Vector2f::ZERO, Vector2f::ZERO )
	, totalLifetimeSeconds( 0.f, 0.f )
	, startScale( 1.f, 1.f )
	, endScale( 1.f, 1.f )
	, constantScale( true )
	, startSpeed( 0.f )
	, startDirectionDegrees( 0.f, 360.f )
	, startColor( Color::WHITE )
	, endColor( Color::WHITE )
{
}


//-------------------------------------------------------------------------------------------------
ParticleEmitterResource::~ParticleEmitterResource( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
ParticleEmitter::ParticleEmitter( ParticleEmitterResource const * resource )
	: m_particleEmitterResource( resource )
	, m_emitTimer( 0.f )
{
}


//-------------------------------------------------------------------------------------------------
ParticleEmitter::~ParticleEmitter( )
{
	//Nothing
}


//-------------------------------------------------------------------------------------------------
Particle * ParticleEmitter::SpawnParticle( Vector2f const & position ) const
{
	Particle * spawn = new Particle( );
	spawn->sprite = SpriteGameRenderer::Create( m_particleEmitterResource->spriteID, layerID );
	spawn->startScale = m_particleEmitterResource->startScale.GetRandom( );
	if( m_particleEmitterResource->constantScale )
	{
		spawn->endScale = spawn->startScale;
	}
	else
	{
		spawn->endScale = m_particleEmitterResource->endScale.GetRandom( );
	}
	spawn->sprite->m_scale = spawn->startScale;
	spawn->sprite->m_position = position + m_particleEmitterResource->offsetPosition.GetRandom( );
	spawn->sprite->m_color = m_particleEmitterResource->startColor;
	spawn->sprite->SetEnabled( true );
	spawn->acceleration = Vector2f::ZERO;
	float directionDegrees = m_particleEmitterResource->startDirectionDegrees.GetRandom( );
	spawn->velocity = UnitVectorFromDegrees( directionDegrees ) * m_particleEmitterResource->startSpeed.GetRandom( );
	spawn->age = 0.f;
	spawn->maxAge = m_particleEmitterResource->totalLifetimeSeconds.GetRandom( );
	spawn->startColor = m_particleEmitterResource->startColor;
	spawn->endColor = m_particleEmitterResource->endColor;
	return spawn;
}