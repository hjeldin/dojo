//
//  ApplePlatform.cpp
//  dojo
//
//  Created by Tommaso Checchi on 5/16/11.
//  Copyright 2011 none. All rights reserved.
//

#include "ApplePlatform.h"

#import <AudioToolbox/AudioToolbox.h>

#include "Timer.h"
#include "Render.h"
#include "SoundManager.h"
#include "InputSystem.h"
#include "Game.h"
#include "Utils.h"
#include "Table.h"

using namespace Dojo;
using namespace std;

ApplePlatform::ApplePlatform()
{
    pool = [[NSAutoreleasePool alloc] init];
}

ApplePlatform::~ApplePlatform()
{
	[pool release];	
}

void ApplePlatform::step( float dt )
{
	Timer frameTimer;
	
    game->loop(dt);
    
    render->render();
    sound->update(dt);
	
	realFrameTime = frameTimer.getElapsedTime();
}

std::string ApplePlatform::getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )
{
	NSString* NSName = Utils::toNSString( name );
	NSString* NSType = Utils::toNSString( type );
	NSString* NSPath = Utils::toNSString( path );
	
	NSString* res = [[NSBundle mainBundle] pathForResource:NSName ofType:NSType inDirectory:NSPath ];
	
	[NSName release];
	[NSType release];
	[NSPath release];
	
	if( res )
		return Utils::toSTDString( res );
	else
		return "";
}


void ApplePlatform::getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )
{	
	DEBUG_ASSERT( type.size() );
	DEBUG_ASSERT( path.size() );
	
    NSString* nstype = Utils::toNSString( type );
	NSString* nspath = Utils::toNSString( path );
	
	NSArray* paths = [[NSBundle mainBundle] pathsForResourcesOfType:nstype inDirectory:nspath];
	
	//convert array
	for( int i = 0; i < [paths count]; ++i )
		out.push_back( Utils::toSTDString( [paths objectAtIndex:i] ) );
	
	[paths release];
	[nspath release];
	[nstype release];
}

uint ApplePlatform::loadFileContent( char*& bufptr, const std::string& path )
{
    bufptr = NULL;
	
	DEBUG_ASSERT( path.size() );
	
	NSString* NSPath = Utils::toNSString( path );
	
	NSData* data = [[NSData alloc] initWithContentsOfFile: NSPath ];
	
	if( !data )
		return false;
	
	uint size = (uint)[data length];
	
	//alloc the new buffer
	bufptr = (char*)malloc( size );
	memcpy( bufptr, [data bytes], size );
	
	[data release];
	
	return size;
}


void ApplePlatform::loadPNGContent( void*& imageData, const std::string& path, uint& width, uint& height )
{
	width = height = 0;
	
	NSString* imgPath = Utils::toNSString( path );
	//magic OBJC code
	NSData *texData = [[NSData alloc] initWithContentsOfFile: imgPath ];
	
	CGDataProviderRef prov = CGDataProviderCreateWithData( 
														  NULL, 
														  [texData bytes], 
														  [texData length], 
														  NULL );
	CGImageRef CGImage = CGImageCreateWithPNGDataProvider( prov, NULL, false, kCGRenderingIntentDefault );	
	
	width = (int)CGImageGetWidth(CGImage);
	height = (int)CGImageGetHeight(CGImage);	
	
	uint internalWidth = Math::nextPowerOfTwo( width );
	uint internalHeight = Math::nextPowerOfTwo( height );
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	imageData = malloc( internalWidth * internalHeight * 4 );
	
	memset( imageData, 0, internalWidth * internalHeight * 4 );
	
	CGContextRef context = CGBitmapContextCreate(imageData, 
												 internalWidth, 
												 internalHeight, 
												 8, 
												 4 * internalWidth, 
												 colorSpace, 
												 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
	
	CGContextClearRect( context, CGRectMake( 0, 0, internalWidth, internalHeight ) );
	CGContextTranslateCTM( context, 0, internalHeight - height );
	CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), CGImage );
	
	//correct premultiplied alpha
	int pixelcount = internalWidth*internalHeight;
	unsigned char* off = (unsigned char*)imageData;
	for( int pi=0; pi<pixelcount; ++pi )
	{
		unsigned char alpha = off[3];
		if( alpha!=255 && alpha!=0 )
		{
			off[0] = ((int)off[0])*255/alpha;
			off[1] = ((int)off[1])*255/alpha;
			off[2] = ((int)off[2])*255/alpha;
		}
		off += 4;
	}
	
	//free everything
	CGContextRelease(context);	
	CGColorSpaceRelease( colorSpace );
	CGImageRelease( CGImage );
	CGDataProviderRelease( prov );
	[texData release];
}


