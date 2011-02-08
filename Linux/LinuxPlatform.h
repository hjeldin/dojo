#pragma once

#include "dojo_common_header.h"

#include <OIS.h>

#include "Platform.h"
#include "Vector.h"

#include "Timer.h"

namespace Dojo
{
	class LinuxPlatform : public Platform, public OIS::MouseListener, public OIS::KeyListener
	{
	public:

		LinuxPlatform();

		virtual void initialise();
		virtual void shutdown();

		virtual void acquireContext();
		virtual void present();

		virtual void step( float dt );
		virtual void loop( float frameTime );

		virtual std::string getCompleteFilePath( const std::string& name, const std::string& type, const std::string& path );
		virtual void getFilePathsForType( const std::string& type, const std::string& path, std::vector<std::string>& out );
		virtual uint loadFileContent( char*& bufptr, const std::string& path );
		virtual void loadPNGContent( void*& bufptr, const std::string& path, uint& width, uint& height, bool POT );
		
		virtual void load(  Table* dest );
		virtual void save( Table* table );

		virtual void openWebPage( const std::string& site );

		virtual bool mouseMoved( const OIS::MouseEvent& arg );
		virtual bool mousePressed( const OIS::MouseEvent& arg, OIS::MouseButtonID id );
		virtual bool mouseReleased(	const OIS::MouseEvent& arg, OIS::MouseButtonID id );

		virtual bool keyPressed(const OIS::KeyEvent &arg);
		virtual bool keyReleased(const OIS::KeyEvent &arg);	

	protected:

		Display                 *dpy;
		Window                  root;
		GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
		XVisualInfo             *vi;
		Colormap                cmap;
		XSetWindowAttributes    swa;
		Window                  win;
		GLXContext              glc;
		XWindowAttributes       gwa;
		XEvent                  xev;

		int width, height;

		Timer frameTimer;

		OIS::InputManager* inputManager;
		OIS::Mouse* mouse;
		OIS::Keyboard* keys;

		Vector cursorPos;

		bool dragging;

		bool _hasExtension( const std::string& type, const std::string& nameOrPath );
		std::string _toNormalPath( const std::string& path );

		std::string _getUserDirectory();

		bool _initialiseWindow( const std::string& caption, uint w, uint h );

	private:
	};
}

