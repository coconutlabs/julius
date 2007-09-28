/**
 * @file   adin-cut.c
 * @author Akinobu LEE
 * @date   Sat Feb 12 13:20:53 2005
 *
 * <JA>
 * @brief  ���������ߤ���Ӳ�����ָ���
 *
 * �������ϥǥХ�������β����ǡ����μ����ߡ�����Ӳ�����֤θ��Ф�
 * �Ԥʤ��ޤ���
 *
 * ������֤θ��Фϡ�������٥����򺹿����Ѥ��ƹԤʤäƤ��ޤ���
 * �������Ҥ��Ȥˡ���٥뤷�����ͤ�ۤ��뿶���ˤĤ�����򺹿��򥫥���Ȥ���
 * ���줬���ꤷ�����ʾ�ˤʤ�С�������ֳ��ϸ��ФȤ���
 * �����ߤ򳫻Ϥ��ޤ��������������򺹿���������ʲ��ˤʤ�С�
 * �����ߤ���ߤ��ޤ����ºݤˤϴ����ڤ�Ф���Ԥʤ����ᡤ��������
 * �����������˥ޡ��������������ڤ�Ф��ޤ���
 * �ޤ�ɬ�פǤ���� DC offset ��Ĵ����Ԥʤ��ޤ���
 *
 * �����ǡ����μ����ߤ��¹Ԥ������ϲ����ν�����Ԥʤ��ޤ������Τ��ᡤ
 * ������������ǡ����Ϥ��μ�����ñ�̡�live���ϤǤϰ�����֡������ե�����
 * �ǤϥХåե��������ˤ��Ȥˡ�����������Ȥ��ƥ�����Хå��ؿ����ƤФ�ޤ���
 * ���Υ�����Хå��ؿ��Ȥ��ƥǡ�������¸����ħ����С�
 * �ʥե졼��Ʊ���Ρ�ǧ��������ʤ��ؿ�����ꤷ�Ƥ����ޤ���
 *
 * �ޥ������Ϥ� NetAudio ���Ϥʤɤ� Live ���Ϥ�ľ���ɤ߹����硤
 * ������Хå���ν������Ť����������Ϥ�®�٤��ɤ��դ��ʤ��ȡ�
 * �ǥХ����ΥХåե�����졤�������Ҥ����Ȥ����礬����ޤ���
 * ���Υ��顼���ɤ�����ˡ��⤷�¹ԴĶ��� pthread �����Ѳ�ǽ�Ǥ���С�
 * ���������ߡ�������ָ����������Τ���Ω��������åɤȤ���ư��ޤ���
 * ���ξ�硤���Υ���åɤ��ܥ���åɤȥХåե� @a speech ��𤷤ưʲ��Τ褦��
 * ��Ĵư��ޤ���
 * 
 *    - Thread 1: ���������ߡ�������ָ��Х���å�
 *        - �ǥХ������鲻���ǡ������ɤ߹��ߤʤ��鲻����ָ��Ф�Ԥʤ���
 *          ���Ф���������֤Υ���ץ�ϥХåե� @a speech ���������༡
 *          �ɲä���롥
 *        - ���Υ���åɤϵ�ư�������ܥ���åɤ�����Ω����ư���
 *          �嵭��ư���Ԥʤ�³���롥
 *    - Thread 2: ����������ǧ��������Ԥʤ��ܥ���å�
 *        - �Хåե� @a speech �������֤��Ȥ˴ƻ뤷�������ʥ���ץ뤬
 *          Thread 1 �ˤ�ä��ɲä��줿�餽�������������������λ����
 *          ʬ�Хåե���ͤ�롥
 *
 * ��������ؿ��γ��פϰʲ��ΤȤ���Ǥ���
 * Julius�Υᥤ��������ƤӽФ����ؿ��� adin_go() �Ǥ���
 * ���������ߤȶ�ָ��н��������Τ� adin_cut() �Ǥ���
 * �������ϥ����������ؤ��ϡ� adin_setup_func() ���оݤȤʤ����ϥ��ȥ꡼���
 * ���ϡ��ɤ߹��ߡ���ߤδؿ�������Ȥ��ƸƤӽФ����ȤǹԤʤ��ޤ���
 * �ޤ��ڤ�Ф������Τ���γƼ�ѥ�᡼���� adin_setup_param() �ǥ��åȤ��ޤ���
 * </JA>
 * <EN>
 * @brief  Read in speech waveform and detect speech segment
 *
 * This file contains functions to get speech waveform from an audio device
 * and detect speech segment.
 *
 * Speech detection is based on level threshold and zero cross count.
 * The number of zero cross are counted for each incoming speech fragment.
 * If the number becomes larger than specified threshold, the fragment
 * is treated as a beginning of speech input (trigger on).  If the number goes
 * below the threshold, the fragment will be treated as an
 * end of speech input (trigger off).  In actual
 * detection, margins are considered on the beginning and ending point, which
 * will be treated as head and tail silence part.  DC offset normalization
 * will be also performed if configured so.
 *
 * The triggered input speech data should be processed concurrently with the
 * detection for real-time recognition.  For this purpose, after the
 * beginning of speech input has been detected, the following triggered input
 * fragments (samples of a certain period in live input, or buffer size in
 * file input) are passed sequencially in turn to a callback function.
 * The callback function should be specified by the caller, typicaly to
 * store the recoded speech, or to process them into a frame-synchronous
 * recognition process.
 *
 * When source is a live input such as microphone, the device buffer will
 * overflow if the processing callback is slow.  In that case, some input
 * fragments may be lost.  To prevent this, the A/D-in part together with
 * speech detection will become an independent thread if @em pthread functions
 * are supported.  The A/D-in and detection thread will cooperate with
 * the original main thread through @a speech buffer, like the followings:
 *
 *    - Thread 1: A/D-in and speech detection thread
 *        - reads audio input from source device and perform speech detection.
 *          The detected fragments are immediately appended
 *          to the @a speech buffer.
 *        - will be detached after created, and run forever till the main
 *          thread dies.
 *    - Thread 2: Main thread
 *        - performs speech processing and recognition.
 *        - watches @a speech buffer, and if detect appendings of new samples
 *          by the Thread 1, proceed the processing for the appended samples
 *          and purge the finished samples from @a speech buffer.
 *
 * adin_setup_func() is used to switch audio input by specifying device-dependent
 * open/read/close functions, and should be called at first.
 * Function adin_setup_param() should be called after adin_setup_func() to
 * set various parameters for speech detection.
 * The adin_go() function is the top function that will be called from
 * outside, to perform actual input processing.  adin_cut() is
 * the main function to read audio input and detect speech segment.
 * </EN>
 *
 * @sa adin.c
 *
 * $Revision: 1.1 $
 * 
 */
