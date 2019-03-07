#include "Engine/RenderSystem/SpriteRenderSystem/SpriteLayer.hpp"
#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"


//-------------------------------------------------------------------------------------------------
SpriteLayer::SpriteLayer( int id )
	: m_enabled( true )
	, m_layerID( id )
	, m_listStart(nullptr )
{

}


//-------------------------------------------------------------------------------------------------
SpriteLayer::~SpriteLayer( )
{

}


//-------------------------------------------------------------------------------------------------
void SpriteLayer::AddSprite( Sprite * newSprite )
{
	newSprite->m_nextSprite = m_listStart;
	if( m_listStart != nullptr )
	{
		m_listStart->m_prevSprite = newSprite;
	}
	m_listStart = newSprite;
}


//-------------------------------------------------------------------------------------------------
void SpriteLayer::RemoveSprite( Sprite * sprite )
{
	//Move the head of the list
	if( m_listStart == sprite )
	{
		m_listStart = sprite->m_nextSprite;
	}
	//Pull out from middle of the list
	else
	{
		Sprite * prev = sprite->m_prevSprite;
		Sprite * next = sprite->m_nextSprite;
		if( prev )
		{
			prev->m_nextSprite = next;
		}
		if( next )
		{
			next->m_prevSprite = prev;
		}
	}
}


//-------------------------------------------------------------------------------------------------
void SpriteLayer::AddEffect( eMaterialEffect const & effect )
{
	m_effects.push_back( effect );
}


//-------------------------------------------------------------------------------------------------
void SpriteLayer::RemoveAllEffects( )
{
	m_effects.clear( );
}


//-------------------------------------------------------------------------------------------------
bool SpriteLayer::IsEnabled( ) const 
{
	return m_enabled;
}


//-------------------------------------------------------------------------------------------------
void SpriteLayer::SetEnabled( bool isEnabled )
{
	m_enabled = isEnabled;
}