/**
 * @file   recogmain.c
 * @author Akinobu Lee
 * @date   Wed Aug  8 14:53:53 2007
 * 
 * <JA>
 * @brief  認識処理のメインループ関数
 * </JA>
 * 
 * <EN>
 * @brief  Main loop function to execute recognition
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

#define GLOBAL_VARIABLE_DEFINE	///< Actually make global vars in global.h
#include <julius/julius.h>
#include <signal.h>
#if defined(_WIN32) && !defined(__CYGWIN32__)
#include <mbctype.h>
#include <mbstring.h>
#endif


/* ---------- utility functions -----------------------------------------*/
#ifdef REPORT_MEMORY_USAGE
/** 
 * <JA>
 * 通常終了時に使用メモリ量を調べて出力する (Linux, sol2
 * 
 * </JA>
 * <EN>
 * Get process size and output on normal exit. (Linux, sol2)
 * 
 * </EN>
 */
static void
print_mem()
{
  char buf[200];
  sprintf(buf,"ps -o vsz,rss -p %d",getpid());
  system(buf);
  fflush(stdout);
  fflush(stderr);
}
#endif
	  

/* --------------------- speech buffering ------------------ */

/** 
 * <JA>
 * @brief  検出区間の音声データをバッファに保存するための adin_go() callback
 *
 * この関数は，検出された音声入力を逐次バッファ @a speech に記録して
 * いきます．バッファ処理モード（＝非リアルタイムモード）で認識を行なう
 * ときに用いられます．
 * 
 * @param now [in] 検出された音声波形データの断片
 * @param len [in] @a now の長さ(サンプル数)
 * 
 * @return エラー時 -1 (adin_go は即時中断する)，通常時 0 (adin_go は
 * 続行する)，区間終了要求時 1 (adin_go は現在の音声区間を閉じる)．
 * 
 * </JA>
 * <EN>
 * @brief  adin_go() callback to score each detected speech segment to buffer.
 *
 * This function records the incomping speech segments detected in adin_go()
 * to a buffer @a speech.  This function will be used when recognition runs
 * in buffered mode (= non-realtime mode).
 * 
 * @param now [in] input speech samples.
 * @param len [in] length of @a now in samples
 * 
 * @return -1 on error (tell adin_go() to terminate), 0 on success (tell
 * adin_go() to continue recording), or 1 when this function requires
 * input segmentation.
 * </EN>
 */
static int
adin_cut_callback_store_buffer(SP16 *now, int len, Recog **recoglist, int recognum)
{
  /**
   * Temporal buffer to save the recorded-but-unprocessed samples
   * when the length of a speech segment exceeds the limit
   * (i.e. MAXSPEECHLEN samples).  They will be restored on the
   * next input at the top of the recording buffer.
   * 
   */
  static SP16 *overflowed_samples = NULL;
  /**
   * Length of above.
   * 
   */
  static int overflowed_samplenum;
  Recog *recog;

  recog = recoglist[0];
  
  /* poll for each input fragment */
  callback_multi_exec(CALLBACK_POLL, recoglist, recognum);

  if (recog->speechlen == 0) {		/* first part of a segment */
    /* termination check */
    if (recog->process_want_terminate ||/* TERMINATE ... force termination */
	!recog->process_active) { /* PAUSE ... keep recording when *triggering */
      return(-2);
    }
    if (overflowed_samples) {	/* last input was overflowed */
      /* restore last overflowed samples */
      memcpy(&(recog->speech[0]), overflowed_samples, sizeof(SP16)*overflowed_samplenum);
      recog->speechlen += overflowed_samplenum;
      free(overflowed_samples);
      overflowed_samples = NULL;
    }
  }
  if (recog->speechlen + len > MAXSPEECHLEN) {
    jlog("WARNING: too long input (> %d samples), segmented now\n", MAXSPEECHLEN);
    /* store the overflowed samples for next segment, and end segment */
    {
      int getlen, restlen;
      getlen = MAXSPEECHLEN - recog->speechlen;
      restlen = len - getlen;
      overflowed_samples = (SP16 *)mymalloc(sizeof(SP16)*restlen);
      memcpy(overflowed_samples, &(now[getlen]), restlen * sizeof(SP16));
      overflowed_samplenum = restlen;
      memcpy(&(recog->speech[recog->speechlen]), now, getlen * sizeof(SP16));
      recog->speechlen += getlen;
    }
    return(1);			/* tell adin_go to end segment */
  }

  /* poll module command and terminate here if requested */
  if (recog->process_want_terminate) {/* TERMINATE ... force termination */
    recog->speechlen = 0;
    return(-2);
  }
  /* store now[0..len] to recog->speech[recog->speechlen] */
  memcpy(&(recog->speech[recog->speechlen]), now, len * sizeof(SP16));
  recog->speechlen += len;
  return(0);			/* tell adin_go to continue reading */
}



