#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEngine.hpp"

#include "Engine/RenderSystem/SpriteRenderSystem/ParticleSystem.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/ParticleEmitter.hpp"
#include "Engine/Math/Vector2f.hpp"


//-------------------------------------------------------------------------------------------------
ParticleEngine * g_ParticleEngine = nullptr;


//-------------------------------------------------------------------------------------------------
ParticleEngine::ParticleEngine( )
{
	LoadAllParticleSystemResources( );
}


//-------------------------------------------------------------------------------------------------
ParticleEngine::~ParticleEngine( )
{
	for( auto deleteResource : m_particleSystemResourceDatabase )
	{
		delete deleteResource.second;
		deleteResource.second = nullptr;
	}
	m_particleSystemResourceDatabase.clear( );

	for( ParticleSystem * deleteParticleSystem : m_activeParticleSystems )
	{
		delete deleteParticleSystem;
		deleteParticleSystem = nullptr;
	}
	m_activeParticleSystems.clear( );
}


//-------------------------------------------------------------------------------------------------
void ParticleEngine::Update( )
{
	for( size_t particleSystemIndex = 0; particleSystemIndex < m_activeParticleSystems.size(); ++particleSystemIndex )
	{
		m_activeParticleSystems[particleSystemIndex]->Update( );

		//Clean up complete particle systems
		if( m_activeParticleSystems[particleSystemIndex]->IsFinished() )
		{
			delete m_activeParticleSystems[particleSystemIndex];
			m_activeParticleSystems[particleSystemIndex] = m_activeParticleSystems[m_activeParticleSystems.size( ) - 1];
			m_activeParticleSystems.pop_back( );
		}
	}
}