/*
 * Copyright (c) 1991-2006 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2006 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#include <julius/julius.h>
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

/// Define this if you want to output a debug message for threading
#undef THREAD_DEBUG
/// Enable some fixes relating adinnet+module
#define TMP_FIX_200602		

/** 
 * Setup silence detection parameters (should be called after adin_select()).
 * If using pthread, the A/D-in and detection thread will be started at the end
 * of this function.
 * 
 * @param silence_cut [in] whether to perform silence cutting.
 *                 0=force off, 1=force on, 2=keep device-specific default
 * @param strip_zero [in] TRUE if enables stripping of zero samples 
 * @param cthres [in]  input level threshold (0-32767)
 * @param czc [in] zero-cross count threshold in a second
 * @param head_margin [in] header margin length in msec
 * @param tail_margin [in] tail margin length in msec
 * @param sample_freq [in] sampling frequency: just providing value for computing other variables
 * @param ignore_speech [in] TRUE if ignore speech input between call, while waiting recognition process
 * @param need_zeromean [in] TRUE if perform zero-mean subtraction
 */
void
adin_setup_param(ADIn *adin, Jconf *jconf)
{
  float samples_in_msec;
  int freq;

  if (jconf->detect.silence_cut < 2) {
    adin->adin_cut_on = (jconf->detect.silence_cut == 1) ? TRUE : FALSE;
  } else {
    adin->adin_cut_on = adin->silence_cut_default;
  }
  adin->strip_flag = jconf->frontend.strip_zero_sample;
  adin->thres = jconf->detect.level_thres;
  adin->ignore_speech_while_recog = TRUE;
#ifdef SP_BREAK_CURRENT_FRAME
  if (jconf->input.speech_input == SP_MIC) {
    /* does not drop speech while decoding */
    adin->ignore_speech_while_recog = FALSE;
  }
#endif
  adin->need_zmean = jconf->frontend.use_zmean;
  /* calc & set internal parameter from configuration */
  freq = jconf->analysis.para.smp_freq;
  samples_in_msec = (float) freq / (float)1000.0;
  /* cycle buffer length = head margin length */
  adin->c_length = (int)((float)jconf->detect.head_margin_msec * samples_in_msec);	/* in msec. */
  /* compute zerocross trigger count threshold in the cycle buffer */
  adin->noise_zerocross = jconf->detect.zero_cross_num * adin->c_length / freq;
  /* variables that comes from the tail margin length (in wstep) */
  adin->nc_max = (int)((float)(jconf->detect.tail_margin_msec * samples_in_msec / (float)DEFAULT_WSTEP)) + 2;
  adin->sbsize = jconf->detect.tail_margin_msec * samples_in_msec + (adin->c_length * jconf->detect.zero_cross_num / 200);
  adin->c_offset = 0;

#ifdef HAVE_PTHREAD
  adin->transfer_online = FALSE;
  adin->speech = NULL;
#endif
  adin->ds = NULL;

  /**********************/
  /* initialize buffers */
  /**********************/
  adin->buffer = (SP16 *)mymalloc(sizeof(SP16) * MAXSPEECHLEN);
  adin->cbuf = (SP16 *)mymalloc(sizeof(SP16) * adin->c_length);
  adin->swapbuf = (SP16 *)mymalloc(sizeof(SP16) * adin->sbsize);
  if (adin->down_sample) {
    adin->io_rate = 3;		/* 48 / 16 (fixed) */
    adin->buffer48 = (SP16 *)mymalloc(sizeof(SP16) * MAXSPEECHLEN * adin->io_rate);
  }
  if (adin->adin_cut_on) {
    init_count_zc_e(&(adin->zc), adin->c_length);
  }
  
  adin->need_init = TRUE;

}


/** 
 * Purge samples already processed in the temporary buffer @a buffer.
 * 
 * @param from [in] Purge samples in range [0..from-1].
 */
static void
adin_purge(ADIn *a, int from)
{
  if (from > 0 && a->current_len - from > 0) {
    memmove(a->buffer, &(a->buffer[from]), (a->current_len - from) * sizeof(SP16));
  }
  a->bp = a->current_len - from;
}