/* --------------------- adin check callback --------------- */
/** 
 * <JA>
 * @brief  音声入力待ち中のモジュールコマンド処理のためのコールバック関数．
 * 音声入力待ち中にクライアントモジュールから送られたコマンドを
 * 処理するためのコールバック関数．音声入力処理中に定期的に呼ばれる．
 * もしコマンドがある場合それを処理する．また，即時の認識中断（入力破棄）や
 * 入力の中止を求められている場合はそのようにステータスを音声入力処理関数に
 * 返す．
 * 
 * @return 通常時 0, 即時中断要求がある時は -2, 認識中止要求があるときは
 * -1 を返す．
 * </JA>
 * <EN>
 * @brief  callback function to process module command while input detection.
 *
 * This function will be called periodically from A/D-in function to check
 * and process commands from module client.  If some commands are found,
 * it will be processed here.  Also, if some termination or stop of recognition
 * process is requested, this function returns to the caller as so.
 * 
 * @return 0 normally, -2 when there is immediate termination request, and -1
 * if there is recognition stop command.
 * </EN>
 */
static int
callback_check_in_adin(Recog **recoglist, int recognum)
{
  Recog *recog;

  recog = recoglist[0];

  /* module: check command and terminate recording when requested */
  callback_multi_exec(CALLBACK_POLL, recoglist, recognum);
  /* With audio input via adinnet, TERMINATE command will issue terminate
     command to the adinnet client.  The client then stops recording
     immediately and return end-of-segment ack.  Then it will cause this
     process to stop recognition as normal.  So we need not to
     perform immediate termination at this callback, but just ignore the
     results in the main.c.  */
  if (recog->process_want_terminate /* TERMINATE ... force termination */
      && recog->jconf->input.speech_input != SP_ADINNET) {
    return(-2);
  }
  if (recog->process_want_reload) {
    return(-1);
  }
  return(0);
}

/*********************/
/* open input stream */
/*********************/
/* 0 on success, -1 on error, -2 on end of input recognition */
int
j_open_stream(Recog *recog, char *file_or_dev_name)
{
  Jconf *jconf;

  jconf = recog->jconf;

  if (jconf->input.speech_input == SP_MFCFILE) {
    /* read parameter file */
    param_init_content(recog->param);
    if (rdparam(file_or_dev_name, recog->param) == FALSE) {
      jlog("ERROR: error in reading parameter file: %s\n", file_or_dev_name);
      return -1;
    }
    /* check and strip invalid frames */
    if (jconf->frontend.strip_zero_sample) {
      param_strip_zero(recog->param);
    }

    /* output frame length */
    callback_exec(CALLBACK_STATUS_PARAM, recog);
  } else {			/* raw speech input */
    /* begin A/D input */
    if (adin_begin(recog->adin) == FALSE) {
      return -2;
    }
  }
    
#if 0
    /* if not module mode, process becomes online after all initialize done */
    process_online = TRUE;
    callback_exec(CALLBACK_EVENT_PROCESS_ONLINE, recog);
#endif

  return 0;

}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

