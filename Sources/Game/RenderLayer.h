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
#include "MessageOverlay.h"


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
	MessageOverlay *MessageOutput;
	TextBox *MessageInput;
	Pos3D Cam;
	Clock PlayTime;
	
	RenderLayer( void );
	virtual ~RenderLayer();
	
	void SetBackground( void );
	void SetWorldLights( bool deathstar, float ambient_scale = 1.f, const std::vector<Vec3D> *obstructions = NULL );
	void SetDynamicLights( Pos3D *pos, Pos3D *offset, int dynamic_lights, std::list<Shot*> *shots, std::list<Effect*> *effects );
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
	
	bool HandleEvent( SDL_Event *event );
	bool KeyDown( SDLKey key );
};

class Renderable
{
public:
	Ship *ShipPtr;
	Shot *ShotPtr;
	Effect *EffectPtr;
	ShipEngine *EnginePtr;
	Pos3D EnginePos; // FIXME: This adds a bit of unnecessary overhead to other renderables.
	float EngineAlpha;
	double EngineScale;
	
	Renderable( Ship *ship );
	Renderable( Shot *shot );
	Renderable( Effect *effect );
	Renderable( ShipEngine *engine, const Pos3D *pos, float alpha, double scale = 1. );
	~Renderable();
};
