#pragma once

#include <string>
#include <vector>
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEmitter.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Particle.hpp"


//-------------------------------------------------------------------------------------------------
class Vector2f;


//-------------------------------------------------------------------------------------------------
class ParticleSystemResource
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	std::string m_id;
	std::vector<ParticleEmitterResource const *> m_emitterResources;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	ParticleSystemResource( std::string const & resourceID );
	~ParticleSystemResource( );

	void AddEmitter( ParticleEmitterResource const * emitter );
};


//-------------------------------------------------------------------------------------------------
class ParticleSystem
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	bool m_isEnabled;

private: 
	ParticleSystemResource const * m_particleSystemResource;
	bool m_isDead;
	Vector2f m_position;
	std::vector<ParticleEmitter*> m_emitters;
	std::vector<Particle*> m_particles;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	ParticleSystem( ParticleSystemResource const * resource, int layer, Vector2f const & position );
	~ParticleSystem( );

	void Initialize( );
	void Destroy( );
	void Update( );
	void UpdateParticles( );
	void CleanUpParticles( );
	void SpawnParticles( );
	
	bool IsFinished( ) const;
	void SetSpawnPosition( Vector2f const & position );
};