/* 1 on stop by callback (stream still continues) */
/* -1 on error, 0 on end of stream */
static int
j_recognize_stream_main(Recog **recoglist, int recognum)
{
  Jconf *jconf;
  int ret;
  float seclen, mseclen;
  Recog *recog;
  int ir;
  boolean ok_p;

  /* assume [0] is primary */
  recog = recoglist[0];

  jconf = recog->jconf;

  if (jconf->input.speech_input != SP_MFCFILE) {
    param_init_content(recog->param);
  }

  /* make the current status to be active */
  j_request_resume(recog);

  /******************************************************************/
  /* do recognition for each incoming segment from the input stream */
  /******************************************************************/
  while (1) {
    
  start_recog:

    /* make sure that all instances share ad-in and controll related data */
    for(ir=0;ir<recognum;ir++) {
      recoglist[ir]->param = recog->param;
    }
    
    /*****************************/
    /* module command processing */
    /*****************************/
    /* If recognition is running (active), commands are polled only once
       here, and if any, process the command, and continue the recognition.
       If recognition is sleeping (inactive), wait here for any command to
       come, and process them until recognition is activated by the
       commands
    */
    /* Output process status when status change occured by module command */
    if (recog->process_online != recog->process_active) {
      recog->process_online = recog->process_active;
      if (recog->process_online) callback_multi_exec(CALLBACK_EVENT_PROCESS_ONLINE, recoglist, recognum);
      else callback_multi_exec(CALLBACK_EVENT_PROCESS_OFFLINE, recoglist, recognum);
    }
    if (recog->process_active) {
      /* process is now active: check a command in buffer and process if any */
      callback_multi_exec(CALLBACK_POLL, recoglist, recognum);
    }
    j_reset_reload(recog);	/* reset reload flag here */

    if (!recog->process_active) {
      /* now sleeping, return */
      /* in the next call, we will resume from here */
      return 1;
    }
    /* update process status */
    if (recog->process_online != recog->process_active) {
      recog->process_online = recog->process_active;
      if (recog->process_online) callback_multi_exec(CALLBACK_EVENT_PROCESS_ONLINE, recoglist, recognum);
      else callback_multi_exec(CALLBACK_EVENT_PROCESS_OFFLINE, recoglist, recognum);
    }
    for(ir=0;ir<recognum;ir++) {
      if (recoglist[ir]->lmtype == LM_DFA) {
	/*********************************************************/
	/* check for grammar to change, and rebuild if necessary */
	/*********************************************************/
	multigram_exec(recoglist[ir]); /* some modification occured if return TRUE*/
      }
    }
    for(ir=0;ir<recognum;ir++) {
      if (recoglist[ir]->lmtype == LM_DFA) {
	if (recoglist[ir]->model->winfo == NULL ||
	    (recoglist[ir]->lmvar == LM_DFA_GRAMMAR && recoglist[ir]->model->dfa == NULL)) {
	  /* stop when no grammar found */
	  j_request_pause(recog);
	  goto start_recog;
	}
      }
    }

    if (jconf->input.speech_input == SP_MFCFILE) {
      /************************/
      /* parameter file input */
      /************************/
      /********************************/
      /* check the analized parameter */
      /********************************/
      /* parameter type check --- compare the type to that of HMM,
	 and adjust them if necessary */
      if (jconf->analysis.paramtype_check_flag) {
	/* return param itself or new malloced param */
	if (param_check_and_adjust(recog->model->hmminfo, recog->param, verbose_flag) == -1) {	/* failed */
	  param_init_content(recog->param);
	  /* tell failure */
	  for(ir=0;ir<recognum;ir++) recoglist[ir]->result.status = -1;
	  callback_multi_exec(CALLBACK_RESULT, recoglist, recognum);
	  goto end_recog;
	}
      }
      /* whole input is already read, so set input status to end of stream */
      /* and jump to the start point of 1st pass */
      ret = 0;
    } else {
      /****************************************************/
      /* raw wave data input (mic, file, adinnet, etc...) */
      /****************************************************/
      if (jconf->search.pass1.realtime_flag) {
	/********************************************/
	/* REALTIME ON-THE-FLY DECODING OF 1ST-PASS */
	/********************************************/
	/* store, analysis and search in a pipeline  */
	/* main function is RealTimePipeLine() at realtime-1stpass.c, and
	   it will be periodically called for each incoming input segment
	   from the AD-in function adin_go().  RealTimePipeLine() will be
	   called as a callback function from adin_go() */
	/* after this part, directly jump to the beginning of the 2nd pass */
#ifdef SP_BREAK_CURRENT_FRAME
	//	if (recog->rest_param) {
	if (recog->process_segment) {
	  /*****************************************************************/
	  /* short-pause segmentation: process last remaining frames first */
	  /*****************************************************************/
	  /* last was segmented by short pause */
	  /* the margin segment in the last input should be re-processed first */
	  callback_multi_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recoglist, recognum);
	  /* process the last remaining parameters */
	  ret = RealTimeResume(recoglist, recognum);
	  if (ret < 0) {		/* error end in the margin */
	    jlog("ERROR: failed to process last remaining samples on RealTimeResume\n"); /* exit now! */
	    return -1;
	  }
	  if (ret != 1) {	/* if segmented again in the margin, not process the rest */
	    /* last parameters has been processed, so continue with the
	       current input as normal */
	    /* output listening start message */
	    //callback_exec(CALLBACK_EVENT_SPEECH_READY, recog);
	    /* process the incoming input */
	    ret = adin_go(RealTimePipeLine, callback_check_in_adin, recoglist, recognum);
	    if (ret < 0) {		/* error end in adin_go */
	      if (ret == -2 || recog->process_want_terminate) {
		/* terminated by callback */
		RealTimeTerminate(recoglist, recognum);
		param_init_content(recog->param);
		if (ret == -2) {
		  /* output fail */
		  for(ir=0;ir<recognum;ir++) recoglist[ir]->result.status = -1;
		  callback_multi_exec(CALLBACK_RESULT, recoglist, recognum);
		}
		goto end_recog; /* cancel this recognition */
	      }
	      jlog("ERROR: an error occured at on-the-fly 1st pass decoding\n");          /* exit now! */
	      return(-1);
	    }
	  }
	  
	} else {
	  /* last was not segmented, process the incoming input  */
#endif
	  /**********************************/
	  /* process incoming speech stream */
	  /**********************************/
	  /* end of this input will be determined by either end of stream
	     (in case of file input), or silence detection by adin_go(), or
	     'TERMINATE' command from module (if module mode) */
	  /* prepare work area for on-the-fly processing */
	  if (RealTimePipeLinePrepare(recoglist, recognum) == FALSE) {
	    jlog("ERROR: failed to prepare for on-the-fly 1st pass decoding");
	    return (-1);
	  }
	  /* output 'listening start' message */
	  callback_multi_exec(CALLBACK_EVENT_SPEECH_READY, recoglist, recognum);
	  /* process the incoming input */
	  ret = adin_go(RealTimePipeLine, callback_check_in_adin, recoglist, recognum);
	  if (ret < 0) {		/* error end in adin_go */
	    if (ret == -2 || recog->process_want_terminate) {	
	    /* terminated by callback */
	      RealTimeTerminate(recoglist, recognum);
	      param_init_content(recog->param);
	      if (ret == -2) {
		/* output fail */
		for(ir=0;ir<recognum;ir++) recoglist[ir]->result.status = -1;
		callback_multi_exec(CALLBACK_RESULT, recoglist, recognum);
	      }
	      goto end_recog;
	    }
	    jlog("ERROR: an error occured at on-the-fly 1st pass decoding\n");          /* exit now! */
	    return(-1);
	  }
#ifdef SP_BREAK_CURRENT_FRAME
	}
#endif
	/******************************************************************/
	/* speech stream has been processed on-the-fly, and 1st pass ends */
	/******************************************************************/
	/* last procedure of 1st-pass */
	if (RealTimeParam(recoglist, recognum) == FALSE) {
	  jlog("ERROR: fatal error occured, program terminates now\n");
	  return -1;
	}
	/* output frame length */
	callback_multi_exec(CALLBACK_STATUS_PARAM, recoglist, recognum);
	/* if terminate signal has been received, discard this input */
	if (recog->process_want_terminate) goto end_recog;

	/* end of 1st pass, jump to 2nd pass */
	goto end_1pass;
	
      } /* end of realtime_flag && speech stream input */
      
	/******************************************/
	/* buffered speech input (not on-the-fly) */
	/******************************************/
#ifdef SP_BREAK_CURRENT_FRAME
      //if (recog->rest_param == NULL) { /* no segment left */
      if (!recog->process_segment) { /* no segment left */
#endif
	/****************************************/
	/* store raw speech samples to speech[] */
	/****************************************/
	recog->speechlen = 0;
	param_init_content(recog->param);
	/* output 'listening start' message */
	callback_multi_exec(CALLBACK_EVENT_SPEECH_READY, recoglist, recognum);
	/* tell module to start recording */
	/* the "adin_cut_callback_store_buffer" simply stores
	   the input speech to a buffer "speech[]" */
	/* end of this input will be determined by either end of stream
	   (in case of file input), or silence detection by adin_go(), or
	   'TERMINATE' command from module (if module mode) */
	ret = adin_go(adin_cut_callback_store_buffer, callback_check_in_adin, recoglist, recognum);
	if (ret < 0) {		/* error end in adin_go */
	  if (ret == -2 || recog->process_want_terminate) {
	    /* terminated by module */
	    if (ret == -2) {
	      /* output fail */
	      for(ir=0;ir<recognum;ir++) recoglist[ir]->result.status = -1;
	      callback_multi_exec(CALLBACK_RESULT, recoglist, recognum);
	    }
	    goto end_recog;
	  }
	  jlog("ERROR: an error occured while recording input\n");
	  return -1;
	}
	
	/* output recorded length */
	seclen = (float)recog->speechlen / (float)jconf->analysis.para.smp_freq;
	jlog("STAT: %d samples (%.2f sec.)\n", recog->speechlen, seclen);
	
	/* -rejectshort 指定時, 入力が指定時間以下であれば
	   ここで入力を棄却する */
	/* when using "-rejectshort", and input was shorter than
	   specified, reject the input here */
	if (jconf->reject.rejectshortlen > 0) {
	  if (seclen * 1000.0 < jconf->reject.rejectshortlen) {
	    for(ir=0;ir<recognum;ir++) recoglist[ir]->result.status = -2;
	    callback_multi_exec(CALLBACK_RESULT, recoglist, recognum);
	    goto end_recog;
	  }
	}
	
	/**********************************************/
	/* acoustic analysis and encoding of speech[] */
	/**********************************************/
	jlog("STAT: ### speech analysis (waveform -> MFCC)\n");
	/* CMN will be computed for the whole buffered input */
	if (wav2mfcc(recog->speech, recog->speechlen, recog) == FALSE) {
	  /* error end, end stream */
	  ret = -1;
	  /* tell failure */
	  for(ir=0;ir<recognum;ir++) recoglist[ir]->result.status = -1;
	  callback_multi_exec(CALLBACK_RESULT, recoglist, recognum);

	  goto end_recog;
	}
	
	/* if terminate signal has been received, cancel this input */
	if (recog->process_want_terminate) goto end_recog;
	
	/* output frame length */
	callback_multi_exec(CALLBACK_STATUS_PARAM, recoglist, recognum);
	
#ifdef SP_BREAK_CURRENT_FRAME
      }
#endif
    }	/* end of data input */
    /* parameter has been got in 'param' */
    
    /******************************************************/
    /* 1st-pass --- backward search to compute heuristics */
    /******************************************************/
    /* (for buffered speech input and HTK parameter file input) */
    if (!jconf->search.pass1.realtime_flag) {
      /* prepare for outprob cache for each HMM state and time frame */
      for(ir=0;ir<recognum;ir++) {
	outprob_prepare(&(recoglist[ir]->hmmwrk), recog->param->samplenum);
      }
    }

    /* if terminate signal has been received, cancel this input */
    if (recog->process_want_terminate) goto end_recog;

#ifdef SP_BREAK_CURRENT_FRAME
    if (!recog->process_segment) {
      callback_multi_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recoglist, recognum);
    }
    callback_multi_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recoglist, recognum);
