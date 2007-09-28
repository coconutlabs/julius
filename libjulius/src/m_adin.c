/**
 * @file   m_adin.c
 * @author Akinobu LEE
 * @date   Fri Mar 18 16:17:23 2005
 * 
 * <JA>
 * @brief  音声入力デバイスの初期化
 * </JA>
 * 
 * <EN>
 * @brief  Initialize audio input device
 * </EN>
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


/** 
 * Select input source and setup device-specific functions.
 * 
 * @param source [In] selection ID of input source @sa adin.h
 * 
 * @return TRUE on success, FALSE if @a source is not available.
 */
static boolean
adin_select(ADIn *a, int source)
{
  switch(source) {
  case SP_RAWFILE:
#ifdef HAVE_LIBSNDFILE
    /* libsndfile interface */
    a->ad_standby 	   = adin_sndfile_standby;
    a->ad_begin 	   = adin_sndfile_begin;
    a->ad_end 		   = adin_sndfile_end;
    a->ad_resume 	   = NULL;
    a->ad_pause 	   = NULL;
    a->ad_read 		   = adin_sndfile_read;
    a->silence_cut_default = FALSE;
    a->enable_thread 	   = FALSE;
#else  /* ~HAVE_LIBSNDFILE */
    /* built-in RAW/WAV reader */
    a->ad_standby 	   = adin_file_standby;
    a->ad_begin 	   = adin_file_begin;
    a->ad_end 		   = adin_file_end;
    a->ad_resume 	   = NULL;
    a->ad_pause 	   = NULL;
    a->ad_read 		   = adin_file_read;
    a->silence_cut_default = FALSE;
    a->enable_thread 	   = FALSE;
#endif
    break;
#ifdef USE_MIC
  case SP_MIC:
    /* microphone input */
    a->ad_standby 	   = adin_mic_standby;
    a->ad_begin 	   = adin_mic_begin;
    a->ad_end 		   = adin_mic_end;
    a->ad_resume 	   = NULL;
    a->ad_pause 	   = NULL;
    a->ad_read 		   = adin_mic_read;
    a->silence_cut_default = TRUE;
    a->enable_thread 	   = TRUE;
    break;
#endif
#ifdef USE_NETAUDIO
  case SP_NETAUDIO:
    /* DatLink/NetAudio input */
    a->ad_standby 	   = adin_netaudio_standby;
    a->ad_begin 	   = adin_netaudio_begin;
    a->ad_end 		   = adin_netaudio_end;
    a->ad_resume 	   = NULL;
    a->ad_pause 	   = NULL;
    a->ad_read 		   = adin_netaudio_read;
    a->silence_cut_default = TRUE;
    a->enable_thread 	   = TRUE;
    break;
#endif
  case SP_ADINNET:
    /* adinnet network input */
    a->ad_standby 	   = adin_tcpip_standby;
    a->ad_begin 	   = adin_tcpip_begin;
    a->ad_end 		   = adin_tcpip_end;
    a->ad_resume 	   = NULL;
    a->ad_pause 	   = NULL;
    a->ad_read 		   = adin_tcpip_read;
    a->silence_cut_default = FALSE;
    a->enable_thread 	   = FALSE;
    break;
  case SP_STDIN:
    /* standard input */
    a->ad_standby 	   = adin_stdin_standby;
    a->ad_begin 	   = adin_stdin_begin;
    a->ad_end 		   = NULL;
    a->ad_resume 	   = NULL;
    a->ad_pause 	   = NULL;
    a->ad_read 		   = adin_stdin_read;
    a->silence_cut_default = FALSE;
    a->enable_thread 	   = FALSE;
    break;
  case SP_MFCFILE:
    /* MFC_FILE is not waveform, so special handling on main routine should be done */
    break;
  default:
    return FALSE;
  }

  return TRUE;
}


/** 
 * <JA>
 * 音声入力デバイスの初期化，パラメータのセットアップ，および
 * マイク入力などスレッド入力を行うデバイスの場合は入力スレッドを開始する．
 * 
 * @param recog 
 * @param arg 
 * </JA>
 * <EN>
 * Initialize audio device, setup parameters, and start A/D-in thread
 * for threaded input device (microphone etc.)
 * 
 * @param recog 
 * @param arg 
 * </EN>
 */