/** 
 * @brief  Main A/D-in function
 *
 * In threaded mode, this function will detach and loop forever in ad-in
 * thread, storing triggered samples in @a speech, and telling the status
 * to another process thread via @a transfer_online.
 * The process thread, called from adin_go(), polls the length of
 * @a speech and @a transfer_online, and if there are stored samples,
 * process them.
 *
 * In non-threaded mode, this function will be called directly from
 * adin_go(), and triggered samples are immediately processed within here.
 *
 * In module mode, the function argument @a ad_check should be specified
 * to poll the status of incoming command from client while recognition.
 * 
 * @return -2 when input terminated by result of the @a ad_check function,
 * -1 on error, 0 on end of stream, 1 if segmented by frontend, 2 if segmented
 * by processing callback.
 */
static int
adin_cut(
	 int (*ad_process)(SP16 *, int, Recog **, int), ///< function to process the triggered samples
	 int (*ad_check)(Recog **, int),	///< function periodically called while input processing
	 Recog **recoglist, int recognum) ///< If NULL, not execute callback
{
  ADIn *a;
  static int i;
  int ad_process_ret;
  int imax, len, cnt;
  int wstep;
  static int end_status;	/* return value */
  static boolean transfer_online_local;	/* local repository of transfer_online */
  static int zc;		/* count of zero cross */
  Recog *recog;

  recog = recoglist[0];

  a = recog->adin;

  /*
   * there are 3 buffers:
   *   temporary storage queue: buffer[]
   *   cycle buffer for zero-cross counting: (in zc_e)
   *   swap buffer for re-starting after short tail silence
   *
   * Each samples are first read to buffer[], then passed to count_zc_e()
   * to find trigger.  Samples between trigger and end of speech are 
   * passed to (*ad_process) with pointer to the first sample and its length.
   *
   */

  if (a->need_init) {
    a->bpmax = MAXSPEECHLEN;
    a->bp = 0;
    a->is_valid_data = FALSE;
    /* reset zero-cross status */
    if (a->adin_cut_on) {
      reset_count_zc_e(&(a->zc), a->thres, a->c_length, a->c_offset);
    }
    a->end_of_stream = FALSE;
    a->nc = 0;
    a->sblen = 0;
    a->need_init = FALSE;		/* for next call */
  }
      
  /****************/
  /* resume input */
  /****************/
  /* restart speech input if paused on the last call */
  if (a->ad_resume != NULL) {
    if ((*(a->ad_resume))() == FALSE)  return(-1);
  }

  /*************/
  /* main loop */
  /*************/
  for (;;) {

    /****************************/
    /* read in new speech input */
    /****************************/
    if (a->end_of_stream) {
      /* already reaches end of stream, just process the rest */
      a->current_len = a->bp;
    } else {
      /*****************************************************/
      /* get samples from input device to temporary buffer */
      /*****************************************************/
      /* buffer[0..bp] is the current remaining samples */
      /*
	mic input - samples exist in a device buffer
        tcpip input - samples exist in a socket
        file input - samples in a file
	   
	Return value is the number of read samples.
	If no data exists in the device (in case of mic input), ad_read()
	will return 0.  If reached end of stream (in case end of file or
	receive end ack from tcpip client), it will return -1.
	If error, returns -2.
      */
      if (a->down_sample) {
	/* get 48kHz samples to temporal buffer */
	cnt = (*(a->ad_read))(a->buffer48, (a->bpmax - a->bp) * a->io_rate);
      } else {
	cnt = (*(a->ad_read))(&(a->buffer[a->bp]), a->bpmax - a->bp);
      }
      if (cnt < 0) {		/* end of stream / segment or error */
	/* set the end status */
	if (cnt == -2) end_status = -1; /* end by error */
	else if (cnt == -1) end_status = 0; /* end by normal end of stream */
	/* now the input has been ended, 
	   we should not get further speech input in the next loop, 
	   instead just process the samples in the temporary buffer until
	   the entire data is processed. */
	a->end_of_stream = TRUE;		
	cnt = 0;			/* no new input */
	/* in case the first trial of ad_read() fails, exit this loop */
	if (a->bp == 0) break;
      }
      if (a->down_sample && cnt != 0) {
	/* convert to 16kHz  */
	cnt = ds48to16(&(a->buffer[a->bp]), a->buffer48, cnt, a->bpmax - a->bp, a->ds);
	if (cnt < 0) {		/* conversion error */
	  jlog("ERROR: adin_cut: error in down sampling\n");
	  end_status = -1;
	  a->end_of_stream = TRUE;
	  cnt = 0;
	  if (a->bp == 0) break;
	}
      }

      /*************************************************/
      /* execute callback here for incoming raw data stream.*/
      /* the content of buffer[bp...bp+cnt-1] or the   */
      /* length can be modified in the functions.      */
      /*************************************************/
      if (cnt > 0) {
	callback_exec_adin(CALLBACK_ADIN_CAPTURED, recog, &(a->buffer[a->bp]), cnt);
      }

      /*************************************************/
      /* some speech processing for the incoming input */
      /*************************************************/
      if (cnt > 0) {
	if (a->strip_flag) {
	  /* strip off successive zero samples */
	  len = strip_zero(&(a->buffer[a->bp]), cnt);
	  if (len != cnt) cnt = len;
	}
	if (a->need_zmean) {
	  /* remove DC offset */
	  sub_zmean(&(a->buffer[a->bp]), cnt);
	}
      }
      
      /* current len = current samples in buffer */
      a->current_len = a->bp + cnt;
    }
#ifdef THREAD_DEBUG
    if (a->end_of_stream) {
      jlog("DEBUG: stream already ended\n");
    }
    if (cnt > 0) {
      jlog("DEBUG: input: get %d samples [%d-%d]\n", a->current_len - a->bp, a->bp, a->current_len);
    }
#endif

    /**************************************************/
    /* call the periodic callback (non threaded mode) */
    /*************************************************/
    /* this function is mainly for periodic checking of incoming command
       in module mode */
    /* in threaded mode, this will be done in process thread, not here in adin thread */
    if (ad_check != NULL
#ifdef HAVE_PTHREAD
	&& !a->enable_thread
#endif
	) {
      /* if ad_check() returns value < 0, termination of speech input is required */
      if ((i = (*ad_check)(recoglist, recognum)) < 0) { /* -1: soft termination -2: hard termination */
	//	if ((i == -1 && current_len == 0) || i == -2) {
 	if (i == -2 ||
 	    (i == -1 && a->adin_cut_on && a->is_valid_data == FALSE) ||
 	    (i == -1 && !a->adin_cut_on && a->current_len == 0)) {
	  end_status = -2;	/* recognition terminated by outer function */
	  goto break_input;
	}
      }
    }

    /***********************************************************************/
    /* if no data has got but not end of stream, repeat next input samples */
    /***********************************************************************/
    if (a->current_len == 0) continue;

    /* When not adin_cut mode, all incoming data is valid.
       So is_valid_data should be set to TRUE when some input first comes
       till this input ends.  So, if some data comes, set is_valid_data to
       TRUE here. */ 
    if (!a->adin_cut_on && a->is_valid_data == FALSE && a->current_len > 0) {
      a->is_valid_data = TRUE;
#ifdef HAVE_PTHREAD
      if (!a->enable_thread) callback_multi_exec(CALLBACK_EVENT_SPEECH_START, recoglist, recognum);
#else
      callback_multi_exec(CALLBACK_EVENT_SPEECH_START, recoglist, recognum);
#endif
    }

    /******************************************************/
    /* prepare for processing samples in temporary buffer */
    /******************************************************/
    
    wstep = DEFAULT_WSTEP;	/* process unit (should be smaller than cycle buffer) */

    /* imax: total length that should be processed at one ad_read() call */
    /* if in real-time mode and not threaded, recognition process 
       will be called and executed as the ad_process() callback within
       this function.  If the recognition speed is over the real time,
       processing all the input samples at the loop below may result in the
       significant delay of getting next input, that may result in the buffer
       overflow of the device (namely a microphone device will suffer from
       this). So, in non-threaded mode, in order to avoid buffer overflow and
       input frame dropping, we will leave here by processing 
       only one segment [0..wstep], and leave the rest in the temporary buffer.
    */
#ifdef HAVE_PTHREAD
    if (a->enable_thread) imax = a->current_len; /* process whole */
    else imax = (a->current_len < wstep) ? a->current_len : wstep; /* one step */
#else
    imax = (a->current_len < wstep) ? a->current_len : wstep;	/* one step */
#endif
    
    /* wstep: unit length for the loop below */
    if (wstep > a->current_len) wstep = a->current_len;

#ifdef THREAD_DEBUG
    jlog("DEBUG: process %d samples by %d step\n", imax, wstep);
#endif

#ifdef HAVE_PTHREAD
    if (a->enable_thread) {
      /* get transfer status to local */
      pthread_mutex_lock(&(a->mutex));
      transfer_online_local = a->transfer_online;
      pthread_mutex_unlock(&(a->mutex));
    }
#endif

    /*********************************************************/
    /* start processing buffer[0..current_len] by wstep step */
    /*********************************************************/
    i = 0;
    while (i + wstep <= imax) {
      
      if (a->adin_cut_on) {

	/********************/
	/* check triggering */
	/********************/
	/* the cycle buffer in count_zc_e() holds the last
	   samples of (head_margin) miliseconds, and the zerocross
	   over the threshold level are counted within the cycle buffer */
	
	/* store the new data to cycle buffer and update the count */
	/* return zero-cross num in the cycle buffer */
	zc = count_zc_e(&(a->zc), &(a->buffer[i]), wstep);
	
	if (zc > a->noise_zerocross) { /* now triggering */
	  
	  if (a->is_valid_data == FALSE) {
	    /*****************************************************/
	    /* process off, trigger on: detect speech triggering */
	    /*****************************************************/
	    
	    a->is_valid_data = TRUE;   /* start processing */
	    a->nc = 0;
#ifdef THREAD_DEBUG
	    jlog("DEBUG: detect on\n");
#endif
#ifdef HAVE_PTHREAD
	    if (!a->enable_thread) callback_multi_exec(CALLBACK_EVENT_SPEECH_START, recoglist, recognum);
#else
	    callback_multi_exec(CALLBACK_EVENT_SPEECH_START, recoglist, recognum);
#endif
	    /****************************************/
	    /* flush samples stored in cycle buffer */
	    /****************************************/
	    /* (last (head_margin) msec samples */
	    /* if threaded mode, processing means storing them to speech[].
	       if ignore_speech_while_recog is on (default), ignore the data
	       if transfer is offline (=while processing second pass).
	       Else, datas are stored even if transfer is offline */
	    if ( ad_process != NULL
#ifdef HAVE_PTHREAD
		 && (!a->enable_thread || !a->ignore_speech_while_recog || transfer_online_local)
#endif
		 ) {
	      /* copy content of cycle buffer to cbuf */
	      zc_copy_buffer(&(a->zc), a->cbuf, &len);
	      /* Note that the last 'wstep' samples are the same as
		 the current samples 'buffer[i..i+wstep]', and
		 they will be processed later.  So, here only the samples
		 cbuf[0...len-wstep] will be processed
	      */
	      if (len - wstep > 0) {
#ifdef THREAD_DEBUG
		jlog("DEBUG: callback for buffered samples (%d bytes)\n", len - wstep);
#endif
#ifdef HAVE_PTHREAD
		if (!a->enable_thread) callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, a->cbuf, len - wstep);
#else
		callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, a->cbuf, len - wstep);
#endif
		ad_process_ret = (*ad_process)(a->cbuf, len - wstep, recoglist, recognum);
		switch(ad_process_ret) {
		case 1:		/* segmentation notification from process callback */
#ifdef HAVE_PTHREAD
		  if (a->enable_thread) {
		    /* in threaded mode, just stop transfer */
		    pthread_mutex_lock(&(a->mutex));
		    a->transfer_online = transfer_online_local = FALSE;
		    pthread_mutex_unlock(&(a->mutex));
		  } else {
		    /* in non-threaded mode, set end status and exit loop */
		    end_status = 2;
		    adin_purge(a, i);
		    goto break_input;
		  }
		  break;
#else
		  /* in non-threaded mode, set end status and exit loop */
		  end_status = 2;
		  adin_purge(a, i);
		  goto break_input;
#endif
		case -1:		/* error occured in callback */
		  /* set end status and exit loop */
		  end_status = -1;
		  goto break_input;
		}
	      }
	    }
	    
	  } else {		/* is_valid_data == TRUE */
	    /******************************************************/
	    /* process on, trigger on: we are in a speech segment */
	    /******************************************************/
	    
	    if (a->nc > 0) {
	      
	      /*************************************/
	      /* re-triggering in trailing silence */
	      /*************************************/
	      
#ifdef THREAD_DEBUG
	      jlog("DEBUG: re-triggered\n");
#endif
	      /* reset noise counter */
	      a->nc = 0;

#ifdef TMP_FIX_200602
	      if (ad_process != NULL
#ifdef HAVE_PTHREAD
		  && (!a->enable_thread || !a->ignore_speech_while_recog || transfer_online_local)
#endif
		  ) {
#endif
	      
	      /*************************************************/
	      /* process swap buffer stored while tail silence */
	      /*************************************************/
	      /* In trailing silence, the samples within the tail margin length
		 will be processed immediately, but samples after the tail
		 margin will not be processed, instead stored in swapbuf[].
		 If re-triggering occurs while in the trailing silence,
		 the swapped samples should be processed now to catch up
		 with current input
	      */
	      if (a->sblen > 0) {
#ifdef THREAD_DEBUG
		jlog("DEBUG: callback for swapped %d samples\n", a->sblen);
#endif
#ifdef HAVE_PTHREAD
		if (!a->enable_thread) callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, a->swapbuf, a->sblen);
#else
		callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, a->swapbuf, a->sblen);
#endif
		ad_process_ret = (*ad_process)(a->swapbuf, a->sblen, recoglist, recognum);
		a->sblen = 0;
		switch(ad_process_ret) {
		case 1:		/* segmentation notification from process callback */
#ifdef HAVE_PTHREAD
		  if (a->enable_thread) {
		    /* in threaded mode, just stop transfer */
		    pthread_mutex_lock(&(a->mutex));
		    a->transfer_online = transfer_online_local = FALSE;
		    pthread_mutex_unlock(&(a->mutex));
		  } else {
		    /* in non-threaded mode, set end status and exit loop */
		    end_status = 2;
		    adin_purge(a, i);
		    goto break_input;
		  }
		  break;
#else
		  /* in non-threaded mode, set end status and exit loop */
		  end_status = 2;
		  adin_purge(a, i);
		  goto break_input;
#endif
		case -1:		/* error occured in callback */
		  /* set end status and exit loop */
		  end_status = -1;
		  goto break_input;
		}
	      }
#ifdef TMP_FIX_200602
	      }
#endif
	    }
	  } 
	} else if (a->is_valid_data == TRUE) {
	  
	  /*******************************************************/
	  /* process on, trigger off: processing tailing silence */
	  /*******************************************************/
	  
#ifdef THREAD_DEBUG
	  jlog("DEBUG: TRAILING SILENCE\n");
#endif
	  if (a->nc == 0) {
	    /* start of tail silence: prepare valiables for start swapbuf[] */
	    a->rest_tail = a->sbsize - a->c_length;
	    a->sblen = 0;
#ifdef THREAD_DEBUG
	    jlog("DEBUG: start tail silence, rest_tail = %d\n", a->rest_tail);
#endif
	  }

	  /* increment noise counter */
	  a->nc++;
	}
      }	/* end of triggering handlers */
      
      
      /********************************************************************/
      /* process the current segment buffer[i...i+wstep] if process == on */
      /********************************************************************/
      
      if (a->adin_cut_on && a->is_valid_data && a->nc > 0 && a->rest_tail == 0) {
	
	/* The current trailing silence is now longer than the user-
	   specified tail margin length, so the current samples
	   should not be processed now.  But if 're-triggering'
	   occurs in the trailing silence later, they should be processed
	   then.  So we just store the overed samples in swapbuf[] and
	   not process them now */
	
#ifdef THREAD_DEBUG
	jlog("DEBUG: tail silence over, store to swap buffer (nc=%d, rest_tail=%d, sblen=%d-%d)\n", a->nc, a->rest_tail, a->sblen, a->sblen+wstep);
#endif
	if (a->sblen + wstep > a->sbsize) {
	  jlog("ERROR: adin_cut: swap buffer for re-triggering overflow\n");
	}
	memcpy(&(a->swapbuf[a->sblen]), &(a->buffer[i]), wstep * sizeof(SP16));
	a->sblen += wstep;
	
      } else {

	/* we are in a normal speech segment (nc == 0), or
	   trailing silence (shorter than tail margin length) (nc>0,rest_tail>0)
	   The current trailing silence is shorter than the user-
	   specified tail margin length, so the current samples
	   should be processed now as same as the normal speech segment */
	
#ifdef TMP_FIX_200602
	if (!a->adin_cut_on || a->is_valid_data == TRUE) {
#else
	if(
	   (!a->adin_cut_on || a->is_valid_data == TRUE)
#ifdef HAVE_PTHREAD
	   && (!a->enable_thread || !a->ignore_speech_while_recog || transfer_online_local)
#endif
	   ) {
#endif
	  if (a->nc > 0) {
	    /* if we are in a trailing silence, decrease the counter to detect
	     start of swapbuf[] above */
	    if (a->rest_tail < wstep) a->rest_tail = 0;
	    else a->rest_tail -= wstep;
#ifdef THREAD_DEBUG
	    jlog("DEBUG: %d processed, rest_tail=%d\n", wstep, a->rest_tail);
#endif
	  }
#ifdef TMP_FIX_200602
	  if (ad_process != NULL
#ifdef HAVE_PTHREAD
	      && (!a->enable_thread || !a->ignore_speech_while_recog || transfer_online_local)
#endif
	      ) {

#else
	  if ( ad_process != NULL ) {
#endif
#ifdef THREAD_DEBUG
	    jlog("DEBUG: callback for input sample [%d-%d]\n", i, i+wstep);
#endif
	    /* call external function */
#ifdef HAVE_PTHREAD
	    if (!a->enable_thread) callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, &(a->buffer[i]), wstep);
#else
	    callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, &(a->buffer[i]), wstep);
#endif
	    ad_process_ret = (*ad_process)(&(a->buffer[i]), wstep, recoglist, recognum);
	    switch(ad_process_ret) {
	    case 1:		/* segmentation notification from process callback */
#ifdef HAVE_PTHREAD
	      if (a->enable_thread) {
		/* in threaded mode, just stop transfer */
		pthread_mutex_lock(&(a->mutex));
		a->transfer_online = transfer_online_local = FALSE;
		pthread_mutex_unlock(&(a->mutex));
	      } else {
		/* in non-threaded mode, set end status and exit loop */
		adin_purge(a, i+wstep);
		end_status = 2;
		goto break_input;
	      }
	      break;
#else
	      /* in non-threaded mode, set end status and exit loop */
	      adin_purge(a, i+wstep);
	      end_status = 2;
	      goto break_input;
#endif
	    case -1:		/* error occured in callback */
	      /* set end status and exit loop */
	      end_status = -1;
	      goto break_input;
	    }
	  }
	}
      }	/* end of current segment processing */

      
      if (a->adin_cut_on && a->is_valid_data && a->nc >= a->nc_max) {
	/*************************************/
	/* process on, trailing silence over */
	/* = end of input segment            */
	/*************************************/
#ifdef THREAD_DEBUG
	jlog("DEBUG: detect off\n");
#endif
	/* end input by silence */
	a->is_valid_data = FALSE;	/* turn off processing */
	a->sblen = 0;
#ifdef HAVE_PTHREAD
	if (a->enable_thread) { /* just stop transfer */
	  pthread_mutex_lock(&(a->mutex));
	  a->transfer_online = transfer_online_local = FALSE;
	  pthread_mutex_unlock(&(a->mutex));
	} else {
	  adin_purge(a, i+wstep);
	  end_status = 1;
	  goto break_input;
	}
#else
	adin_purge(a, i+wstep);
	end_status = 1;
	goto break_input;
#endif
      }

      /*********************************************************/
      /* end of processing buffer[0..current_len] by wstep step */
      /*********************************************************/
      i += wstep;		/* increment to next wstep samples */
    }
    
    /* purge processed samples and update queue */
    adin_purge(a, i);

    /* end of input by end of stream */
    if (a->end_of_stream && a->bp == 0) break;
  }

