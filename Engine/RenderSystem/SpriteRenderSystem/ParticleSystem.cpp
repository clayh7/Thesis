#include "Engine/RenderSystem/SpriteRenderSystem/ParticleSystem.hpp"

#include "Engine/Core/Time.hpp"


//-------------------------------------------------------------------------------------------------
ParticleSystemResource::ParticleSystemResource( std::string const & resourceID )
	: m_id( resourceID )
{
}


//-------------------------------------------------------------------------------------------------
ParticleSystemResource::~ParticleSystemResource( )
{
	for( ParticleEmitterResource const * emitterResource : m_emitterResources )
	{
		delete emitterResource;
		emitterResource = nullptr;
	}
	m_emitterResources.clear( );
}


//-------------------------------------------------------------------------------------------------
void ParticleSystemResource::AddEmitter( ParticleEmitterResource const * emitter )
{
	m_emitterResources.push_back( emitter );
}


//-------------------------------------------------------------------------------------------------
ParticleSystem::ParticleSystem( ParticleSystemResource const * resource, int layer, Vector2f const & position )
	: m_isEnabled( true )
	, m_particleSystemResource( resource )
	, m_isDead( false )
	, m_position( position )
{
	for( ParticleEmitterResource const * emitterResource : resource->m_emitterResources )
	{
		ParticleEmitter * emitter = new ParticleEmitter( emitterResource );
		emitter->layerID = layer;
		m_emitters.push_back( emitter );
	}
}


//-------------------------------------------------------------------------------------------------
ParticleSystem::~ParticleSystem( )
{
	for( ParticleEmitter * deleteEmitter : m_emitters )
	{
		delete deleteEmitter;
		deleteEmitter = nullptr;
	}
	m_emitters.clear( );

	for( Particle * deleteParticle : m_particles )
	{
		delete deleteParticle;
		deleteParticle = nullptr;
	}
	m_particles.clear( );
}


//-------------------------------------------------------------------------------------------------
void ParticleSystem::Initialize( )
{
	//Spawn starting particles
	for( ParticleEmitter * emitter : m_emitters )
	{
		int spawnAmount = emitter->m_particleEmitterResource->initialSpawn;
		for( int initialIndex = 0; initialIndex < spawnAmount; ++initialIndex )
		{
			m_particles.push_back( emitter->SpawnParticle( m_position ) );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void ParticleSystem::Destroy( )
{
	m_isDead = true;
}


//-------------------------------------------------------------------------------------------------
void ParticleSystem::Update( )
{
	UpdateParticles( );
	CleanUpParticles( );
	if( m_isEnabled )
	{
		SpawnParticles( );
	}
}


//-------------------------------------------------------------------------------------------------
void ParticleSystem::UpdateParticles( )
{
	for( Particle * particle : m_particles )
	{
		particle->age += Time::DELTA_SECONDS;
		particle->velocity += particle->acceleration * Time::DELTA_SECONDS;
		particle->sprite->m_position += particle->velocity * Time::DELTA_SECONDS;
		particle->sprite->m_scale = Lerp( particle->startScale, particle->endScale, particle->age / particle->maxAge );
		float percentAge = ( particle->age / particle->maxAge );
		particle->sprite->m_color = Color::Lerp( particle->startColor, particle->endColor, percentAge );
	}
}


//-------------------------------------------------------------------------------------------------
void ParticleSystem::CleanUpParticles( )
{
	for( size_t particleIndex = 0; particleIndex < m_particles.size( ); ++particleIndex )
	{
		Particle * particle = m_particles[particleIndex];
		if( particle->age >= particle->maxAge )
		{
			delete particle;
			m_particles[particleIndex] = m_particles[m_particles.size( ) - 1];
			m_particles.pop_back( );
			--particleIndex;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void ParticleSystem::SpawnParticles( )
{
	if( m_isDead )
	{
		return;
	}

	for( ParticleEmitter * emitter : m_emitters )
	{
		emitter->m_emitTimer += Time::DELTA_SECONDS;
		while( emitter->m_emitTimer > emitter->m_particleEmitterResource->spawnRate )
		{
			m_particles.push_back( emitter->SpawnParticle( m_position ) );
			emitter->m_emitTimer -= emitter->m_particleEmitterResource->spawnRate;
		}
	}
}


//-------------------------------------------------------------------------------------------------
bool ParticleSystem::IsFinished( ) const
{
	return m_isDead && m_particles.size( ) == 0;
}


//-------------------------------------------------------------------------------------------------
void ParticleSystem::SetSpawnPosition( Vector2f const & position )
{
	m_position = position;
}