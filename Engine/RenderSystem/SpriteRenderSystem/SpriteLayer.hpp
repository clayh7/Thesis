#pragma once
#include <vector>
#include "SpriteGameRenderer.hpp"


//-------------------------------------------------------------------------------------------------
class Sprite;


//-------------------------------------------------------------------------------------------------
class SpriteLayer
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	bool m_enabled;

public:
	int m_layerID;
	Sprite * m_listStart;
	std::vector<eMaterialEffect> m_effects;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	SpriteLayer( int id );
	~SpriteLayer( );

	void AddSprite( Sprite * newSprite );
	void RemoveSprite( Sprite * sprite );
	void AddEffect( eMaterialEffect const & effect );
	void RemoveAllEffects( );

	bool IsEnabled( ) const;

	void SetEnabled( bool isEnabled );
};