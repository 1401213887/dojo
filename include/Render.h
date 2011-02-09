//
//  Render.h
//  NinjaTraining
//
//  Created by Tommaso Checchi on 4/23/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//
#ifndef Render_h__
#define Render_h__

#include "dojo_common_header.h"

#include "Array.h"
#include "Color.h"
#include "Vector.h"
#include "RenderState.h"
#include "BaseObject.h"

namespace Dojo {
	
	class Renderable;
	class Texture;
	class Viewport;
	class Mesh;
	class Platform;
	
	class Render : public BaseObject
	{	
	public:				
		
		enum RenderOrientation
		{
			RO_PORTRAIT,
			RO_PORTRAIT_REVERSE,
			RO_LANDSCAPE_LEFT,
			RO_LANDSCAPE_RIGHT
		};
				
		const static uint layerNumber = 10;
		
		typedef Array<Renderable*> RenderableList;
		typedef Array<RenderableList*> LayerList;
		
		Render( uint width, uint height, uint devicePixelScale, RenderOrientation);		
		
		~Render();		
						
		void addRenderable( Renderable* s, int layer );
				
		void removeRenderable( Renderable* s );
		
		void removeAllRenderables()
		{
			for( uint i = 0; i < negativeLayers.size(); ++i )
				negativeLayers.at(i)->clear();
			
			for( uint i = 0; i < positiveLayers.size(); ++i )
				positiveLayers.at(i)->clear();
		}
		
		void setViewport( Viewport* v );
		
		inline void setCullingEnabled( bool state )	{	cullingEnabled = state;	}
				
		void setInterfaceOrientation( RenderOrientation o );
		
		inline RenderOrientation getInterfaceOrientation()
		{
			return renderOrientation;
		}
		
		RenderableList* getLayer( int layerID );
		
		inline int getWidth()						{	return width;		}
		inline int getHeight()						{	return height;		}
		inline float getContentScale()				{	return devicePixelScale;	}
		
		inline Viewport* getViewport()				{	return viewport;	}
		
		inline bool isValid()						{	return valid;		}
						
		void startFrame();
		
		void renderElement( Renderable* r );
		
		void renderLayer( RenderableList* list );
		
		void endFrame();
		
		//renders all the layers and their contained Renderables in the given order
		void render()
		{			
			startFrame();
			
			//first render from the most negative to -1
			if( negativeLayers.size() > 0 )
			{				
				for( uint i = negativeLayers.size(); i >= 0; --i )
					renderLayer( negativeLayers.at(i) );
			}
			
			//then from 0 to the most positive
			for( uint i = 0; i < positiveLayers.size(); ++i )
				renderLayer( positiveLayers.at(i) );
			
			endFrame();
		}
				
	protected:	

		Platform* platform;
		
		bool valid;
						
		// The pixel dimensions of the CAEAGLLayer
		int width, height;
		uint viewportWidth, viewportHeight;
		float devicePixelScale;
		
		float renderRotation;
		RenderOrientation renderOrientation, deviceOrientation;
						
		Viewport* viewport;	
		Vector viewportPixelRatio, textureScreenPixelRatio, spriteScreenPixelSize;
		
		RenderState* currentRenderState, *firstRenderState;
		
		bool cullingEnabled;		
		
		bool frameStarted;
		
		LayerList negativeLayers, positiveLayers;

		void _updateGLViewportDimensions();
	};		
}

#endif