break_input:

  /****************/
  /* pause input */
  /****************/
  /* stop speech input */
  if (a->ad_pause != NULL) {
    if ((*(a->ad_pause))() == FALSE) {
      jlog("ERROR: adin_cut: failed to pause recording\n");
      end_status = -1;
    }
  }

  /* execute callback */
#ifdef HAVE_PTHREAD
  if (!a->enable_thread) callback_multi_exec(CALLBACK_EVENT_SPEECH_STOP, recoglist, recognum);
#else
  callback_multi_exec(CALLBACK_EVENT_SPEECH_STOP, recoglist, recognum);
#endif

  if (a->end_of_stream) {			/* input already ends */
    if (a->bp == 0) {		/* rest buffer successfully flushed */
      /* reset status */
      a->need_init = TRUE;		/* bufer status shoule be reset at next call */
    }
    end_status = (a->bp) ? 1 : 0;
  }
  
  return(end_status);
}








#ifdef HAVE_PTHREAD
/***********************/
/* threading functions */
/***********************/

/*************************/
/* adin thread functions */
/*************************/

/** 
 * Callback for storing triggered samples to @a speech in A/D-in thread.
 * 
 * @param now [in] triggered fragment
 * @param len [in] length of above
 * 
 * @return always 0, to tell caller to continue recording.
 */
