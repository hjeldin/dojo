/*
 *  Particle.h
 *  NinjaTraining
 *
 *  Created by Tommaso Checchi on 5/5/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "dojo_common_header.h"

#include "AnimatedQuad.h"

namespace Dojo 
{
	class ParticlePool;
	class GameState;


	///class tightly coupled to ParticlePool needed to create fast appearing/disappearing effects
	class Particle : public AnimatedQuad 
	{
	public:	

		friend class ParticlePool;

		class EventListener
		{
		public:
			
			virtual void onTimedEvent( Particle* p )=0;
		};
				
		float lifeTime;		
		float spriteSizeScaleSpeed;

		Vector acceleration;
		
		Particle( ParticlePool* p, Object* level, int i ) :
		AnimatedQuad( level, Vector::ZERO ),
		pool( p ),
		index( i ),
		lifeTime( 1 )
		{
			onReset();
			
			setVisible( false );
		}		
		
		virtual void onReset()
		{
			AnimatedQuad::reset();

			acceleration.x = 0;
			acceleration.y = 0;
			
			spriteSizeScaleSpeed = 0;
			listener = NULL;
		}
		
		void setTimedEvent( EventListener* l, float lifeTime )
		{			
			eventTime = lifeTime;
			listener = l;
		}
		
		void removeTimedEvent()
		{
			listener = NULL;
			eventTime = 0;
		}
		
		EventListener* getListener()		{	return listener;	}
		
		bool launchTimedEvent()		
		{
			return listener && lifeTime < eventTime;
		}
		
		void move( float dt )
		{
            DEBUG_TODO; //particles need to be updated to ogl2.0
            
			/*advanceAnim( dt );
			advanceFade( dt );

			worldPosition.x += speed.x * dt;
			worldPosition.y += speed.y * dt;	

			speed.x += acceleration.x * dt;
			speed.y += acceleration.y * dt; 

			//worldRotation += rotationSpeed * dt;

			pixelScale.x += spriteSizeScaleSpeed * dt;
			pixelScale.y += spriteSizeScaleSpeed * dt;

			if( launchTimedEvent() )
			{
				getListener()->onTimedEvent( this );
				removeTimedEvent();
			}*/
		}
		
		void _setPoolIdx( int i )	{	index = i;			}
		int _getPoolIdx()			{	return index;		}
		
	protected:

		int index;
		
		ParticlePool* pool;
		
		EventListener* listener;
		
		float eventTime;
		
	};
}

