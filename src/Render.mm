//
//  Render.m
//  NinjaTraining
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#include "Render.h"

#include "Renderable.h"
#include "TextArea.h"

#include "Viewport.h"

#include "Mesh.h"
#include "Model.h"

using namespace Dojo;

Render::Render() :
frameStarted( false ),
viewport( NULL ),
valid( true ),
cullingEnabled( true ),
interfaceOrientation( IO_LANDSCAPE_LEFT ),
interfaceRotation( 90 )
{		
	context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
	
    if (!context || ![EAGLContext setCurrentContext:context])
	{
		valid = false;
		return;
    }
	
	// Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
	glGenFramebuffersOES(1, &defaultFramebuffer);
	glGenRenderbuffersOES(1, &colorRenderbuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);
	
	//get default screen size	
	//HACK width and height are inverted for horizontal screens!
	devicePixelScale = [UIScreen mainScreen].scale;
	width = [UIScreen mainScreen].bounds.size.height * devicePixelScale;
	height = [UIScreen mainScreen].bounds.size.width * devicePixelScale;	
	
	//gles settings
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );	
	
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		
	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST );
		
	//projection is always the same
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();		
					
	//setup object data	
	glEnableClientState(GL_VERTEX_ARRAY);			
	//glEnableClientState(GL_COLOR_ARRAY);			
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);			
	//glEnableClientState(GL_NORMAL_ARRAY);
		
	currentRenderState = firstRenderState = new RenderState();
}

Render::~Render()
{
	delete firstRenderState;
	
	// Tear down GL
	if (defaultFramebuffer)
	{
		glDeleteFramebuffersOES(1, &defaultFramebuffer);
		defaultFramebuffer = 0;
	}
	
	if (colorRenderbuffer)
	{
		glDeleteRenderbuffersOES(1, &colorRenderbuffer);
		colorRenderbuffer = 0;
	}
		
	// Tear down context
	if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
		
	[context release];	
}


bool Render::resizeFromLayer(CAEAGLLayer * layer)
{
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	
	[context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:layer];
	
	int w, h;
	
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &w);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &h);
	
	width = h;
	height = w;
	
	if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
		return false;
	}
	
	return true;
}

Render::RenderableList* Render::getLayer( int layerID )
{	
	LayerList* layerList = &positiveLayers;
	if( layerID < 0 )
	{
		layerID = -layerID - 1; //flip and shift by 1
		layerList = &negativeLayers;
	}
	
	//allocate the needed layers if layerID > layer size
	while( layerList->size() <= layerID )
		layerList->addElement( new RenderableList() );	
	
	//get the needed layer	
	return layerList->at( layerID );
}

void Render::addRenderable( Renderable* s, int layerID )
{				
	//get the needed layer	
	RenderableList* layer = getLayer( layerID );

	//insert this object in the place where the distances from its neighbours are a minimum.	
	uint bestIndex = 0;
	uint bestDistanceSum = 0xffffffff;
	
	uint distance;
	uint lastDistance = firstRenderState->getDistance( s );
	for( uint i = 0; i < layer->size(); ++i )
	{
		distance = layer->at(i)->getDistance( s );
		if( distance + lastDistance < bestDistanceSum )
		{
			bestDistanceSum = distance + lastDistance;
			bestIndex = i;
		}
		
		lastDistance = distance;
	}
		
	s->_notifyRenderInfo( this, layerID, bestIndex );
		
	layer->addElement( s, bestIndex );
}

void Render::removeRenderable( Renderable* s )
{	
	getLayer( s->getLayer() )->removeElement( s );
	
	s->_notifyRenderInfo( NULL, 0, 0 );
}

void Render::startFrame()
{	
	if( frameStarted || !viewport )
		return;
			
	[EAGLContext setCurrentContext:context];
				
	// This application only creates a single default framebuffer which is already bound at this point.
	// This call is redundant, but needed if dealing with multiple framebuffers.
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
	
	glViewport( 0, 0, height, width );
	
	//clear the viewport
	glClearColor( 
				 viewport->getClearColor().r, 
				 viewport->getClearColor().g, 
				 viewport->getClearColor().b, 
				 viewport->getClearColor().a );
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	//load view matrix on top
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();					
	
	//rotate to balance interface orientation
	glRotatef( interfaceRotation, 0, 0, -1 );
	
	//scale with area and window ratio
	glScalef( 
			 2.f / viewport->getSize().x, 
			 2.f / viewport->getSize().y, 
			 1.f);
		
	//translate
	glTranslatef( 
				 -viewport->position.x,
				 -viewport->position.y, 
				 0.f );		
	
	if( interfaceOrientation == IO_LANDSCAPE_LEFT || interfaceOrientation == IO_LANDSCAPE_RIGHT )
	{
		viewportPixelRatio.x = viewport->getSize().x / width;
		viewportPixelRatio.y = viewport->getSize().y / height;
	}
	else 
	{
		//swap
		viewportPixelRatio.x = viewport->getSize().y / width;
		viewportPixelRatio.y = viewport->getSize().x / height;
	}
		
	frameStarted = true;
}

void Render::renderElement( Renderable* s )
{
	if( !frameStarted || !s )
		return;
	
	s->prepare( viewportPixelRatio );
	
	//change the renderstate
	s->commitChanges( currentRenderState );
	currentRenderState = s;
	
	//clone the view matrix on the top of the stack		
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();	
	
	//move
	glTranslatef( 
				 s->position.x,
				 s->position.y,
				 s->position.z );
	
	//rotate
	glRotatef( s->spriteRotation, 0, 0, 1 );
	
	//and then scale with the pixel size
	glScalef( 
			 s->scale.x,
			 s->scale.y, 
			 s->scale.z );
	
	Mesh* m = currentRenderState->getMesh();
	switch( m->getTriangleMode() )
	{
		case Mesh::TM_STRIP:
			glDrawArrays(GL_TRIANGLE_STRIP, 0, m->getVertexCount());
			break;
			
		case Mesh::TM_LIST:
			glDrawArrays(GL_TRIANGLES, 0, m->getVertexCount());
			break;
	}	
	
	//reset original view on the top of the stack
	glPopMatrix();
}

void Render::endFrame()
{			
	// This application only creates a single color renderbuffer which is already bound at this point.
	// This call is redundant, but needed if dealing with multiple renderbuffers.
	if( !frameStarted )
		return;
	
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	
	[context presentRenderbuffer:GL_RENDERBUFFER_OES];
	
	frameStarted = false;
}

void Render::renderLayer( RenderableList* list )
{
	Renderable* s;

	for( uint i = 0; i < list->size(); ++i )
	{
		s = list->at(i);
		
		if( s->isVisible() && (!cullingEnabled || s->_canBeRenderedBy( viewport ) ) )
			renderElement( s );
	}
}