static int
adin_store_buffer(SP16 *now, int len, Recog **recoglist, int recognum)
{
  ADIn *a;
  Recog *recog;

  recog = recoglist[0];
  a = recog->adin;
  if (a->speechlen + len > MAXSPEECHLEN) {
    /* just mark as overflowed, and continue this thread */
    pthread_mutex_lock(&(a->mutex));
    a->adinthread_buffer_overflowed = TRUE;
    pthread_mutex_unlock(&(a->mutex));
    return(0);
  }
  pthread_mutex_lock(&(a->mutex));
  memcpy(&(a->speech[a->speechlen]), now, len * sizeof(SP16));
  a->speechlen += len;
  pthread_mutex_unlock(&(a->mutex));
#ifdef THREAD_DEBUG
  jlog("DEBUG: input: stored %d samples, total=%d\n", len, a->speechlen);
#endif

  return(0);			/* continue */
}

static Recog **recoglist_for_thread;
static int recognum_for_thread;

/** 
 * A/D-in thread main function: just call adin_cut() with storing function.
 * 
 * @param dummy [in] a dummy data, not used.
 */
static void
adin_thread_input_main(void *dummy)
{
  adin_cut(adin_store_buffer, NULL, recoglist_for_thread, recognum_for_thread);
}

/** 
 * Start new A/D-in thread, and also initialize buffer @a speech.
 * 
 */