#else
    callback_multi_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recoglist, recognum);
#endif
    /* execute computation of left-to-right backtrellis */
    ok_p = TRUE;
    for(ir=0;ir<recognum;ir++) {
      if (get_back_trellis(recoglist[ir]) == FALSE) ok_p = FALSE;
    }
    if (! ok_p) {
      jlog("ERROR: fatal error occured, program terminates now\n");
      return -1;
    }

  end_1pass:

    /**********************************/
    /* end processing of the 1st-pass */
    /**********************************/
    /* on-the-fly 1st pass processing will join here */
    
    /* -rejectshort 指定時, 入力が指定時間以下であれば探索失敗として */
    /* 第２パスを実行せずにここで終了する */
    /* when using "-rejectshort", and input was shorter than the specified
       length, terminate search here and output recognition failure */
    if (jconf->reject.rejectshortlen > 0) {
      mseclen = (float)recog->param->samplenum * (float)jconf->analysis.para.smp_period * (float)jconf->analysis.para.frameshift / 10000.0;
      if (mseclen < jconf->reject.rejectshortlen) {
	for(ir=0;ir<recognum;ir++) recoglist[ir]->result.status = -2;
	callback_multi_exec(CALLBACK_RESULT, recoglist, recognum);
	goto end_recog;
      }
    }
    
    /* if backtrellis function returns with bad status, terminate search */
    if (recog->backmax == LOG_ZERO) {
      for(ir=0;ir<recognum;ir++) recoglist[ir]->result.status = -1;
      callback_multi_exec(CALLBACK_RESULT, recoglist, recognum);
      goto end_recog;
    }
    
    /* if terminate signal has been received, cancel this input */
    if (recog->process_want_terminate) goto end_recog;
    
    /* if GMM is specified and result are to be rejected, terminate search here */
    if (jconf->reject.gmm_reject_cmn_string != NULL) {
      ok_p = TRUE;
      for(ir=0;ir<recognum;ir++) {
	if (! gmm_valid_input(recoglist[ir])) ok_p = FALSE;
      }
      if (! ok_p) {
	for(ir=0;ir<recognum;ir++) recoglist[ir]->result.status = -3;
	callback_multi_exec(CALLBACK_RESULT, recoglist, recognum);
	goto end_recog;
      }
    }

    /* if [-1pass] is specified, terminate search here */
    if (jconf->sw.compute_only_1pass) {
      goto end_recog;
    }

    /***********************************************/
    /* 2nd-pass --- forward search with heuristics */
    /***********************************************/