//-------------------------------------------------------------------------------------------------
void ParticleEngine::LoadAllParticleSystemResources( )
{
	//#TODO: Make this XML
	//---------------------------------------------------------------------------------------------
	//Burst Particle System
	ParticleSystemResource * burst = new ParticleSystemResource( "burst" );

	//Adding an emitter
	ParticleEmitterResource * burstEmitter = new ParticleEmitterResource( "circle" );
	burstEmitter->initialSpawn = 30;
	burstEmitter->spawnRate = 0.0f;
	burstEmitter->totalLifetimeSeconds = Range<float>( 1.5f, 2.5f );
	burstEmitter->startScale = Range<float>( 0.0035f, 0.005f );
	burstEmitter->startSpeed = Range<float>( 0.0f, 0.5f );
	burstEmitter->startColor = Color( 180, 100, 100, 255 );
	burstEmitter->endColor = Color( 180, 100, 100, 0 );
	burst->AddEmitter( burstEmitter );

	//Add new Particle System to Database
	size_t particleSystemIDHash = std::hash<std::string>{ }( "burst" );
	m_particleSystemResourceDatabase.insert( std::pair<size_t, ParticleSystemResource*>( particleSystemIDHash, burst ) );


	//---------------------------------------------------------------------------------------------
	//Burst Particle System
	ParticleSystemResource * rockBurst = new ParticleSystemResource( "rockBurst" );

	//Adding an emitter
	ParticleEmitterResource * rockBurstEmitter = new ParticleEmitterResource( "asteroid1" );
	rockBurstEmitter->initialSpawn = 20;
	rockBurstEmitter->spawnRate = 0.0f;
	rockBurstEmitter->totalLifetimeSeconds = Range<float>( 0.5f, 1.5f );
	rockBurstEmitter->startScale = Range<float>( 0.035f , 0.05f );
	//rockBurstEmitter->rate = Range<float>( 0.0f, 0.5f );
	rockBurstEmitter->startSpeed = Range<float>( 0.25f, 0.75f );
	rockBurstEmitter->startColor = Color( 193, 154, 107, 255 );
	rockBurstEmitter->endColor = Color( 193, 154, 107, 127 );
	rockBurst->AddEmitter( rockBurstEmitter );
	
	//Add new Particle System to Database
	particleSystemIDHash = std::hash<std::string>{ }( "rockBurst" );
	m_particleSystemResourceDatabase.insert( std::pair<size_t, ParticleSystemResource*>( particleSystemIDHash, rockBurst ) );

	//---------------------------------------------------------------------------------------------
	//Explosion Particle System
	ParticleSystemResource * explosion = new ParticleSystemResource( "explosion" );

	//Adding an emitter
	ParticleEmitterResource * explosionEmitter = new ParticleEmitterResource( "circle" );
	explosionEmitter->initialSpawn = 3;
	explosionEmitter->spawnRate = 0.0f;
	explosionEmitter->totalLifetimeSeconds = Range<float>( 1.0f, 2.0f );
	explosionEmitter->startScale = Range<float>( 0.15f, 0.2f );
	//rockBurstEmitter->rate = Range<float>( 0.0f, 0.5f );
	explosionEmitter->startSpeed = Range<float>( 0.25f );
	explosionEmitter->startColor = Color( 255, 255, 255, 200 );
	explosionEmitter->endColor = Color( 0, 0, 0, 0 );
	explosion->AddEmitter( explosionEmitter );

	//Add new Particle System to Database
	particleSystemIDHash = std::hash<std::string>{ }( "explosion" );
	m_particleSystemResourceDatabase.insert( std::pair<size_t, ParticleSystemResource*>( particleSystemIDHash, explosion ) );

	//---------------------------------------------------------------------------------------------
	//bullet pop Particle System
	ParticleSystemResource * bulletPop = new ParticleSystemResource( "bulletPop" );

	//Adding an emitter
	ParticleEmitterResource * bulletPopEmitter = new ParticleEmitterResource( "square" );
	bulletPopEmitter->initialSpawn = 40;
	bulletPopEmitter->spawnRate = 0.0f;
	bulletPopEmitter->totalLifetimeSeconds = Range<float>( 0.f, 0.8f );
	bulletPopEmitter->startScale = Range<float>( 0.02f, 0.02f );
	//bulletPopEmitter->rate = Range<float>( 0.0f, 0.5f );
	bulletPopEmitter->startSpeed = Range<float>( 1.75f, 2.25f );
	bulletPopEmitter->startColor = Color( 0, 0, 0, 255 );
	bulletPopEmitter->endColor = Color( 0, 0, 0, 0 );
	bulletPop->AddEmitter( bulletPopEmitter );

	//Add new Particle System to Database
	particleSystemIDHash = std::hash<std::string>{ }( "bulletPop" );
	m_particleSystemResourceDatabase.insert( std::pair<size_t, ParticleSystemResource*>( particleSystemIDHash, bulletPop ) );

	//---------------------------------------------------------------------------------------------
	//Explode Particle System
	ParticleSystemResource * explode = new ParticleSystemResource( "explode" );

	//Adding an emitter
	ParticleEmitterResource * exlpodeEmitter = new ParticleEmitterResource( "square" );
	exlpodeEmitter->initialSpawn = 50;
	exlpodeEmitter->spawnRate = 0.0f;
	exlpodeEmitter->totalLifetimeSeconds = Range<float>( 2.f, 2.f );
	exlpodeEmitter->startScale = Range<float>( 0.03f, 0.03f );
	//exlpodeEmitter->rate = Range<float>( 0.0f, 0.5f );
	exlpodeEmitter->startSpeed = Range<float>( 3.f, 3.f );
	exlpodeEmitter->startColor = Color( 255, 255, 255, 255 );
	exlpodeEmitter->endColor = Color( 255, 255, 255, 0 );
	explode->AddEmitter( exlpodeEmitter );

	//Add new Particle System to Database
	particleSystemIDHash = std::hash<std::string>{ }( "explode" );
	m_particleSystemResourceDatabase.insert( std::pair<size_t, ParticleSystemResource*>( particleSystemIDHash, explode ) );

	//---------------------------------------------------------------------------------------------
	//Level up Particle System
	ParticleSystemResource * levelup = new ParticleSystemResource( "levelup" );

	//Adding an emitter
	ParticleEmitterResource * levelupEmitter = new ParticleEmitterResource( "triangle" );
	levelupEmitter->initialSpawn = 50;
	levelupEmitter->spawnRate = 0.0f;
	levelupEmitter->totalLifetimeSeconds = Range<float>( 2.f, 2.f );
	levelupEmitter->startScale = Range<float>( 0.03f, 0.03f );
	//levelupEmitter->rate = Range<float>( 0.0f, 0.5f );
	levelupEmitter->startSpeed = Range<float>( 1.f, 6.f );
	levelupEmitter->startColor = Color::YELLOW;
	levelupEmitter->endColor = Color( 255, 255, 0, 0 );
	levelup->AddEmitter( levelupEmitter );

	//Add new Particle System to Database
	particleSystemIDHash = std::hash<std::string>{ }( "levelup" );
	m_particleSystemResourceDatabase.insert( std::pair<size_t, ParticleSystemResource*>( particleSystemIDHash, levelup ) );

	//---------------------------------------------------------------------------------------------
	//Trail Particle System
	ParticleSystemResource * trail = new ParticleSystemResource( "trail" );

	//Adding an emitter
	ParticleEmitterResource * trailEmitter = new ParticleEmitterResource( "square" );
	trailEmitter->initialSpawn = 0;
	trailEmitter->spawnRate = 0.005f;
	trailEmitter->totalLifetimeSeconds = Range<float>( 0.25f, 0.75f );
	trailEmitter->startScale = Range<float>( 0.005f, 0.005f );
	//trailEmitter->rate = Range<float>( 0.05f, 0.1f );
	trailEmitter->startSpeed = Range<float>( 0.25f );
	trailEmitter->startColor = Color( 100, 100, 100, 255 );
	trailEmitter->endColor = Color( 100, 100, 100, 0 );
	trail->AddEmitter( trailEmitter );

	//Add new Particle System to Database
	particleSystemIDHash = std::hash<std::string>{ }( "trail" );
	m_particleSystemResourceDatabase.insert( std::pair<size_t, ParticleSystemResource*>( particleSystemIDHash, trail ) );

	//---------------------------------------------------------------------------------------------
	//Engine Particle System
	ParticleSystemResource * engine = new ParticleSystemResource( "engine" );

	//Adding an emitter
	ParticleEmitterResource * engineEmitter = new ParticleEmitterResource( "square" );
	engineEmitter->initialSpawn = 0;
	engineEmitter->spawnRate = 0.05f;
	engineEmitter->totalLifetimeSeconds = Range<float>( 0.5f, 1.5f );
	engineEmitter->startScale = Range<float>( 0.01f, 0.02f );
	engineEmitter->endScale = Range<float>( 0.02f, 0.04f );
	//engineEmitter->rate = Range<float>( 0.0f, 0.0f );
	engineEmitter->startSpeed = Range<float>( 0.15f );
	engineEmitter->startColor = Color( 255, 153, 51, 255 );
	engineEmitter->endColor = Color( 0, 0, 0, 0 );
	engine->AddEmitter( engineEmitter );

	//Add new Particle System to Database
	particleSystemIDHash = std::hash<std::string>{ }( "engine" );
	m_particleSystemResourceDatabase.insert( std::pair<size_t, ParticleSystemResource*>( particleSystemIDHash, engine ) );

	//---------------------------------------------------------------------------------------------
	//Trail Particle System
	ParticleSystemResource * background = new ParticleSystemResource( "background" );

	//Adding an emitter
	ParticleEmitterResource * backgroundEmitter = new ParticleEmitterResource( "square" );
	backgroundEmitter->initialSpawn = 0;
	backgroundEmitter->spawnRate = 0.03f;
	backgroundEmitter->offsetPosition = Range<Vector2f>( Vector2f( -10.f, 0.f ), Vector2f( 10.f, 0.f ) );
	backgroundEmitter->totalLifetimeSeconds = Range<float>( 7.5f, 12.5f );
	backgroundEmitter->startScale = Range<float>( 0.02f, 0.03f );
	backgroundEmitter->endScale = Range<float>( 0.0f, 0.0f );
	backgroundEmitter->constantScale = false;
	//backgroundEmitter->rate = Range<float>( 0.05f, 0.1f );
	backgroundEmitter->startSpeed = Range<float>( 0.5f,1.f );
	backgroundEmitter->startDirectionDegrees = Range<float>( 90.f );
	backgroundEmitter->startColor = Color( 57, 124, 88, 255 );
	backgroundEmitter->endColor = Color( 57, 124, 88, 0 );
	background->AddEmitter( backgroundEmitter );

	//Add new Particle System to Database
	particleSystemIDHash = std::hash<std::string>{ }( "background" );
	m_particleSystemResourceDatabase.insert( std::pair<size_t, ParticleSystemResource*>( particleSystemIDHash, background ) );
}