boolean
adin_thread_create(Recog **recoglist, int recognum)
{
  pthread_t adin_thread;	///< Thread information
  ADIn *a;
  Recog *recog;
  int i;

  recog = recoglist[0];
  a = recog->adin;

  /* init storing buffer */
  a->speech = (SP16 *)mymalloc(sizeof(SP16) * MAXSPEECHLEN);
  a->speechlen = 0;

  a->transfer_online = FALSE; /* tell adin-mic thread to wait at initial */
  a->adinthread_buffer_overflowed = FALSE;

  /* make local copy of recoglist for the creating thread */
  recoglist_for_thread = (Recog **)mymalloc(sizeof(Recog *) * recognum);
  for(i=0;i<recognum;i++) recoglist_for_thread[i] = recoglist[i];
  recognum_for_thread = recognum;

  if (pthread_mutex_init(&(a->mutex), NULL) != 0) { /* error */
    jlog("ERROR: adin_thread_create: failed to initialize mutex\n");
    return FALSE;
  }
  if (pthread_create(&adin_thread, NULL, (void *)adin_thread_input_main, NULL) != 0) {
    jlog("ERROR: adin_thread_create: failed to create AD-in thread\n");
    return FALSE;
  }
  if (pthread_detach(adin_thread) != 0) { /* not join, run forever */
    jlog("ERROR: adin_thread_create: failed to detach AD-in thread\n");
    return FALSE;
  }
  jlog("STAT: AD-in thread created\n");
  return TRUE;
}

