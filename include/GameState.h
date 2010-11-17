/*
 *  GameState.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 5/16/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef GameState_h__
#define GameState_h__

#include "dojo_config.h"

#include "Array.h"

#include "ResourceGroup.h"
#include "TouchSource.h"
#include "StateInterface.h"

namespace Dojo {
	
	class Viewport;
	class SoundManager;
	class Renderable;
	class Object;
	class Game;
	
	class GameState : public ResourceGroup, public TouchSource::Listener, public StateInterface
	{
	public:
		
		typedef Array<Object*> ObjectList;
		typedef Array<Renderable*> RenderableList;
		
		GameState( Game* parentGame );
		
		virtual ~GameState();
		
		///initialise from cleared state
		virtual void initialise()=0;
		
		///clear and prepare for a new initialise
		virtual void clear();
		
		inline Game* getGame()				{	return game;			}
				
		inline float getCurrentTime()		{	return timeElapsed;		}		
		inline Viewport* getViewport()		{	return camera;			}		
		SoundManager* getSoundManager();
						
		inline void addClickable( Renderable* s )
		{
			clickables.addElement( s );
		}
		
		inline void addObject( Object* o )
		{
			objects.addElement( o );
		}
				
		void addObject( Renderable* s, int layer, bool clickable = false );
		
		inline void removeObject( Object* o )
		{
			objects.removeElement( o );
		}
		
		inline void removeClickableSprite( Renderable* s );
		void removeSprite( Renderable* s );
		
		void removeAll();
		
		Renderable* getClickableAtPoint( const Vector& point );
		
		void updateObjects( float dt );
		
		virtual void onButtonClicked( Renderable* button )=0;		
		virtual void onButtonReleased( Renderable* button )=0;
		
		virtual void update( float dt )=0;
		
	protected:
		
		Game* game;
		
		Viewport* camera;
		
		float timeElapsed;		
		
		ObjectList objects;
		RenderableList clickables;
	};
}

#endif