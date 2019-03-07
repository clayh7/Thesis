#pragma once

#include <map>
#include <vector>
#include "Engine/Math/Vector2f.hpp"


//-------------------------------------------------------------------------------------------------
class ParticleEngine;
class ParticleSystem;
class ParticleSystemResource;


//-------------------------------------------------------------------------------------------------
extern ParticleEngine * g_ParticleEngine;


//-------------------------------------------------------------------------------------------------
class ParticleEngine
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
private:
	std::map<size_t, ParticleSystemResource*> m_particleSystemResourceDatabase;
	std::vector<ParticleSystem*> m_activeParticleSystems;

	ParticleSystemResource const * GetParticleSystemResource( std::string const & resourceName );

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	ParticleEngine( );
	~ParticleEngine( );

	void Update( );
	void LoadAllParticleSystemResources( );
	ParticleSystem * PlayOnce( std::string const & particleSystemID, int layerID, Vector2f const & position = Vector2f::ZERO );
	ParticleSystem * Create( std::string const & particleSystemID, int layerID, Vector2f const & position = Vector2f::ZERO );
};