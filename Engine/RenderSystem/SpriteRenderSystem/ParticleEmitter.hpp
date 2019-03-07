#pragma once

#include <string>
#include "Engine/Math/Range.hpp"
#include "Engine/Math/Vector2f.hpp"
#include "Engine/RenderSystem/Color.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/SpriteResource.hpp"


//-------------------------------------------------------------------------------------------------
class Particle;


//-------------------------------------------------------------------------------------------------
class ParticleEmitterResource
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	std::string spriteID;
	int initialSpawn;
	float spawnRate;
	Range<Vector2f> offsetPosition;
	Range<float> totalLifetimeSeconds;
	Range<float> startScale;
	Range<float> endScale;
	bool constantScale; // true - ignores end scale
	//Range<float> rate;
	Range<float> startSpeed;
	Range<float> startDirectionDegrees;
	Color startColor;
	Color endColor;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	ParticleEmitterResource( std::string const & spriteName );
	~ParticleEmitterResource( );
};


//-------------------------------------------------------------------------------------------------
class ParticleEmitter
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	ParticleEmitterResource const * m_particleEmitterResource;
	float m_emitTimer;
	int layerID;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	ParticleEmitter( ParticleEmitterResource const * resource );
	~ParticleEmitter( );

	Particle * SpawnParticle( Vector2f const & position ) const;
};