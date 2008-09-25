/**
 * @file   adin.h
 *
 * <EN>
 * @brief  Definitions for A/D-in processing and sound detection.
 *
 * This file has some definitions relating audio input processing from
 * various devices, and start/end of speech detection.
 * @sa speech.h
 * </EN>
 * <JA>
 * @brief  音声入力および振幅による音区間検出に関する定義
 *
 * このファイルには, さまざまなソースからの音声入力処理と音声区間の検出
 * に関連するいくつかの定義が含まれています．
 * @sa speech.h
 * </JA>
 *
 * @author Akinobu LEE
 * @date   Thu Feb 10 17:22:36 2005
 *
 * $Revision: 1.4 $ 
 */
/*
 * Copyright (c) 1991-2007 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2007 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#ifndef __SENT_ADIN__
#define __SENT_ADIN__

#include <sent/stddefs.h>
#include <sent/speech.h>

#if defined(HAVE_ALSA_ASOUNDLIB_H) || defined(HAVE_SYS_ASOUNDLIB_H)
#define HAS_ALSA
#endif
#ifdef __linux__
#if defined(HAVE_SYS_SOUNDCARD_H) || defined(HAVE_MACHINE_SOUNDCARD_H)
#define HAS_OSS
#endif
#endif /* __linux__ */
#ifdef HAVE_ESD_H
#define HAS_ESD
#endif

/// Speech input type
enum {
  INPUT_WAVEFORM,
  INPUT_VECTOR
};

/// Speech input source
enum {
  SP_RAWFILE,			///< Wavefile
  SP_MIC,			///< Live microphone device, or plugin
  SP_ADINNET,			///< Network client (adintool etc.)
  SP_MFCFILE,			///< HTK parameter file
  SP_NETAUDIO,			///< Live NetAudio/DatLink input
  SP_STDIN,			///< Standard input
  SP_MFCMODULE			///< parameter module
};

/// Input device
enum {
  SP_INPUT_DEFAULT,
  SP_INPUT_ALSA,
  SP_INPUT_OSS,
  SP_INPUT_ESD,
};

/// Default unit size of speech input segment in bytes
#define DEFAULT_WSTEP 1000

/**
 * @def SUPPORTED_WAVEFILE_FORMAT
 * String describing the list of supported wave file formats.
 * It depends on HAVE_LIBSNDFILE.
 */
#ifdef HAVE_LIBSNDFILE
#define SUPPORTED_WAVEFILE_FORMAT "RAW(BE),WAV,AU,SND,NIST,ADPCM and more"
#else
#define SUPPORTED_WAVEFILE_FORMAT "RAW(BE),WAV"
#endif

/**
 * Number of samples from beggining of input to be used for computing
 * the zero mean of source channel (for microphone/network input).
 * 
 */
#define ZMEANSAMPLES 48000

#define	DS_RBSIZE	512	///< Filter size
#define DS_BUFSIZE 	256	///< Work area buffer size for x[]
#define DS_BUFSIZE_Y 	512	///< Work area buffer size for y[]
/**
 * down sampling filter
 * 
 */
typedef struct {
  int decrate;			///< Sample step rate from
  int intrate;			///< Sample step rate to
  double hdn[DS_RBSIZE+1];	///< Filter coefficients
  int hdn_len;		///< Filter length
  int delay;		///< Filter start point delay length
  double x[DS_BUFSIZE];	///< Work area for down sampling
  double y[DS_BUFSIZE_Y];	///< Work area for down sampling
  double rb[DS_RBSIZE];	///< Temporal buffer for firin() and firout()
  int indx;		///< Current index of rb[]
  int bp;		///< Pointer of current input samples
  int count;		///< Current output counter
} DS_FILTER;
/**
 * down sampling data
 * 
 */
typedef struct {
  DS_FILTER *fir[3]; ///< FIR filters for 48k-to-16k down sampling
  double *buf[4]; ///< work buffer for each filter
  int buflen; ///< Length of buffer
} DS_BUFFER;

/**
 * Work area for zero-cross computation
 * 
 */
