#pragma once

#include "Game/GameObjects/GameObject.hpp"


//-------------------------------------------------------------------------------------------------
class Bullet;
class ParticleSystem;
class Sprite;
class TextRenderer;
enum ePlayerClass : byte_t;
typedef unsigned int SoundID;


//-------------------------------------------------------------------------------------------------
class Ship : public GameObject
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static float const IMMUNE_TIME;
	static float const BULLET_START_OFFSET;
	static float const SLOW_DURATION;

	//These aren't const because I cant make them static const unless that have a static initializer
	//And g_AudioSystem doesn't exist at that point
	//#TODO: I may change or fix this later
	static SoundID * SOUND_FIRE;
	static SoundID * SOUND_EXPLODE;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static void Initialize( );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
//Synced Members
public:
	uint16_t m_health;
	uint16_t m_netHealth;
	uint16_t m_shield;
	uint16_t m_netShield;
	uint16_t m_energy;
	uint16_t m_netEnergy;

//Non-Synced Members
protected:
	float m_fireCooldownTimer;
	float m_immuneTimer;
	ParticleSystem * m_engineEffect[2];

public:
	uint8_t m_killedByPlayerIndex;

protected:
	float m_slowAmount;
	float m_slowEffectTimer;
	bool m_netIsSlow;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	virtual void WriteToMessage( NetMessage * out_message ) override;
	virtual void UpdateFromMessage( NetSender const & sender, NetMessage const & message ) override;

	virtual void WriteToXMLNode( XMLNode * out_xmlNode ) override;
	virtual void UpdateFromXMLNode( XMLNode const & node ) override;

	Ship( byte_t playerIndex, eNetGameObjectType const & gameObjectType, bool hostObject );
	virtual ~Ship( ) override;
	virtual void Update( ) override;
	void UpdateFreezeSpriteEffect( );
	virtual void HostHandleCollision( GameObject * gameObject ) override;
	virtual bool HostIsAlive( ) const override;
	virtual void HostFireWeapon( ) const = 0;
	virtual char const * GetEntityName( ) const override = 0;
	virtual std::string GetSpriteID( ) const override = 0;
	virtual float GetNetSyncPercent( ) const override = 0;
	virtual float GetPhysicsRadius( ) const override = 0;
	virtual float GetSpriteScale( ) const override = 0;
	virtual std::string GetEngineID( ) const;
	virtual void HostOnDestroy( ) override;
	virtual void ClientOnDestroy( ) override;

	virtual void ApplyControlInput( ) = 0;
	virtual int GetMaxHealth( ) const = 0;
	virtual int GetMaxShield( ) const = 0;
	virtual int GetMaxEnergy( ) const = 0;
	virtual float GetMaxSpeed( ) const = 0;
	virtual float GetFireRate( ) const = 0;
	virtual uint8_t GetSummoningBarValue( ) const = 0;

	void UpdateTimers( );
	bool CanFire( ) const;
	void ResetFireCooldown( );
	SoundID GetRandomFireSound( ) const;
	SoundID GetRandomExplodeSound( ) const;
	bool HasImmunity( ) const;
	void HostTakeDamage( int incomingDamage, uint8_t sourcePlayerIndex, uint16_t weaponCode );
	void HostTakeDamage( int incomingDamage, uint8_t sourcePlayerIndex );
	Vector2f GetPositionInfront( float unitsForward, float degreesOffset = 0.f ) const;
	void HostApplySlow( float amount );
	bool HostIsSlowed( ) const;
};