/****************************/
/* process thread functions */
/****************************/
/* used for module mode: return value: -2 = input cancellation forced by control module */

/** 
 * @brief  Main function of processing triggered samples at main thread.
 *
 * Wait for the new samples to be stored in @a speech by A/D-in thread,
 * and if found, process them.
 * 
 * @param ad_process [in] function to process the recorded fragments
 * @param ad_check [in] function to be called periodically for checking
 * incoming user command in module mode.
 * 
 * @return -2 when input terminated by result of the @a ad_check function,
 * -1 on error, 0 on end of stream, 1 if segmented by frontend, 2 if segmented
 * by processing callback.
 */
static int
adin_thread_process(int (*ad_process)(SP16 *, int, Recog **, int), int (*ad_check)(Recog **, int), Recog **recoglist, int recognum)
{
  int prev_len, nowlen;
  int ad_process_ret;
  int i;
  boolean overflowed_p;
  boolean transfer_online_local;
  boolean first_trig;
  ADIn *a;
  Recog *recog;

  recog = recoglist[0];

  a = recog->adin;

  /* reset storing buffer --- input while recognition will be ignored */
  pthread_mutex_lock(&(a->mutex));
  /*if (speechlen == 0) transfer_online = TRUE;*/ /* tell adin-mic thread to start recording */
  a->transfer_online = TRUE;
#ifdef THREAD_DEBUG
  jlog("DEBUG: process: reset, speechlen = %d, online=%d\n", a->speechlen, a->transfer_online);
#endif
  pthread_mutex_unlock(&(a->mutex));

  /* main processing loop */
  prev_len = 0;
  first_trig = TRUE;
  for(;;) {
    /* get current length (locking) */
    pthread_mutex_lock(&(a->mutex));
    nowlen = a->speechlen;
    overflowed_p = a->adinthread_buffer_overflowed;
    transfer_online_local = a->transfer_online;
    pthread_mutex_unlock(&(a->mutex));
    /* check if other input thread has overflowed */
    if (overflowed_p) {
      jlog("WARNING: adin_thread_process: too long input (> %d samples), segmented now\n", MAXSPEECHLEN);
      /* segment input here */
      pthread_mutex_lock(&(a->mutex));
      a->adinthread_buffer_overflowed = FALSE;
      a->speechlen = 0;
      a->transfer_online = transfer_online_local = FALSE;
      pthread_mutex_unlock(&(a->mutex));
      if (!first_trig) callback_multi_exec(CALLBACK_EVENT_SPEECH_STOP, recoglist, recognum);
      return(1);		/* return with segmented status */
    }
    /* callback poll */
    if (ad_check != NULL) {
      if ((i = (*(ad_check))(recoglist, recognum)) < 0) {
	if ((i == -1 && nowlen == 0) || i == -2) {
	  pthread_mutex_lock(&(a->mutex));
	  a->transfer_online = transfer_online_local = FALSE;
	  a->speechlen = 0;
	  pthread_mutex_unlock(&(a->mutex));
	  if (!first_trig) callback_multi_exec(CALLBACK_EVENT_SPEECH_STOP, recoglist, recognum);
	  return(-2);
	}
      }
    }
    if (prev_len < nowlen) {
#ifdef THREAD_DEBUG
      jlog("DEBUG: process: proceed [%d-%d]\n",prev_len, nowlen);
#endif
      /* got new sample, process */
      /* As the speech[] buffer is monotonously increase,
	 content of speech buffer [prev_len..nowlen] would not alter
	 in both threads
	 So locking is not needed while processing.
       */
      /*jlog("DEBUG: main: read %d-%d\n", prev_len, nowlen);*/
      /* call on/off callback */
      if (first_trig) {
	first_trig = FALSE;
	callback_multi_exec(CALLBACK_EVENT_SPEECH_START, recoglist, recognum);
      }
      if (ad_process != NULL) {
	callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, &(a->speech[prev_len]), nowlen - prev_len);
	ad_process_ret = (*ad_process)(&(a->speech[prev_len]), nowlen - prev_len, recoglist, recognum);
#ifdef THREAD_DEBUG
	jlog("DEBUG: ad_process_ret=%d\n", ad_process_ret);
#endif
	switch(ad_process_ret) {
	case 1:			/* segmented */
	  /* segmented by callback function */
	  /* purge processed samples and keep transfering */
	  pthread_mutex_lock(&(a->mutex));
	  if(a->speechlen > nowlen) {
	    memmove(a->buffer, &(a->buffer[nowlen]), (a->speechlen - nowlen) * sizeof(SP16));
	    a->speechlen -= nowlen;
	  } else {
	    a->speechlen = 0;
	  }
	  a->transfer_online = transfer_online_local = FALSE;
	  pthread_mutex_unlock(&(a->mutex));
	  if (!first_trig) callback_multi_exec(CALLBACK_EVENT_SPEECH_STOP, recoglist, recognum);
	  /* keep transfering */
	  return(2);		/* return with segmented status */
	case -1:		/* error */
	  pthread_mutex_lock(&(a->mutex));
	  a->transfer_online = transfer_online_local = FALSE;
	  pthread_mutex_unlock(&(a->mutex));
	  if (!first_trig) callback_multi_exec(CALLBACK_EVENT_SPEECH_STOP, recoglist, recognum);
	  return(-1);		/* return with error */
	}
      }
      prev_len = nowlen;
    } else {
      if (transfer_online_local == FALSE) {
	/* segmented by zero-cross */
	/* reset storing buffer for next input */
	pthread_mutex_lock(&(a->mutex));
	a->speechlen = 0;
	pthread_mutex_unlock(&(a->mutex));
	if (!first_trig) callback_multi_exec(CALLBACK_EVENT_SPEECH_STOP, recoglist, recognum);
        break;
      }
      usleep(50000);   /* wait = 0.05sec*/            
    }
  }

  /* as threading assumes infinite input */
  /* return value should be 1 (segmented) */
  return(1);
}
#endif /* HAVE_PTHREAD */