#if !defined(PASS2_STRICT_IWCD) || defined(FIX_35_PASS2_STRICT_SCORE)    
    /* adjust trellis score not to contain outprob of the last frames */
    for(ir=0;ir<recognum;ir++) {
      if (!recoglist[ir]->model->hmminfo->multipath) {
	bt_discount_pescore(recoglist[ir]->wchmm, recoglist[ir]->backtrellis, recoglist[ir]->param);
      }
#ifdef LM_FIX_DOUBLE_SCORING
      if (recoglist[ir]->lmtype == LM_PROB) {
	bt_discount_lm(recoglist[ir]->backtrellis);
      }
#endif
    }
#endif
    
    /* execute stack-decoding search */
    for(ir=0;ir<recognum;ir++) {
      if (recoglist[ir]->lmtype == LM_PROB) {
	wchmm_fbs(recog->param, recoglist[ir], 0, 0);
      } else if (recoglist[ir]->lmtype == LM_DFA) {
	if (jconf->output.multigramout_flag) {
	  /* execute 2nd pass multiple times for each grammar sequencially */
	  /* to output result for each grammar */
	  MULTIGRAM *m;
	  for(m = recoglist[ir]->model->grammars; m; m = m->next) {
	    if (m->active) {
	      jlog("STAT: execute 2nd pass limiting words for gram #%d\n", m->id);
	      wchmm_fbs(recog->param, recoglist[ir], m->cate_begin, m->dfa->term_num);
	    }
	  }
	} else {
	  /* only the best among all grammar will be output */
	  wchmm_fbs(recog->param, recoglist[ir], 0, recoglist[ir]->model->dfa->term_num);
	}
      }
    }

  end_recog:
    /**********************/
    /* end of recognition */
    /**********************/

