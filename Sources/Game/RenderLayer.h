/*
 *  RenderLayer.h
 */

#pragma once
class RenderLayer;
class Renderable;

#include "PlatformSpecific.h"

#include "Layer.h"
#include <stdint.h>
#include "RaptorGL.h"
#include "Animation.h"
#include "Pos.h"
#include "Vec.h"
#include "Font.h"
#include "TextBox.h"
#include "Ship.h"
#include "Shot.h"
#include "Effect.h"


#define STAR_COUNT 2000
#define DEBRIS_COUNT 1000
#define DEBRIS_DIST 120.


class RenderLayer : public Layer
{
public:
	Animation Background;
	std::string BackgroundName;
	GLdouble Stars[ STAR_COUNT * 3 ];
	Pos3D Debris[ DEBRIS_COUNT ];
	Font *BigFont, *SmallFont, *ScreenFont, *RadarDirectionFont;
	TextBox *MessageInput;
	
	RenderLayer( void );
	virtual ~RenderLayer();
	
	void SetBackground( void );
	void SetWorldLights( float ambient_scale = 1.f, const std::vector<const Vec3D*> *obstructions = NULL );
	void ClearWorldLights( void );
	void ClearDynamicLights( void );
	void ClearMaterial( void );
	
	void Draw( void );
	void DrawBackground( void );
	void DrawStars( void );
	void DrawDebris( void );
	void DrawScores( void );
	
	#ifdef WIN32
	void UpdateSaitek( const Ship *player_ship, bool is_player, int view );
	#endif
	
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