/**
 * @brief  Top function to start input processing
 *
 * If threading mode is enabled, this function simply enters to
 * adin_thread_process() to process triggered samples detected by
 * another running A/D-in thread.
 *
 * If threading mode is not available or disabled by either device requirement
 * or OS capability, this function simply calls adin_cut() to detect speech
 * segment from input device and process them concurrently by one process.
 * 
 * @param ad_process [in] function to process the recorded fragments
 * @param ad_check [in] function to be called periodically for checking
 * incoming user command in module mode.
 * 
 * @return the same as adin_thread_process() in threading mode, or
 * same as adin_cut() when non-threaded mode.
 */
int
adin_go(int (*ad_process)(SP16 *, int, Recog **, int), int (*ad_check)(Recog **, int), Recog **recoglist, int recognum)
{
#ifdef HAVE_PTHREAD
  if (recoglist[0]->adin->enable_thread) {
    return(adin_thread_process(ad_process, ad_check, recoglist, recognum));
  }
#endif
  return(adin_cut(ad_process, ad_check, recoglist, recognum));
}

/** 
 * Call device-specific initialization.
 * 
 * @param freq [in] sampling frequency
 * @param arg [in] device-dependent extra argument
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
adin_standby(ADIn *a, int freq, void *arg)
{
  if (a->need_zmean) zmean_reset();
  if (a->ad_standby != NULL) return(a->ad_standby(freq, arg));
  return TRUE;
}
/** 
 * Call device-specific function to begin capturing of the audio stream.
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
adin_begin(ADIn *a)
{
  if (a->need_zmean) zmean_reset();
  if (a->ad_begin != NULL) return(a->ad_begin());
  return TRUE;
}
/** 
 * Call device-specific function to end capturing of the audio stream.
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
adin_end(ADIn *a)
{
  if (a->ad_end != NULL) return(a->ad_end());
  return TRUE;
}

void
adin_free_param(Recog *recog)
{
  ADIn *a;

  a = recog->adin;

  if (a->ds) {
    ds48to16_free(a->ds);
    a->ds = NULL;
  }
  if (a->adin_cut_on) {
    free_count_zc_e(&(a->zc));
  }
  if (a->down_sample) {
    free(a->buffer48);
  }
  exit(1);
  free(a->swapbuf);
  free(a->cbuf);
  free(a->buffer);
#ifdef HAVE_PTHREAD
  if (a->speech) free(a->speech);
#endif
}