//-------------------------------------------------------------------------------------------------
ParticleSystemResource const * ParticleEngine::GetParticleSystemResource( std::string const & resourceName )
{
	size_t particleSystemIDHash = std::hash<std::string>{ }( resourceName );
	auto particleSystemResourceFound = m_particleSystemResourceDatabase.find( particleSystemIDHash );
	if( particleSystemResourceFound == m_particleSystemResourceDatabase.end( ) )
	{
		ERROR_AND_DIE( "Particle System resource not found." );
	}
	return particleSystemResourceFound->second;
}


//-------------------------------------------------------------------------------------------------
ParticleSystem * ParticleEngine::PlayOnce( std::string const & particleSystemID, int layerID, Vector2f const & position /*= Vector2f::ZERO*/ )
{
	ParticleSystemResource const * particleSystemResource = GetParticleSystemResource( particleSystemID );
	ParticleSystem * spawnedSystem = new ParticleSystem( particleSystemResource, layerID, position );
	spawnedSystem->Initialize( );
	spawnedSystem->Destroy( );
	m_activeParticleSystems.push_back( spawnedSystem );
	return spawnedSystem;
}


//-------------------------------------------------------------------------------------------------
ParticleSystem * ParticleEngine::Create( std::string const & particleSystemID, int layerID, Vector2f const & position /*= Vector2f::ZERO */ )
{
	ParticleSystemResource const * particleSystemResource = GetParticleSystemResource( particleSystemID );
	ParticleSystem * spawnedSystem = new ParticleSystem( particleSystemResource, layerID, position );
	spawnedSystem->Initialize( );
	m_activeParticleSystems.push_back( spawnedSystem );
	return spawnedSystem;
}