#ifdef SP_BREAK_CURRENT_FRAME
    callback_multi_exec(CALLBACK_EVENT_SEGMENT_END, recoglist, recognum);
    if (recog->rest_param == NULL) callback_multi_exec(CALLBACK_EVENT_RECOGNITION_END, recoglist, recognum);
#else
    callback_multi_exec(CALLBACK_EVENT_RECOGNITION_END, recoglist, recognum);
#endif

    /* update CMN info for next input (in case of realtime wave input) */
    if (jconf->input.speech_input != SP_MFCFILE && jconf->search.pass1.realtime_flag && recog->param->samplenum > 0) {
      RealTimeCMNUpdate(recog->param, recog);
    }
    
    jlog("\n");
    jlog_flush();

#ifdef SP_BREAK_CURRENT_FRAME
    /* param is now shrinked to hold only the processed input, and */
    /* the rests are holded in (newly allocated) "rest_param" */
    /* if this is the last segment, rest_param is NULL */
    if (recog->rest_param != NULL) {
      /* process the rest parameters in the next loop */
      recog->process_segment = TRUE;
      jlog("STAT: <<<restart the rest>>>\n\n");
      free_param(recog->param);
      recog->param = recog->rest_param;
      recog->rest_param = NULL;
    } else {
      recog->process_segment = FALSE;
      /* input has reached end of stream, terminate program */
      if (ret <= 0 && ret != -2) break;
    }
