#include "stdafx.h"

#include "Texture.h"

#include "dojomath.h"

#include "Platform.h"

using namespace Dojo;

bool Texture::load()
{
	DEBUG_ASSERT( !loaded );
		
	glGenTextures(1, &glhandle );
	
	DEBUG_ASSERT( glhandle );
	
	glBindTexture( GL_TEXTURE_2D, glhandle );
		
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		
	DEBUG_ASSERT( textureType == "png" );
	
	loaded = _loadPNGToBoundTexture();

	if( !loaded )
		unload();
	
	//force disable filtering and alpha on too big textures
	enableBilinearFiltering();
		
	return loaded;
}

bool Texture::loadFromAtlas( Texture* tex, uint x, uint y, uint sx, uint sy )
{
	DEBUG_ASSERT( tex );
	DEBUG_ASSERT( tex->isLoaded() );
	DEBUG_ASSERT( !isLoaded() );	
			
	loaded = true;			
	textureType = "atlas";
	parentAtlas = tex;
	
	width = sx;
	height = sy;
	internalWidth = tex->internalWidth;
	internalHeight = tex->internalHeight;
	
	DEBUG_ASSERT( sx && sy && internalWidth && internalHeight );
		
	//copy bind handle
	glhandle = tex->glhandle;
	
	//find uv coordinates
	xOffset = (float)x/(float)internalWidth;
	yOffset = (float)y/(float)internalHeight;
	
	//find uv size
	xRatio = (float)sx/(float)internalWidth;
	yRatio = (float)sy/(float)internalHeight;
	
	return true;
}

void Texture::unload()
{		
	DEBUG_ASSERT( loaded );
	
	if( OBB )
	{
		OBB->unload();
		
		delete OBB;
		OBB = NULL;
	}
	
	if( !parentAtlas ) //don't unload parent texture!
	{
		DEBUG_ASSERT( glhandle );
		glDeleteTextures(1, &glhandle );
		
		glhandle = 0;
	}			
			
	loaded = false;
}


void Texture::_buildOptimalBillboard()
{
	OBB = new Mesh();
	
	OBB->setVertexFieldEnabled( Mesh::VF_POSITION2D, true );
	OBB->setVertexFieldEnabled( Mesh::VF_UV, true );
	
	OBB->begin( 4 );
	
	OBB->vertex( -0.5, -0.5 );		
	OBB->uv( _getXTextureOffset(), 
			 _getYTextureOffset() + _getYTextureUVRatio() );
	
	OBB->vertex( 0.5, -0.5 );		
	OBB->uv( _getXTextureOffset() + _getXTextureUVRatio(), 
			 _getYTextureOffset() + _getYTextureUVRatio() );
	
	OBB->vertex( -0.5, 0.5 );		
	OBB->uv( _getXTextureOffset(), 
			 _getYTextureOffset() );
	
	OBB->vertex( 0.5, 0.5 );
	OBB->uv( _getXTextureOffset() + _getXTextureUVRatio(), 
			 _getYTextureOffset() );
	
	OBB->end();			
}

bool Texture::_loadPNGToBoundTexture()
{
	void* imageData, *buf;
	
	Platform::getSingleton()->loadPNGContent( imageData, filePath, width, height );
		
	internalWidth = Math::nextPowerOfTwo( width );
	internalHeight = Math::nextPowerOfTwo( height );	
	
	npot = ( internalWidth != width || internalHeight != height );
	
	xRatio = (float)width/(float)internalWidth;
	yRatio = (float)height/(float)internalHeight;

	size = internalWidth * internalHeight * sizeof( float ) * 2;
							 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, internalWidth, internalHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
		
	free(imageData);
	
	return true;
}