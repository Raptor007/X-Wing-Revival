/*
 *  RenderLayer.h
 */

#pragma once
class RenderLayer;
class Renderable;

#include "platforms.h"

#include "Layer.h"
#include <stdint.h>
#include "Animation.h"
#include "Pos.h"
#include "Vec.h"
#include "Font.h"
#include "TextBox.h"
#include "Shot.h"
#include "Effect.h"


#define STAR_COUNT 2000
#define DEBRIS_COUNT 1000
#define DEBRIS_DIST 120.


class RenderLayer : public Layer
{
public:
	Animation Background;
	Vec3D Stars[ STAR_COUNT ];
	Pos3D Debris[ DEBRIS_COUNT ];
	Font *BigFont, *SmallFont, *ScreenFont, *RadarDirectionFont;
	TextBox *MessageInput;
	
	RenderLayer( void );
	virtual ~RenderLayer();
	
	void Draw( void );
	void DrawBackground( void );
	void DrawStars( void );
	void DrawDebris( void );
	void DrawScores( void );
	
	bool KeyDown( SDLKey key );
};

class Renderable
{
public:
	Shot *ShotPtr;
	Effect *EffectPtr;
	
	Renderable( Shot *shot );
	Renderable( Effect *effect );
	~Renderable();
};
