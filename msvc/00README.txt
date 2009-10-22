HOW TO COMPILE JULIUS ON MSVC / THE JULIUS CLASS
=================================================

This file explains how to compile Julius on Microsoft Visual C++ 2008.
A sample application "SampleApp" and the Julius wrapper class is also
described here.  See below to see how to compile Julius and test them.

This package has been developed and tested on Visual C++ 2008 Express
Edition on Windows Vista 32bit/64bit, and Visual C++ 2008 Professional
on Windows XP.


You need an acoustic model and a language model to run Julius as
speech recognizer.  You should also have a jconf configuration file.
Julius is not distirbuted with models.  If you don't know what to do,
see the Julius Web for details.


1. Preparation
===============

"Microsoft DirectX SDK" is required to compile Julius.
You can get it from the Microsoft Web.

Julius also uses these two open-source libraries:

   - zlib
   - portaudio (V19)

The pre-compiled win32 libraries and header files are already
included under the "zlib" and "portaudio" directory.

"SampleApp" requires acoustic model, language model and jconf file
that also works with Julius.  The jconf file should be placed as
"default.jconf" in the same directory as "SampleApp.exe".


2. Compile
===========

Simply open the "JuliusLib.sln" file, and build it!  You will get
"julius.exe" and "SampleApp.exe" under "Debug" or "Release" directory.

If you got an error when linking "zlib" or "portaudio", try compiling
them by your own.  Get the sources, compile it, and then place all the
.h and generated .lib files under the corresponding directories, and
rebuild Julius.  If you have re-compiled portaudio library, you may
have to copy the generated portaudio DLL file into the "Release" and
"Debug" directories.


3. Test
========

You need an acoustic model and a language model to run Julius as
speech recognizer.  You should also have a jconf configuration file.
Julius is not distirbuted with models.  If you don't know what to do,
see the Julius Web for details.

3.1  julius.exe
-----------------

"julius.exe" is a console application, which runs as the same as the
distributed win32 version of Julius.  You can run it from command
prompt with a working jconf file:

    % julius.exe -C xxx.jconf

3.2  SampleApp.exe
-------------------

"SampleApp.exe" is a sample GUI application which runs Julius engine
and dump each speech event to a main window.

Before start, you should place a working jconf file as "default.jconf"
at the same directory as "SampleApp.exe".

When SampleApp is run, the Julius engine will start inside as a
separate thread, and will send messages to the main window at each
speech event (trigger, recognition result, etc.).

If you have some trouble displaying the results, try modifying the
locale setting at line 98 of SampleApp.cpp to match your language
model and re-compile.

The Julius enging output will be stored in a text file "juliuslog.txt".
Please check it if you encounter engine error.


4. The Julius Class
====================

A simple class definition "Julius.cpp" and "Julius.h" is used in
SampleApp.  They defines a wrapper class named "cJulius" that utilizes
JuliusLib functions in Windows messaging style.  You can use it in
your application like this:

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
