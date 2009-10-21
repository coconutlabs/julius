HOW TO COMPILE JULIUS ON MSVC / THE JULIUS CLASS
=================================================

This file explains how to compile Julius on Microsoft Visual C++ 2008.
A sample application "SampleApp" and the Julius wrapper class is also
described here.  See below to see how to compile Julius and test them.

This package has been developed and tested on Visual C++ 2008 Express
Edition on Windows Vista 32bit/64bit.


1. Preparation
===============

"Microsoft DirectX SDK" is required to compile Julius.
You can get it from the Microsoft Web.

Julius also uses these two open-source libraries:

   - zlib
   - portaudio (V19)

The pre-compiled win32 libraries and header files are already
included under the "zlib" and "portaudio" directory.

You need an acoustic model and a language model to run Julius as speech recognizer.
See the Julius Web for more details.


2. Compile
===========

Simply open the "JuliusLib.sln" file, and build it!
You will get "julius.exe" and "SampleApp.exe" under "Debug" or "Release"
directory.

If you got an error when linking "zlib" or "portaudio", try compiling them
by your own.  Get the sources, compile it, and then place all the .h and generated
.lib files under the corresponding directories, and rebuild Julius.  If you have
re-compiled portaudio library, you may have to copy the generated portaudio DLL file
into the "Release" and "Debug" directories.


3. Test
========

! You need an acoustic model and a language model to run Julius as speech recognizer.
! See the Julius Web for more details.

"julius.exe" is a console application, which just runs as the same as the normal win32 version.

The "SampleApp.exe" is a sample GUI application.  Julius will start and the engine events will
be output to the main window.  You can test it by the procedure below:

   a) Prepare an acoustic model, language model and jconf file for them.
   b) Place a Jconf file as "fast.jconf" at the same directory as the exe.
   c) Modify the locale setting at line 98 of SampleApp.cpp to match the
      character encoding of your language model.
   d) Recompile and Run it.

The Julius output log will be stored in a text file "juliuslog.txt".



4. The Julius Class
====================

SampleApp uses a simple class definition "Julius.cpp" and "Julius.h".
They defines a wrapper class named "cJulius" that handles speech
recognition in a standard Windows messaging style.  You can try to
use it like this:

-----------------------------------------------------------------
#include "Julius.h"

cJulius julius;

....

// Windows Procedure callback
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch( message ) {
	case WM_CREATE:
	    // start Julius when the main window is created
	    julius.initialize( "fast.jconf" );
	    julius.startProcess( hWnd );
	    break;
	case WM_JULIUS:
            // Julius events
	    switch( LOWORD( wParam ) ) {
		case JEVENT_AUDIO_READY: ...
		case JEVENT_RECOG_BEGIN: ...
		case JEVENT_RESULT_FINAL:....
	    }
	.....
    }
    ...
}
-----------------------------------------------------------------

See SampleApp.cpp and Julius.cpp for details.


5.  About the character codes in the sources
=============================================

The source code of Julius contains Japanese characters at EUC-JP encoding.
If you want to read them in MSVC++, convert them to UTF-8.


6.  History
==============

2009/11 (ver.4.1.3)

	INITIAL RELEASE.
