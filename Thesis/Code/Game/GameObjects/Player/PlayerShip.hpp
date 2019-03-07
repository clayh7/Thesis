#pragma once

#include "Game/GameObjects/Player/Ship.hpp"


//-------------------------------------------------------------------------------------------------
class Bullet;
class ParticleSystem;
class Sprite;
class TextRenderer;
enum ePlayerClass : byte_t;


//-------------------------------------------------------------------------------------------------
class PlayerShip : public Ship
{
//-------------------------------------------------------------------------------------------------
// Static Members
//-------------------------------------------------------------------------------------------------
public:
	static char const * NAME;
	static float const NET_SYNC_PERCENT;
	static float const SPEED;
	static float const SPEED_DECAY_RATE;
	static float const TURN_SPEED;
	static float const RADIUS;
	static float const PLAYERSHIP_RADIUS_TO_SCALE;
	static float const FIRE_RATE;
	static float const WARP_TIME_BASE;
	static float const WARP_TIME_PER_ITEM;
	static float const TARGET_RADIUS_DEGREES;
	static float const TARGET_MAX_RANGE;
	static byte_t const TARGET_KEY;
	static char const * ID_LOCKED_ON;
	static char const * ID_TARGETING;
	static float const SHIELD_REGEN_COOLDOWN;
	static float const ENERGY_REGEN_COOLDOWN;
	static float const ROGUE_SPEED_MULTIPLIER;
	
	static int const CTHULHU_HEALTH;
	static int const CTHULHU_SHIELD;
	static int const CTHULHU_ENERGY;
	static float const CTHULHU_RADIUS;
	static float const CTHULHU_SPEED;
	static float const CTHULHU_FIRE_RATE;

	static float const CRYSTAL_FEED_RATE;
	static float const CRYSTAL_FEED_RANGE;
	static uint16_t const CRYSTAL_FEED_AMOUNT;

private:
	static GameObjectRegistration s_PlayerShipMessageRegistration;

//-------------------------------------------------------------------------------------------------
// Static Functions
//-------------------------------------------------------------------------------------------------
public:
	static GameObject * MessageCreation( NetSender const &, NetMessage const & );

//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
//Synced Members
public:
	uint16_t m_inputBitfieldState;
	uint16_t m_netInputBitfieldState;
	bool m_isWarping;
	bool m_netIsWarping;

//Non-Synced Members
protected:
	Sprite * m_weaponSprite;
	float m_warpTimer;
	float m_shieldRegenTimer;
	float m_energyRegenTimer;

public:
	bool m_lockedOn;
	GameObject * m_targetGameObject;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	PlayerShip( NetSender const & sender, NetMessage const & message );
	virtual void WriteToMessage( NetMessage * out_message ) override;
	virtual void UpdateFromMessage( NetSender const & sender, NetMessage const & message ) override;

	//PlayerShip( XMLNode const & node );
	//virtual void WriteToXMLNode( XMLNode * out_xmlNode ) override;
	//virtual void UpdateFromXMLNode( XMLNode const & node ) override;

	PlayerShip( byte_t playerIndex, bool hostObject );
	virtual ~PlayerShip( ) override;
	virtual void Update( ) override;
	virtual void HostHandleCollision( GameObject * gameObject ) override;
	virtual void HostFireWeapon( ) const override;
	virtual char const * GetEntityName( ) const override;
	virtual std::string GetSpriteID( ) const override;
	virtual float GetNetSyncPercent( ) const override;
	virtual float GetPhysicsRadius( ) const override;
	virtual float GetSpriteScale( ) const override;
	virtual void HostOnDestroy( ) override;
	virtual void ClientOnDestroy( ) override;

	virtual void ApplyControlInput( ) override;
	virtual int GetMaxHealth( ) const override;
	virtual int GetMaxShield( ) const override;
	virtual int GetMaxEnergy( ) const override;
	virtual float GetMaxSpeed( ) const override;
	virtual float GetFireRate( ) const override;
	virtual uint8_t GetSummoningBarValue( ) const override;

	void UpdateTimers( );
	void UpdateWeaponSprite( );
	void ApplyInput( uint16_t inputBitfield );
	void HostToggleWarping( );
	bool IsWarping( ) const;
	bool CanFeedCrystal( ) const;
	float GetCurrentWarpTime( ) const;
	float GetMaxWarpTime( ) const;
	ePlayerClass GetPlayerClass( ) const;
	bool HasTarget( ) const;
	GameObject * GetTarget( ) const;
	std::string GetTargetBracketSpriteID( ) const;
	bool HostIsEvil( ) const;
};