uint ApplePlatform::loadAudioFileContent( ALuint& buffer, const std::string& path )
{
	//only load caf files on iphone
	DEBUG_ASSERT( Utils::hasExtension( "caf", path ) );
	
	NSString* filePath = Utils::toNSString( path );
	
	// first, open the file	
	AudioFileID fileID;
	// use the NSURl instead of a cfurlref cuz it is easier
	NSURL * afUrl = [NSURL fileURLWithPath:filePath];
	
	OSStatus result = AudioFileOpenURL((CFURLRef)afUrl, kAudioFileReadPermission, 0, &fileID);
	
	UInt64 outDataSize; 
	UInt32 propertySize, writable;
	
	AudioFileGetPropertyInfo( fileID, kAudioFilePropertyAudioDataByteCount, &propertySize, &writable );
	AudioFileGetProperty( fileID, kAudioFilePropertyAudioDataByteCount, &propertySize, &outDataSize);
	UInt32 fileSize = (UInt32)outDataSize;
	
	UInt32 freq;
	
	AudioFileGetPropertyInfo( fileID, kAudioFilePropertyBitRate, &propertySize, &writable );
	AudioFileGetProperty( fileID, kAudioFilePropertyBitRate, &propertySize, &freq );
	
	// this is where the audio data will live for the moment
	void* outData = malloc(fileSize);
	
	// this where we actually get the bytes from the file and put them
	// into the data buffer
	result = AudioFileReadBytes(fileID, false, 0, &fileSize, outData);
	AudioFileClose(fileID); //close the file
	
	// jam the audio data into the new buffer
	alBufferData( buffer, AL_FORMAT_STEREO16, outData, fileSize, freq/32); 
	
	free( outData );
	
	return fileSize;
}

NSString* ApplePlatform::_getDestinationFilePath( Table* table, const std::string& absPath )
{	
	DEBUG_ASSERT( table );
	DEBUG_ASSERT( absPath.size() == 0 || absPath.at( absPath.size()-1 ) != '/' ); //need to NOT have the terminating /
	
	NSString* fullPath;	
	std::string fileName = "/" + game->getName() + "/" + table->getName() + ".table";
	
	//save this in a file in the user preferences folder, or in the abs path
	if( absPath.size() == 0 )
	{
		NSArray* nspaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString* nspath = [nspaths objectAtIndex:0];
		NSString* nsname = Utils::toNSString( fileName );
		
		fullPath = [nspath stringByAppendingString:nsname];
		
		[nsname release];
		[nspaths release];
	}
	else
	{
		fullPath = Utils::toNSString( absPath + fileName );
	}
	
	return fullPath;
}

void ApplePlatform::load( Table* dest, const std::string& absPath )
{
	NSString* fullPath = _getDestinationFilePath( dest, absPath );
	
	//read file
	NSData* nsfile = [[NSData alloc] initWithContentsOfFile:fullPath];
	
	//do nothing if file doesn't exist
	if( nsfile )
	{
		//drop the data in a stringstr - TODO don't duplicate it
		std::stringstream str;
		str.write( (char*)[nsfile bytes], [nsfile length] );
	
		dest->deserialize( str );
		
		[nsfile release];
	}
	
	[fullPath release];
}

void ApplePlatform::save( Table* src, const std::string& absPath )
{
	//serialize to stream
	std::stringstream buf;
	src->serialize( buf );
	
	NSString* fullPath = _getDestinationFilePath( src, absPath );
	
	//drop into NSData
	NSData* nsfile = [[NSData alloc] 	initWithBytesNoCopy:(void*)buf.str().c_str() 
										length:buf.str().size() 
										freeWhenDone:false ];
	
	//drop on file
	[nsfile writeToFile:fullPath atomically:false];
	
	[fullPath release];
	[nsfile release];
}

void ApplePlatform::openWebPage( const std::string& site )
{
	NSString* nssite = Utils::toNSString(site);
	NSURL* url = [NSURL URLWithString:nssite];
	
#ifdef PLATFORM_OSX
	[[NSWorkspace sharedWorkspace] openURL: url ];
#else
	[[UIApplication sharedApplication] openURL:url];
#endif
	
	[nssite release];
	[url release];
}