static boolean
adin_setup_all(ADIn *adin, Jconf *jconf, void *arg)
{

  if (jconf->input.use_ds48to16) {
    if (jconf->input.use_ds48to16 && jconf->analysis.para.smp_freq != 16000) {
      jlog("ERROR: m_adin: in 48kHz input mode, target sampling rate should be 16k!\n");
      return FALSE;
    }
    /* setup for 1/3 down sampling */
    adin->ds = ds48to16_new();
    adin->down_sample = TRUE;
    /* set device sampling rate to 48kHz */
    if (adin_standby(adin, 48000, arg) == FALSE) { /* fail */
      jlog("ERROR: m_adin: failed to ready input device\n");
      return FALSE;
    }
  } else {
    adin->down_sample = FALSE;
    if (adin_standby(adin, jconf->analysis.para.smp_freq, arg) == FALSE) { /* fail */
      jlog("ERROR: m_adin: failed to ready input device\n");
      return FALSE;
    }
  }

  /* set parameter for recording/silence detection */
  adin_setup_param(adin, jconf);

  return TRUE;
}

/** 
 * <JA>
 * Jconf の設定に従い音声入力デバイスをセットアップする．
 * 
 * </JA>
 * <EN>
 * Initialize audio input device according to the jconf configurations.
 * 
 * </EN>
 */
boolean
adin_initialize(Recog *recog)
{
  char *arg = NULL;
  ADIn *adin;
  Jconf *jconf;

  adin = recog->adin;
  jconf = recog->jconf;

  if (jconf->input.speech_input == SP_MFCFILE) {
    return TRUE; /* no need to initialize */
  }
  
  jlog("STAT: ###### initialize input device\n");

  /* select input device: file, mic, netaudio, etc... */
  if (adin_select(adin, jconf->input.speech_input) == FALSE) {
    jlog("ERROR: m_adin: failed to select input device\n");
    return FALSE;
  }

  /* set sampling frequency and device-dependent configuration
     (argument is device-dependent) */
  switch(jconf->input.speech_input) {
  case SP_ADINNET:		/* arg: port number */
    arg = mymalloc(100);
    sprintf(arg, "%d", jconf->input.adinnet_port);
    break;
  case SP_RAWFILE:		/* arg: filename of file list (if any) */
    if (jconf->input.inputlist_filename != NULL) {
      arg = mymalloc(strlen(jconf->input.inputlist_filename)+1);
      strcpy(arg, jconf->input.inputlist_filename);
    } else {
      arg = NULL;
    }
    break;
  case SP_STDIN:
    arg = NULL;
    break;
#ifdef USE_NETAUDIO
  case SP_NETAUDIO:		/* netaudio server/port name */
    arg = mymalloc(strlen(jconf->input.netaudio_devname)+1);
    strcpy(arg, jconf->input.netaudio_devname);
    break;
#endif
  }

  if (adin_setup_all(adin, jconf, arg) == FALSE) {
    return FALSE;
  }

  if (arg != NULL) free(arg);

  return TRUE;
}

/** 
 * <JA>
 * ユーザ指定のA/D-in関数を用いて音声入力デバイスをセットアップする．
 * recog->adin にデバイス用の各種関数 (ad->*) とパラメータ 
 * (silence_cut_default, enable_thread)があらかじめ格納されていること．
 * 詳細は adin_select() を参照のこと．
 * 
 * </JA>
 * <EN>
 * Initialize audio input device using user-specified A/D-in functions.
 * The user functions and parameters (silence_cut_default and enable_thread)
 * should be defined beforehand.  See adin_select() for details.
 * 
 * </EN>
 */

boolean
adin_initialize_user(Recog *recog, void *arg)
{
  boolean ret;
  ADIn *adin;
  Jconf *jconf;

  adin = recog->adin;
  jconf = recog->jconf;

  jlog("STAT: ###### initialize input device (user defined)\n");
  /* skip adin_select() */
  ret = adin_setup_all(adin, jconf, arg);
  /* create A/D-in thread here */

  return ret;
}
