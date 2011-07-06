//
//  ApplePlatform.cpp
//  dojo
//
//  Created by Tommaso Checchi on 5/16/11.
//  Copyright 2011 none. All rights reserved.
//

#include "ApplePlatform.h"

#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/NSLocale.h>

#ifdef PLATFORM_IOS
	#import <UIKit/UIApplication.h>
#endif

#include "Timer.h"
#include "Render.h"
#include "SoundManager.h"
#include "InputSystem.h"
#include "Game.h"
#include "Utils.h"
#include "Table.h"
#include "dojostring.h"
#include "StringReader.h"

using namespace Dojo;
using namespace std;

ApplePlatform::ApplePlatform( const Table& config ) :
Platform( config ),
testSuite( std::cout )
{
    pool = [[NSAutoreleasePool alloc] init];
	
	locale = String( [[NSLocale currentLocale] localeIdentifier] );
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

String ApplePlatform::getCompleteFilePath( const String& name, const String& type, const String& path )
{
	NSString* NSName = name.toNSString();
	NSString* NSType = type.toNSString();
	NSString* NSPath = path.toNSString();
	
	NSString* res = [[NSBundle mainBundle] pathForResource:NSName ofType:NSType inDirectory:NSPath ];
	
	/*[NSName release];
	[NSType release];
	[NSPath release];*/
	
	if( res )
		return String( res );
	else
		return String::EMPTY;
}


void ApplePlatform::getFilePathsForType( const String& type, const String& path, std::vector<String>& out )
{	
	DEBUG_ASSERT( type.size() );
	DEBUG_ASSERT( path.size() );
	
    NSString* nstype = type.toNSString();
	NSString* nspath = path.toNSString();
	
	NSArray* paths = [[NSBundle mainBundle] pathsForResourcesOfType:nstype inDirectory:nspath];
	
	//convert array
	for( int i = 0; i < [paths count]; ++i )
		out.push_back( String( [paths objectAtIndex:i] ) );
	
	//[nspath release];
	//[nstype release];
}


NSString* ApplePlatform::_getFullPath( const String& path )
{	
	//is it already absolute?
	if( path[0] == '/' )
		return path.toNSString();
		
	NSString* nsbundlepath = [[NSBundle mainBundle] bundlePath];
	NSString* nspath = [nsbundlepath stringByAppendingString: ("/" + path).toNSString() ];
		
	return nspath;
}

NSString* ApplePlatform::_getDestinationFilePath( Table* table, const String& absPath )
{	
	DEBUG_ASSERT( table );
	DEBUG_ASSERT( absPath.size() == 0 || absPath.at( absPath.size()-1 ) != '/' ); //need to NOT have the terminating /
	
	NSString* fullPath;	
	
	//save this in a file in the user preferences folder, or in the abs path
	if( absPath.size() == 0 )
	{		
		String fileName = "/" + game->getName() + "/" + table->getName() + ".ds";
		
		NSArray* nspaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString* nspath = [nspaths objectAtIndex:0];
		NSString* nsname = fileName.toNSString();
		
		fullPath = [nspath stringByAppendingString:nsname];
		
		/*[fullPath retain];
		 [nsname release];
		 [nspaths release];*/
	}
	else
	{		
		fullPath = _getFullPath( absPath.toNSString() );
	}
	
	return fullPath;
}

void ApplePlatform::load( Table* dest, const String& absPath )
{
	NSString* fullPath = _getDestinationFilePath( dest, absPath );
	
	//read file
	NSData* nsfile = [[NSData alloc] initWithContentsOfFile:fullPath];
	
	//do nothing if file doesn't exist
	if( !nsfile )
		return;
	
	//drop the data in a String - TODO don't duplicate it
	String buf;
	buf.appendRaw( (char*)[nsfile bytes], [nsfile length] );
	
	dest->setName( Utils::getFileName( String( fullPath ) ) );
	
	StringReader in( buf );
	dest->deserialize( in );
	
	/*[nsfile release];
	 [fullPath release];*/
}

void ApplePlatform::save( Table* src, const String& absPath )
{
	//serialize to stream
	String buf;
	src->serialize( buf );
	
	//ensure application dir created
	_createApplicationDirectory();
	
	NSString* fullPath = _getDestinationFilePath( src, absPath );
	
	//drop into NSData
	NSData* nsfile = [[NSData alloc] 	initWithBytesNoCopy:(void*)buf.data()
												   length:buf.byteSize()
											 freeWhenDone:false ];
	
	//drop on file
	[nsfile writeToFile:fullPath atomically:true];
	
	/*[fullPath release];
	 [nsfile release];*/
}


uint ApplePlatform::loadFileContent( char*& bufptr, const String& path )
{
    bufptr = NULL;
	
	DEBUG_ASSERT( path.size() );
		
	NSData* data = [ NSData dataWithContentsOfFile: _getFullPath( path ) ];
	
	if( !data )
		return false;
	
	uint size = (uint)[data length];
	
	//alloc the new buffer
	bufptr = (char*)malloc( size );
	memcpy( bufptr, [data bytes], size );
	
	//[data release];
	
	return size;
}


void ApplePlatform::loadPNGContent( void*& imageData, const String& path, uint& width, uint& height )
{
	width = height = 0;
	
	NSString* imgPath = path.toNSString();
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
			float a = ((float)(alpha)/255.f);
			
			off[0] = ((float)off[0]) / a;
			off[1] = ((float)off[1]) / a;
			off[2] = ((float)off[2]) / a;
		}
		off += 4;
	}
	
	//free everything
	CGContextRelease(context);	
	CGColorSpaceRelease( colorSpace );
	CGImageRelease( CGImage );
	CGDataProviderRelease( prov );
	
	//[texData release];
}


uint ApplePlatform::loadAudioFileContent( ALuint& buffer, const String& path )
{
	//only load caf files on iphone
	DEBUG_ASSERT( Utils::hasExtension( "caf", path ) );
	
	NSString* filePath = path.toNSString();
	
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

void ApplePlatform::_createApplicationDirectory()
{	
	String dirname = "/" + game->getName();
	
	NSArray* nspaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString* nspath = [nspaths objectAtIndex:0];
	NSString* nsgamedir = dirname.toNSString();	
	NSString* nsdir = [nspath stringByAppendingString:nsgamedir];
	
	//check if the directory exists, if not existing create it!
	NSFileManager *fileManager= [NSFileManager defaultManager]; 
	if(![ fileManager fileExistsAtPath:nsdir ])
	{
		bool success = [fileManager 
						createDirectoryAtPath:nsdir
						withIntermediateDirectories:YES 
						attributes:nil 
						error:NULL];
		
		DEBUG_ASSERT( success );
	}	
}

void ApplePlatform::openWebPage( const String& site )
{
	NSURL* url = [NSURL URLWithString: site.toNSString() ];
	
#ifdef PLATFORM_OSX
	[[NSWorkspace sharedWorkspace] openURL: url ];
#else
	[[UIApplication sharedApplication] openURL:url];
#endif
}