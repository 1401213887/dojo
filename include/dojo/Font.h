/*
 *  Font.h
 *  Dojo Training
 *
 *  Created by Tommaso Checchi on 5/27/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef Font_h__
#define Font_h__

#include "dojo_common_header.h"

#include "Vector.h"
#include "Texture.h"
#include "FrameSet.h"

#include "Utils.h"

#define FONT_PPI (1.f/64.f)

#define FONT_PAGE_SIDE 16
#define FONT_CHARS_PER_PAGE ( FONT_PAGE_SIDE * FONT_PAGE_SIDE )
#define FONT_MAX_PAGES (65535 / FONT_CHARS_PER_PAGE )

namespace Dojo 
{
	class Font : public BaseObject
	{
	public:

		class Page;
		class Character;
		
		class Character
		{
		public:			
			unichar character;
			uint gliphIdx;
			
			Vector uvPos;
			float uvWidth, uvHeight;
			
			uint pixelWidth;
			float widthRatio, heightRatio; //the ratio of the cell that is occupied

			float bearingU, bearingV;

			Page* page;
						
			Character()
			{

			}

			void init( Page* p, unichar c, int x, int y, int sx, int sy, FT_Glyph_Metrics* metrics );

			inline Texture* getTexture()
			{
				return page->getTexture();
			}
		};
		
		class Page
		{
		public:
			
			///create a new page for the given index
			Page( Font* font, int index );

			virtual ~Page()
			{
				texture->unload();

				delete texture;
			}

			inline Texture* getTexture() 
			{
				return texture;
			}

			///get the char in this page
			inline Character* getChar( unichar c )
			{
				DEBUG_ASSERT( _charInPage(c) );

				return &( chars[ c - firstCharIdx ] );
			}

			inline Font* getFont()
			{
				return font;
			}

		protected:

			Font* font;
			Texture* texture;
			uint index;
			unichar firstCharIdx;

			Character chars[ FONT_CHARS_PER_PAGE ];

			inline boolean _charInPage( unichar c ) 
			{
				return c >= firstCharIdx && c < (firstCharIdx + FONT_CHARS_PER_PAGE);
			}
		};
		
		Font( const std::string& path );
		
		virtual ~Font()
		{
			//clear every page
			unload();
		}
		
		inline uint getFontWidth()			{	return fontWidth;		}
		inline uint getFontHeight()			{	return fontHeight;		}

		inline Page* getPage( uint index )
		{
			DEBUG_ASSERT( index < FONT_MAX_PAGES );

			Page* res = pages[ index ];

			if( !res )
				pages[ index ] = res = new Page( this, index );

			return res;
		}

		inline Page* getPageForChar( unichar c )
		{
			return getPage( c / FONT_CHARS_PER_PAGE );
		}

		///returns the texture page and the uv min and max for that character
		inline Character* getCharacter( unichar c )
		{
			return getPageForChar( c )->getChar( c );
		}

		inline Texture* getTexture( unichar c )
		{
			return getPageForChar( c )->getTexture();
		}

		float getKerning( Character* next, Character* prev );

		inline float getSpacing()
		{
			return spacing;
		}

		inline float getCharIndex( Character* c )
		{
			return FT_Get_Char_Index( face, c->character );
		}

		inline bool isAntialiased()
		{
			return antialias;
		}

		inline bool isKerningEnabled()
		{
			return kerning;
		}

		///make sure that the pages at the given indices are loaded
		inline void preloadPages( const char pages[], uint n ) 
		{
			for( int i = 0; i < n; ++i )
				getPage( pages[i] );
		}

		///unload, remove and destroy the given page
		inline void destroyPage( uint idx )
		{
			DEBUG_ASSERT( idx < FONT_MAX_PAGES );

			if( pages[idx] ) 
			{
				delete pages[idx];
				pages[idx] = NULL;
			}
		}
		
		void unload()
		{
			for( uint i = 0; i < FONT_MAX_PAGES; ++i )
				destroyPage(i);
		}
		
	protected:
		
		std::string fontName;
		std::string fontFile;

		bool border;
		Color borderColor;

		bool antialias, kerning;

		float spacing;

		uint fontWidth, fontHeight; //measurements of the "character box"

		FT_Face face;

		///this has to be called each time that we need to use the face
		void _prepareFace();
		
		Page* pages[ FONT_MAX_PAGES ]; 
	};
}

#endif