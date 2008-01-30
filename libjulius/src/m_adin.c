/**
 * @file   m_adin.c
 * 
 * <JA>
 * @brief  音声入力デバイスの初期化
 * </JA>
 * 
 * <EN>
 * @brief  Initialize audio input device
 * </EN>
 * 
 * @author Akinobu LEE
 * @date   Fri Mar 18 16:17:23 2005
 *
 * $Revision: 1.3 $
 * 
 */
/*
 * Copyright (c) 1991-2007 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2007 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#include <julius/julius.h>


/** 
 * Set up device-specific parameters and functions to AD-in work area.
 *
 * @param a [i/o] AD-in work area
 * @param source [in] input source ID @sa adin.h
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
 * 音声入力デバイスを初期化し，音入力切出用パラメータをセットアップする. 
 *
 * @param adin [in] AD-in ワークエリア
 * @param jconf [in] 全体設定パラメータ
 * @param arg [in] デバイス依存引数
 * </JA>
 * <EN>
 * Initialize audio device and set up parameters for sound detection.
 * 
 * @param adin [in] AD-in work area
 * @param jconf [in] global configuration parameters
 * @param arg [in] device-specific argument
 * </EN>
 */
static boolean
adin_setup_all(ADIn *adin, Jconf *jconf, void *arg)
{

  if (jconf->input.use_ds48to16) {
    if (jconf->input.use_ds48to16 && jconf->input.sfreq != 16000) {
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
    adin->ds = NULL;
    adin->down_sample = FALSE;
    if (adin_standby(adin, jconf->input.sfreq, arg) == FALSE) { /* fail */
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
 * 設定パラメータに従い音声入力デバイスをセットアップする. 
 *
 * @param recog [i/o] エンジンインスタンス
 * 
 * </JA>
 * <EN>
 * Set up audio input device according to the jconf configurations.
 * 
 * @param recog [i/o] engine instance
 * </EN>
 *
 * @callgraph
 * @callergraph
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
 * ユーザ指定のA/D-in関数を用いて音声入力デバイスをセットアップする. 
 * recog->adin にデバイス用の各種関数 (ad->*) とパラメータ 
 * (silence_cut_default, enable_thread)があらかじめ格納されていること. 
 * 詳細は adin_select() を参照のこと. 
 *
 * @param recog [i/o] エンジンインスタンス
 * @param arg [in] adin_initialize 用引数
 * 
 * </JA>
 * <EN>
 * Initialize audio input device using user-specified A/D-in functions.
 * The user functions and parameters (silence_cut_default and enable_thread)
 * should be defined beforehand.  See adin_select() for details.
 *
 * @param recog [i/o] engine instance
 * @param arg [in] argument for adin_initialize
 * 
 * </EN>
 * @callgraph
 * @callergraph
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
/* end of file */
