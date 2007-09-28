/**
 * @file   recogloop.c
 * @author Akinobu Lee
 * @date   Sun Sep 02 21:12:52 2007
 * 
 * <JA>
 * @brief  メイン認識ループ
 * </JA>
 * 
 * <EN>
 * @brief  Main recognition loop
 * </EN>
 * 
 * $Revision: 1.1 $
 * 
 */
/*
 * Copyright (c) 1991-2006 Kawahara Lab., Kyoto University
 * Copyright (c) 1997-2000 Information-technology Promotion Agency, Japan
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2006 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#include "app.h"

void
main_recognition_stream_loop(Recog **recoglist, int recognum)
{
  Recog *recog;
  Jconf *jconf;
  Model *model;
  int file_counter;
  int ret;
  FILE *mfclist;
  static char speechfilename[MAXPATHLEN];	/* pathname of speech file or MFCC file */
  
  recog = recoglist[0];

  jconf = recog->jconf;

  /* reset file count */
  file_counter = 0;
  
  if (jconf->input.speech_input == SP_MFCFILE) {
    if (jconf->input.inputlist_filename != NULL) {
      /* open filelist for mfc input */
      if ((mfclist = fopen(jconf->input.inputlist_filename, "r")) == NULL) { /* open error */
	fprintf(stderr, "Error: cannot open inputlist \"%s\"\n", jconf->input.inputlist_filename);
	return;
      }
    }
  }
      
  /**********************************/
  /** Main Recognition Stream Loop **/
  /**********************************/
  for (;;) {

    printf("\n");
    if (verbose_flag) printf("------\n");
    fflush(stdout);

    /*********************/
    /* open input stream */
    /*********************/
    if (jconf->input.speech_input == SP_MFCFILE) {
      /* from MFCC parameter file (in HTK format) */
      VERMES("### read analyzed parameter\n");
      if (jconf->input.inputlist_filename != NULL) {	/* has filename list */
	do {
	  if (getl_fp(speechfilename, MAXPATHLEN, mfclist) == NULL) {
	    fclose(mfclist);
	    fprintf(stderr, "%d files processed\n", file_counter);
#ifdef REPORT_MEMORY_USAGE
	    print_mem();
#endif
	    return;
	  }
	} while (speechfilename[0] == '\0' || speechfilename[0] == '#');
      } else {
	if (get_line_from_stdin(speechfilename, MAXPATHLEN, "enter MFCC filename->") == NULL) {
	  fprintf(stderr, "%d files processed\n", file_counter);
#ifdef REPORT_MEMORY_USAGE
	  print_mem();
#endif
	  return;
	}
      }
      if (verbose_flag) printf("\ninput MFCC file: %s\n", speechfilename);

      /* open stream */
      ret = j_open_stream(recog, speechfilename);
      switch(ret) {
      case 0:			/* succeeded */
	break;
      case -1:      		/* error */
	/* go on to the next input */
	continue;
      case -2:			/* end of recognition */
	return;
      }

      /* start recognizing the stream */
      do {

	ret = j_recognize_stream_multi(recoglist, recognum);

	switch(ret) {
	case 1:	      /* paused by callback (stream may continues) */
	  /* do whatever you want while stopping recognition */

	  /* after here, recognition will restart */
	  break;
	case 0:			/* end of stream */
	  /* go on to the next input */
	  break;
	case -1: 		/* error */
	  return;
	}
      } while (ret == 1);
	  
      /* count number of processed files */
      file_counter++;

    } else {			/* raw speech input */

      VERMES("### read waveform input\n");
      /* begin A/D input */
      ret = j_open_stream(recog, NULL);
      switch(ret) {
      case 0:			/* succeeded */
	break;
      case -1:      		/* error */
	/* go on to next input */
	continue;
      case -2:			/* end of recognition process */
	if (jconf->input.speech_input == SP_RAWFILE) {
	  fprintf(stderr, "%d files processed\n", file_counter);
	} else if (jconf->input.speech_input == SP_STDIN) {
	  fprintf(stderr, "reached end of input on stdin\n");
	} else {
	  fprintf(stderr, "failed to begin input stream\n");
	}
	return;
      }
      /* start recognizing the stream */
      ret = j_recognize_stream_multi(recoglist, recognum);
      /* how to stop:
	 add a function to CALLBACK_POLL and call j_request_pause() or
	 j_request_terminate() in the function.
	 Julius will them stop search and call CALLBACK_PAUSE_FUNCTION.
	 after all callbacks in CALLBACK_PAUSE_FUNCTION was processed,
	 Julius resume the search.
      */
      if (ret == -1) {		/* error */
	return;
      }
      /* else, end of stream */
      
      /* count number of processed files */
      if (jconf->input.speech_input == SP_RAWFILE) {
	file_counter++;
      }
    }
  }

}