#else
    /* input has reached end of stream, terminate program */
    if (ret <= 0 && ret != -2) break;
#endif

    /* recognition continues for next (silence-aparted) segment */
      
  } /* END OF STREAM LOOP */
    
    /* input stream ended. it will happen when
       - input speech file has reached the end of file, 
       - adinnet input has received end of segment mark from client,
       - adinnet input has received end of input from client,
       - adinnet client disconnected.
    */

  if (jconf->input.speech_input != SP_MFCFILE) {
    /* close the stream */
    adin_end(recog->adin);
  }

  /* return to the opening of input stream */

  return(0);

}

/* returns -1 on error, 0 on end of stream */
/* when pause requested by any callback, recognition stops here and
   call callbacks for it */
int
j_recognize_stream_multi(Recog **recoglist, int num)
{
  int ret;
  int i;
  int n;

  do {
    
    ret = j_recognize_stream_main(recoglist, num);

    switch(ret) {
    case 1:	      /* paused by a callback (stream will continue) */
      /* call pause event callbacks */
      callback_multi_exec(CALLBACK_EVENT_PAUSE, recoglist, num);
      /* call pause functions */
      /* block until all pause functions exits */
      n = 0;
      for (i = 0; i < num; i++) {
	if (callback_exist(recoglist[i], CALLBACK_PAUSE_FUNCTION)) {
	  callback_exec(CALLBACK_PAUSE_FUNCTION, recoglist[i]);
	  n++;
	}
      }
      if (n == 0) {
	jlog("WARNING: pause requested but no pause function specified\n");
	jlog("WARNING: engine will resume now immediately\n");
      }
      /* after here, recognition will restart for the rest input */
      /* call resume event callbacks */
      callback_multi_exec(CALLBACK_EVENT_RESUME, recoglist, num);
      break;
    case 0:			/* end of stream */
      /* go on to the next input */
      break;
    case -1: 		/* error */
      jlog("ERROR: an error occured while recognition, terminate stream\n");
      return -1;
    }
  } while (ret == 1);		/* loop when paused by callback */

  return 0;
}

/* returns -1 on error, 0 on end of stream */
/* when pause requested by any callback, recognition stops here and
   call callbacks for it */
int
j_recognize_stream(Recog *recog)
{
  int ret;
  int i;

  do {
    
    ret = j_recognize_stream_main(&recog, 1);

    switch(ret) {
    case 1:	      /* paused by a callback (stream will continue) */
      /* call pause event callbacks */
      callback_exec(CALLBACK_EVENT_PAUSE, recog);
      /* call pause functions */
      /* block until all pause functions exits */
      if (! callback_exist(recog, CALLBACK_PAUSE_FUNCTION)) {
	jlog("WARNING: pause requested but no pause function specified\n");
	jlog("WARNING: engine will resume now immediately\n");
      }
      callback_exec(CALLBACK_PAUSE_FUNCTION, recog);
      /* after here, recognition will restart for the rest input */
      /* call resume event callbacks */
      callback_exec(CALLBACK_EVENT_RESUME, recog);
      break;
    case 0:			/* end of stream */
      /* go on to the next input */
      break;
    case -1: 		/* error */
      jlog("ERROR: an error occured while recognition, terminate stream\n");
      return -1;
    }
  } while (ret == 1);		/* loop when paused by callback */

  return 0;
}

