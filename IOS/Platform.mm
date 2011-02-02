#include "Platform.h"

#include "Utils.h"
#include "dojomath.h"

using namespace Dojo;

std::string Platform::getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path )
{
	NSString* NSName = Utils::toNSString( name );
	NSString* NSType = Utils::toNSString( type );
	NSString* NSPath = Utils::toNSString( path );
	
	NSString* res = [[NSBundle mainBundle] pathForResource:NSName ofType:NSType inDirectory:NSPath ];
	
	if( res )
		return Utils::toSTDString( res );
	else
		return "";
}

void Platform::getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out )
{
	DEBUG_ASSERT( type.size() );
	DEBUG_ASSERT( path.size() );
	
	NSString* NSType = Utils::toNSString( type );
	NSString* NSPath = Utils::toNSString( path );
	
	NSArray* paths = [[NSBundle mainBundle] pathsForResourcesOfType:NSType inDirectory:NSPath ];
	
	for( uint i = 0; i < [paths count]; ++i )
		out.push_back( Utils::toSTDString( (NSString*)[paths objectAtIndex:i] ) );
								  
}

uint Platform::loadFileContent( char*& bufptr, const std::string& path )
{
	bufptr = NULL;
	
	DEBUG_ASSERT( path.size() );
	
	NSString* NSPath = Utils::toNSString( path );
	
	NSData* data = [[NSData alloc] initWithContentsOfFile: NSPath ];
	
	if( !data )
		return false;
	
	uint size = [data length];
	
	//alloc the new buffer
	bufptr = (char*)malloc( size );
	memcpy( bufptr, [data bytes], size );
	
	[data release];
	
	return size;
}

void Platform::loadPNGContent( void*& imageData, const std::string& path, uint& width, uint& height )
{
	width = height = 0;
	imageData = NULL;
	
	NSString* imgPath = Utils::toNSString( path );
	//magic OBJC code
	NSData *texData = [[NSData alloc] initWithContentsOfFile: imgPath ];
	UIImage *image = [[UIImage alloc] initWithData:texData];
	
	if (image == nil)
		return;
	
	width = CGImageGetWidth(image.CGImage);
	height = CGImageGetHeight(image.CGImage);	
	
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
	CGColorSpaceRelease( colorSpace );
	CGContextClearRect( context, CGRectMake( 0, 0, internalWidth, internalHeight ) );
	CGContextTranslateCTM( context, 0, internalHeight - height );
	CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), image.CGImage );
	
	CGContextRelease(context);
		
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
	
}