======================================================================
                  Large Vocabulary Continuous Speech
                          Recognition Engine

                                Julius

                                                (Rev 4.0   2007/12/19)
                                                (Rev 3.5.3 2006/12/29)
                                                (Rev 3.5   2005/11/11)
                                                (Rev 3.4.2 2004/04/30)
                                                (Rev 3.4   2003/10/01)
                                                (Rev 3.3   2002/09/12)
                                                (Rev 3.2   2001/06/18)
                                                (Rev 3.1   2000/05/11)
                                                (Rev 3.0   2000/02/14)
                                                (Rev 2.0   1999/02/20)
                                                (Rev 1.0   1998/02/20)

 Copyright (c) 1991-2007 Kawahara Lab., Kyoto University
 Copyright (c) 1997-2000 Information-technology Promotion Agency, Japan
 Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 Copyright (c) 2005-2007 Julius project team, Nagoya Institute of Technology
 All rights reserved
======================================================================

About Julius
=============

"Julius" is a high-performance, two-pass large vocabulary continuous
speech recognition (LVCSR) decoder software for speech-related
researches and developments.  It supports N-gram based dictataion (N
unlimited) , DFA grammar based parsing, and one-pass isolated word
recognition.  Phone context dependencies are supported up to triphone.
It can perform a multi-model decoding, a recognition using several LMs
and AMs simultaneously with a single processor, and also support for
"hot plugging" of arbitrary modules at run time.  The core engine is
implemented as a C library along with a simple API, which can be
easily integrated into various applications.  Standard model formats
for famous tools such as HTK, CMU-Cam SLM toolkit, etc. are adopted.

The main platform is Linux and other Unix workstations, and also works
on Windows (SAPI/console). Julius is distributed with open license
together with source codes.


What's new in Julius-4.0
==========================

The Julius rev.4.0 is a full major version up, a re-innovation of the
decoder as a flexible speech recognition engine.  The internal
structures are re-organized and modularized thoroughly, which results
in the librarization of core engine, enhancement and unification of
language model, realization of multi-decoding with hot plugging
feature.  The major features are listed below:

 - Engine core becomes separate C library with simple API
 - Can handle various LM, thus Julius and Julian are integrated
 - Multi-decoding with multiple models
 - Support to add and remove models while running the decoder
 - Support N-gram longer than 4 (N now unlimited)
 - User-defined LM function
 - Confusion network output
 - GMM-based and decoder-based VAD
 - New tools added, new functions added
 - Memory efficiency is improved

Julius-4.0 ensures the compatibility with Julius-3.x for its usage and
Jconf configuration, so that one can easily migrate to Julius-4.
The decoding performance of Julius-4.0 is still kept as the same as
the latest release (Julius-3.5.3) for now.


Contents of Julius-4.0
=========================

	(Documents with suffix "ja" are written in Japanese)

	00readme.txt		ReadMe (This file)
	LICENSE.txt		Terms and conditions of use
	Release.txt		Release note / ChangeLog
	configure		configure script
	configure.in		
	Sample.jconf		Sample configuration file for Julius-3.5.3
	julius/			Julius/Julian 3.5.3 sources
	libsent/		Julius/Julian 3.5.3 library sources
	adinrec/		Record one sentence utterance to a file
	adintool/		Record/split/send/receive speech data
	generate-ngram/		Tool to generate random sentences from N-gram
	gramtools/		Tools to build and test recognition grammar
	jcontrol/		A sample network client module 
	mkbingram/		Convert N-gram to binary format
	mkbinhmm/		Convert ascii hmmdefs to binary format
	mkgshmm/		Model conversion for Gaussian Mixture Selection
	mkss/			Estimate noise spectrum from mic input
	support/		some tools to compile julius/julian from source
	olddoc/			ChangeLogs before 3.2


Documentation
===============

The up-to-date documentations are available at the Julius Web site:

    http://julius.sourceforge.jp/


License
========

Julius is an open-source software provided as is.  For more
information about the license, please refer to the "LICENSE.txt" file
included in this archive.


Contact Us
===========

The contact address of Julius/Julian development team is:
(please replace 'at' with '@')

      "julius-info at lists.sourceforge.jp"


EOF
