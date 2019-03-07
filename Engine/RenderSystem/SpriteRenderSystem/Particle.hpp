#pragma once

#include "Engine/RenderSystem/SpriteRenderSystem/Sprite.hpp"


//-------------------------------------------------------------------------------------------------
class Particle
{
//-------------------------------------------------------------------------------------------------
// Members
//-------------------------------------------------------------------------------------------------
public:
	Sprite * sprite;
	Vector2f velocity;
	Vector2f acceleration;
	float age;
	float maxAge;
	float startScale;
	float endScale;
	Color startColor;
	Color endColor;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
public:
	Particle( );
	~Particle( );

	void SetPosition( Vector2f const & position );
};