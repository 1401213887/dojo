#ifndef ANDROIDPLATFORM_H
#define ANDROIDPLATFORM_H

#include "Platform.h"
#include "InputSystem.h"
#include "SoundManager.h"
#include "Vector.h"

#ifdef PLATFORM_ANDROID

#include <string>
#include "AndroidGLExtern.h"
#include <android/sensor.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include "Timer.h"

extern "C" {
	void android_main(struct android_app* state);
	int32_t android_handle_input(struct android_app* app, AInputEvent* event);
	void android_handle_cmd(struct android_app* app, int32_t cmd);
}

namespace Dojo
{
	class AndroidPlatform : public Platform
	{
	public:

		AndroidPlatform(const Table& table);

		virtual void initialise(Game *game);
		virtual void shutdown();

		virtual void acquireContext();
		virtual void present();

		virtual void step( float dt );
		virtual void loop();

		virtual void setFullscreen( bool fullscreen )
		{
			//android can only fullscreen 
		}


		virtual bool isNPOTEnabled()
		{
			DEBUG_MESSAGE("AndroidPlatform::isNPOTEnabled");
			return true; 
		}
                ///CALL THIS BEFORE USING ANY OTHER THREAD FOR GL OPERATIONS
                virtual void prepareThreadContext(){
			//???
		}
		
		virtual GLenum loadImageFile( void*& bufptr, const String& path, int& width, int& height, int & pixelSize );
		virtual String getAppDataPath();
		virtual String getResourcesPath();
		virtual String getRootPath();		
		/*
		   I can't make a dir in internal storage, but is an INTERNAL storage 
		   $(Appdata)/$(GameName)/....
		   to
		   $(root)/$(app storage)/files/.....
		*/
		virtual String _getTablePath( Table* dest, const String& absPath );
		
		//TODO

		virtual void openWebPage( const std::string& site ){}	
		virtual void openWebPage( const String& site ){}
		

	protected:
		
		void ResetDisplay();
		int32_t width, height;	
		Timer frameTimer;
		String apkdir;
		
		//android	
		//app manager
		struct android_app* app;
		ASensorManager* sensorManager;
		const ASensor* accelerometerSensor;
		ASensorEventQueue* sensorEventQueue;
		//android accelerometer
		void UpdateEvent();
		
		//openGL EGL
		int running;
		int isInPause;
		EGLDisplay display;
		EGLSurface surface;
		EGLContext context;

		friend int32_t ::android_handle_input(struct android_app* app, AInputEvent* event) ;
		friend void ::android_handle_cmd(struct android_app* app, int32_t cmd);
		
	private:
	};

}

#endif

#endif