typedef struct {
  int trigger;		///< Level threshold
  int length;		///< Cycle buffer size = number of samples to hold
  int offset;		///< Static data DC offset
  int zero_cross;		///< Current zero-cross num
  int is_trig;		///< Triggering status for zero-cross comp.
  int sign;			///< Current sign of waveform
  int top;			///< Top pointer of zerocross cycle buffer
  int valid_len;		///< Filled length
  SP16 *data;		///< Temporal data buffer for zerocross output
  int *is_zc;		///< zero-cross location
  int level;		///< Maximum absolute value of waveform signal in the zerocross buffer
} ZEROCROSS;

#define ZC_UNDEF 2			///< Undefined mark for zerocross
#define ZC_POSITIVE 1		///< Positive mark used for zerocross
#define ZC_NEGATIVE -1		///< Negative mark used for zerocross

/* adin/adin_mic_*.c */
boolean adin_mic_standby(int freq, void *arg);
boolean adin_mic_begin();
boolean adin_mic_end();
int adin_mic_read(SP16 *buf, int sampnum);
/* adin/adin_mic_linux_alsa.c */
boolean adin_alsa_standby(int freq, void *arg);
boolean adin_alsa_begin();
boolean adin_alsa_end();
int adin_alsa_read(SP16 *buf, int sampnum);
/* adin/adin_mic_linux_oss.c */
boolean adin_oss_standby(int freq, void *arg);
boolean adin_oss_begin();
boolean adin_oss_end();
int adin_oss_read(SP16 *buf, int sampnum);
/* adin/adin_esd.c */
boolean adin_esd_standby(int freq, void *arg);
boolean adin_esd_begin();
boolean adin_esd_end();
int adin_esd_read(SP16 *buf, int sampnum);
/* adin/adin_netaudio.c  and adin/adin_na.c */
boolean adin_netaudio_standby(int freq, void *arg);
boolean adin_netaudio_begin();
boolean adin_netaudio_end();
int adin_netaudio_read(SP16 *buf, int sampnum);
int NA_standby(int, char *);
void NA_start();
void NA_stop();
int NA_read(SP16 *buf, int sampnum);

/* adin/adin_file.c */
boolean adin_file_standby(int freq, void *arg);
boolean adin_file_begin();
int adin_file_read(SP16 *buf, int sampnum);
boolean adin_file_end();
boolean adin_stdin_standby(int freq, void *arg);
boolean adin_stdin_begin();
int adin_stdin_read(SP16 *buf, int sampnum);
char *adin_file_get_current_filename();

/* adin/adin_sndfile.c */
#ifdef HAVE_LIBSNDFILE
boolean adin_sndfile_standby(int freq, void *arg);
boolean adin_sndfile_begin();
int adin_sndfile_read(SP16 *buf, int sampnum);
boolean adin_sndfile_end();
char *adin_sndfile_get_current_filename();
#endif

/* adin/adin_tcpip.c */
boolean adin_tcpip_standby(int freq, void *arg);
boolean adin_tcpip_begin();
boolean adin_tcpip_end();
int adin_tcpip_read(SP16 *buf, int sampnum);
boolean adin_tcpip_send_pause();
boolean adin_tcpip_send_terminate();
boolean adin_tcpip_send_resume();

/* adin/zc-e.c */
void init_count_zc_e(ZEROCROSS *zc, int length);
void reset_count_zc_e(ZEROCROSS *zc, int c_trigger, int c_length, int c_offset);
void free_count_zc_e(ZEROCROSS *zc);
int count_zc_e(ZEROCROSS *zc, SP16 *buf,int step);
void zc_copy_buffer(ZEROCROSS *zc, SP16 *newbuf, int *len);

/* adin/zmean.c */
void zmean_reset();
void sub_zmean(SP16 *speech, int samplenum);

/* adin/ds48to16.c */
DS_BUFFER *ds48to16_new();
void ds48to16_free(DS_BUFFER *ds);
int ds48to16(SP16 *dst, SP16 *src, int srclen, int maxdstlen, DS_BUFFER *ds);

#endif /* __SENT_ADIN__ */
