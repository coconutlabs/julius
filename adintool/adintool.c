/**
 * @file   adintool.c
 * @author Akinobu LEE
 * @date   Wed Mar 23 20:43:32 2005
 * 
 * <JA>
 * @brief  音声入力を記録/分割/送受信する汎用音声入力ツール
 *
 * このツールは Julius の音声ライブラリを用いて様々な音声の入力や出力を
 * 行います．マイクデバイス・ファイル・adinnetネットワーククライアント・
 * 標準入力から音声を読み込んで(必要であれば)音声区間検出を行い，
 * その結果のデータを順次，ファイル・adinnetネットワークサーバー・標準出力
 * などへ出力します．
 *
 * 応用例として，Julius/Julian へのネットワーク入力(入力=マイク,出力=adinnet)
 * ，音声ファイルの音声区間抽出(入力=ファイル,出力=ファイル)などに
 * 利用できます．
 * </JA>
 * 
 * <EN>
 * @brief  AD-in tool to record / split / send / receive speech data
 *
 * This tool is to handle speech input and output from/to various devices
 * using libsetn library in Julius.  It reads input from either of
 * microphone, file, adinnet network client, or standard input, and
 * perform speech detection based on level and zero cross (you can disable
 * this), and output them to either of file, adinnet network server, or
 * standard output.
 *
 * For example, you can send microphone input to Julius running on other host
 * by specifying input to microphone and output to adinnet (Julius should
 * be run with "-input adinnet"), or you can long recorded speech data by
 * long silence by specifying both input and output to file and enabling
 * segment detection.
 * </EN>
 * 
 * $Revision: 1.1 $
 * 
 */
/*
 * Copyright (c) 1991-2006 Kawahara Lab., Kyoto University
 * Copyright (c) 2001-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2006 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#include <julius/juliuslib.h>
#include <signal.h>

/* input */
static int file_counter = 0;	///< num of input files (for SP_RAWFILE)
static int sfreq;		///< Temporal storage of sample rate

/* output */
enum{SPOUT_FILE, SPOUT_STDOUT, SPOUT_ADINNET}; ///< value for speech_output
static int speech_output = SPOUT_FILE; ///< output device
static int total_speechlen;	///< total samples of recorded segments
static int speechlen;		///< samples of one recorded segments
static char *filename = NULL;	///< Output file name
static int fd = -1;		///< File descriptor for output
static FILE *fp = NULL;		///< File pointer for WAV output
static int  size;		///< Output file size
static boolean stout = FALSE;	///< True if output to stdout
static boolean use_raw = FALSE;	///< Output in RAW format if TRUE
static boolean continuous_segment = TRUE; ///< enable/disable successive output
static int startid = 0;		///< output file numbering variable
static int sid = 0;		///< current file ID (for SPOUT_FILE)
static char *outpath = NULL;	///< work space for output file name formatting
static char *adinnet_serv = NULL; ///< Server name to send recorded data
static boolean writing_file = FALSE;

static boolean stop_at_next = FALSE; ///< TRUE if need to stop at next input by server command.  Will be set when PAUSE or TERMINATE command received while input.

/** 
 * <JA>ヘルプを表示して終了する</JA>
 * <EN>Print help and exit</EN>
 */
static boolean
opt_help(Jconf *jconf, char *arg[], int argnum)
{
  fprintf(stderr, "adintool --- AD-in tool to record/split/send/receive speech data\n");
  fprintf(stderr, "Usage: adintool [options] -in inputdev -out outputdev\n");
  fprintf(stderr, "inputdev: read speech data from:\n");
#ifdef USE_MIC
  fprintf(stderr, "    mic         microphone (default)\n");
#endif
#ifdef USE_NETAUDIO
  fprintf(stderr, "    netaudio    DatLink (NetAudio) server\n");
#endif
  fprintf(stderr, "    file        speech file (filename given from prompt)\n");
  fprintf(stderr, "    adinnet     from adinnet client (I'm server)\n");
  fprintf(stderr, "    stdin       standard tty input\n");
  fprintf(stderr, "outputdev: output data to:\n");
  fprintf(stderr, "    file        speech file (\"foo.0000.wav\" - \"foo.N.wav\"\n");
  fprintf(stderr, "    adinnet     to adinnet server (I'm client)\n");
  fprintf(stderr, "    stdout      standard tty output\n");
  
  fprintf(stderr, "I/O options:\n");
#ifdef USE_NETAUDIO
  fprintf(stderr, "    -NA             (netaudio) NetAudio server host:unit\n");
#endif
  fprintf(stderr, "    -server host    (adinnet-out) server hostname\n");
  fprintf(stderr, "    -port number    (adinnet-in/out) port number (%d)\n", jconf->input.adinnet_port);
  fprintf(stderr, "    -filename foo   (file-out) filename to record\n");
  fprintf(stderr, "    -startid id     (file-out) recording start id (%04d)\n", startid);

  fprintf(stderr, "Recording and Pause segmentation options:\n");
  fprintf(stderr, "  [-nosegment]          not segment input speech\n");
  fprintf(stderr, "  [-oneshot]            record only the first segment\n");
  fprintf(stderr, "  [-freq frequency]     sampling frequency in Hz    (%d)\n", jconf->analysis.para_default.smp_freq);
  fprintf(stderr, "  [-48]                 48000Hz recording with down sampling (16kHz only)\n");
  fprintf(stderr, "  [-lv unsignedshort]   silence cut level threshold (%d)\n", jconf->detect.level_thres);
  fprintf(stderr, "  [-zc zerocrossnum]    silence cut zerocross num   (%d)\n", jconf->detect.zero_cross_num);
  fprintf(stderr, "  [-headmargin msec]    head margin length          (%d)\n", jconf->detect.head_margin_msec);
  fprintf(stderr, "  [-tailmargin msec]    tail margin length          (%d)\n", jconf->detect.tail_margin_msec);
  fprintf(stderr, "  [-nostrip]            do not strip zero samples\n");
  fprintf(stderr, "  [-zmean]              remove DC by zero mean\n");
  fprintf(stderr, "  [-raw]                output in RAW format\n");
  fprintf(stderr, "\nLibrary configuration: ");
  confout_version(stderr);
  confout_audio(stderr);
  confout_process(stderr);
  fprintf(stderr, "\n");
  exit(1);			/* exit here */
  return TRUE;
}

static boolean
opt_in(Jconf *jconf, char *arg[], int argnum)
{
  switch(arg[0][0]) {
  case 'm':
#ifdef USE_MIC
    jconf->input.speech_input = SP_MIC;
#else
    fprintf(stderr,"Error: mic input not available\n");
    return FALSE;
#endif
    break;
  case 'f':
    jconf->input.speech_input = SP_RAWFILE;
    break;
  case 's':
    jconf->input.speech_input = SP_STDIN;
    break;
  case 'a':
    jconf->input.speech_input = SP_ADINNET;
    break;
  case 'n':
#ifdef USE_NETAUDIO
    jconf->input.speech_input = SP_NETAUDIO;
#else
    fprintf(stderr,"Error: netaudio input not available\n");
    return FALSE;
#endif
    break;
  default:
    fprintf(stderr,"Error: no such input device: %s\n", arg[0]);
    return FALSE;
  }
  return TRUE;
}
static boolean
opt_out(Jconf *jconf, char *arg[], int argnum)
{
  switch(arg[0][0]) {
  case 'f':
    speech_output = SPOUT_FILE;
    break;
  case 's':
    speech_output = SPOUT_STDOUT;
    break;
  case 'a':
    speech_output = SPOUT_ADINNET;
    break;
  default:
    fprintf(stderr,"Error: no such output device: %s\n", arg[0]);
    return FALSE;
  }
  return TRUE;
}
static boolean
opt_server(Jconf *jconf, char *arg[], int argnum)
{
  if (speech_output == SPOUT_ADINNET) {
    adinnet_serv = arg[0];
  } else {
    fprintf(stderr, "Warning: server [%s] should be with adinnet\n", arg[0]);
    return FALSE;
  }
  return TRUE;
}
static boolean
opt_NA(Jconf *jconf, char *arg[], int argnum)
{
#ifdef USE_NETAUDIO
  if (jconf->input.speech_input == SP_NETAUDIO) {
    jconf->input.netaudio_devname = arg[0];
  } else {
    fprintf(stderr, "Error: use \"-NA\" with \"-in netaudio\"\n");
    return FALSE;
  }
  return TRUE;
#else  /* ~USE_NETAUDIO */
  fprintf(stderr, "Error: NetAudio(DatLink) not supported\n");
  return FALSE;
#endif
}
static boolean
opt_port(Jconf *jconf, char *arg[], int argnum)
{
  jconf->input.adinnet_port = atoi(arg[0]);
  return TRUE;
}
static boolean
opt_filename(Jconf *jconf, char *arg[], int argnum)
{
  filename = arg[0];
  return TRUE;
}
static boolean
opt_startid(Jconf *jconf, char *arg[], int argnum)
{
  startid = atoi(arg[0]);
  return TRUE;
}
static boolean
opt_freq(Jconf *jconf, char *arg[], int argnum)
{
  jconf->analysis.para.smp_freq = atoi(arg[0]);
  jconf->analysis.para.smp_period = freq2period(jconf->analysis.para.smp_freq);
  return TRUE;
}
static boolean
opt_nosegment(Jconf *jconf, char *arg[], int argnum)
{
  jconf->detect.silence_cut = FALSE;
  return TRUE;
}
static boolean
opt_oneshot(Jconf *jconf, char *arg[], int argnum)
{
  continuous_segment = FALSE;
  return TRUE;
}
static boolean
opt_raw(Jconf *jconf, char *arg[], int argnum)
{
  use_raw = TRUE;
  return TRUE;
}

/** 
 * <JA>
 * 確認のため入出力設定をテキスト出力する
 * 
 * </JA>
 * <EN>
 * Output input/output configuration in text for a confirmation.
 * 
 * </EN>
 */
void
put_status(Recog *recog)
{
  Jconf *jconf = recog->jconf;

  fprintf(stderr,"----\n");
  fprintf(stderr,"Input-Source: ");
  switch(jconf->input.speech_input) {
  case SP_RAWFILE: fprintf(stderr,"Wave File (filename from stdin)\n"); break;
#ifdef USE_MIC
  case SP_MIC: fprintf(stderr,"Microphone\n"); break;
#endif
#ifdef USE_NETAUDIO
  case SP_NETAUDIO: fprintf(stderr,"NetAudio(DatLink) server on %s\n", jconf->input.netaudio_devname); break;
#endif
  case SP_STDIN: fprintf(stderr,"Standard Input\n"); break;
  case SP_ADINNET: fprintf(stderr,"adinnet client (port %d)\n", jconf->input.adinnet_port); break;
  }
  fprintf(stderr,"Segmentation: ");
  if (jconf->detect.silence_cut) {
    if (continuous_segment) {
      fprintf(stderr,"on, continuous\n");
    } else {
      fprintf(stderr,"on, only one snapshot\n");
    }
    if (recog->adin->down_sample) {
      fprintf(stderr,"  SampleRate: 48000Hz -> %d Hz\n", sfreq);
    } else {
      fprintf(stderr,"  SampleRate: %d Hz\n", sfreq);
    }
    fprintf(stderr,"       Level: %d / 32767\n", jconf->detect.level_thres);
    fprintf(stderr,"   ZeroCross: %d per sec.\n", jconf->detect.zero_cross_num);
    fprintf(stderr,"  HeadMargin: %d msec.\n", jconf->detect.head_margin_msec);
    fprintf(stderr,"  TailMargin: %d msec.\n", jconf->detect.tail_margin_msec);
  } else {
    fprintf(stderr,"OFF\n");
  }
  if (jconf->frontend.strip_zero_sample) {
    fprintf(stderr,"  ZeroFrames: drop\n");
  } else {
    fprintf(stderr,"  ZeroFrames: keep\n");
  }
  if (jconf->frontend.use_zmean) {
    fprintf(stderr,"   remove DC: on\n");
  } else {
    fprintf(stderr,"   remove DC: off\n");
  }
  fprintf(stderr,"   Recording: ");
  switch(speech_output) {
  case SPOUT_FILE:
    if (jconf->detect.silence_cut) {
      if (continuous_segment) {
	if (use_raw) {
	  fprintf(stderr,"%s.%04d.raw, %s.%04d.raw, ...\n", filename,startid, filename, startid+1);
	} else {
	  fprintf(stderr,"%s.%04d.wav, %s.%04d.wav, ...\n", filename,startid, filename, startid+1);
	}
      } else {
	fprintf(stderr,"%s\n", outpath);
      }
    } else {
      fprintf(stderr,"%s (warning: inifinite recording: be care of disk space!)\n", outpath);
    }
    break;
  case SPOUT_STDOUT:
    fprintf(stderr,"STDOUT\n");
    use_raw = TRUE;
    break;
  case SPOUT_ADINNET:
    fprintf(stderr,"(adinnet server [%s %d])\n", adinnet_serv, jconf->input.adinnet_port);
    break;
  }
  fprintf(stderr,"----\n");
}    

static char *
new_output_filename(char *filename, char *suffix)
{
  int len, slen;
  char *buf;

  len = strlen(filename);
  slen = strlen(suffix);
  if (len < slen || strcmp(&(filename[len-slen]), suffix) != 0) {
    buf = strcpy((char *)mymalloc(len+slen+1), filename);
    strcat(buf, suffix);
  } else {
    buf = strcpy((char *)mymalloc(len+1), filename);
  }
  return buf;
}


/**
 * <JA>
 * 読み込んだサンプル列を  fd もしくは fp に記録
 * するコールバック関数
 * 
 * @param now [in] 録音されたサンプル列
 * @param len [in] 長さ（サンプル数）
 * 
 * @return エラー時 -1，処理成功時 0，処理成功＋区間終端検出時 1 を返す．
 * </JA>
 * <EN>
 * Callback handler to record the sample fragments to file pointed by
 * the file descriptor "fd".
 * 
 * @param now [in] recorded fragments of speech sample
 * @param len [in] length of above in samples
 * 
 * @return -1 on device error (require caller to exit and terminate input),
 * 0 on success (allow caller to continue),
 * 1 on succeeded but segmentation detected (require caller to exit but
 * input will continue in the next call.
 * </EN>
 */
static int
adin_callback_file(SP16 *now, int len, Recog **recoglist, int recognum)
{
  int count;
  Recog *recog;

  recog = recoglist[0];

  /* erase "<<<please speak>>>" text on tty at trigger up */
  if (recog->jconf->input.speech_input == SP_MIC && speechlen == 0) {
    fprintf(stderr, "\r                    \r");
  }

  /* open files for recording at first trigger */
  if (speech_output == SPOUT_FILE && speechlen == 0) {
    if (continuous_segment) {
      if (use_raw) {
	sprintf(outpath, "%s.%04d.raw", filename, sid);
      } else {
	sprintf(outpath, "%s.%04d.wav", filename, sid);
      }
    }
    fprintf(stderr,"[%s]", outpath);
    if (access(outpath, F_OK) == 0) {
      if (access(outpath, W_OK) == 0) {
	fprintf(stderr, "(override)");
      } else {
	perror("adintool");
	return(-1);
      }
    }
    if (use_raw) {
      if ((fd = creat(outpath, 0644)) == -1) {
	perror("adintool");
	return -1;
      }
    } else {
      if ((fp = wrwav_open(outpath, sfreq)) == NULL) {
	perror("adintool");
	return -1;
      }
    }
    writing_file = TRUE;
  }

  /* write recorded sample to file */
  if (use_raw) {
    count = wrsamp(fd, now, len);
    if (count < 0) {
      perror("adinrec: cannot write");
      return -1;
    }
    if (count < len * sizeof(SP16)) {
      fprintf(stderr, "adinrec: cannot write more %d bytes\ncurrent length = %d\n", count, speechlen * sizeof(SP16));
      return -1;
    }
  } else {
    if (wrwav_data(fp, now, len) == FALSE) {
      fprintf(stderr, "adinrec: cannot write\n");
      return -1;
    }
  }
  
  /* accumulate sample num of this segment */
  speechlen += len;
  
  /* progress bar in dots */
  fprintf(stderr, ".");

  return(0);
}

/**
 * <JA>
 * 読み込んだサンプル列をソケットデスクリプタ "fd" 上のadinnetサーバに送信
 * するコールバック関数
 * 
 * @param now [in] 録音されたサンプル列
 * @param len [in] 長さ（サンプル数）
 * 
 * @return エラー時 -1，処理成功時 0，処理成功＋区間終端検出時 1 を返す．
 * </JA>
 * <EN>
 * Callback handler to record the sample fragments to adinnet server
 * pointed by the socket descriptor "fd".
 * 
 * @param now [in] recorded fragments of speech sample
 * @param len [in] length of above in samples
 * 
 * @return -1 on device error (require caller to exit and terminate input),
 * 0 on success (allow caller to continue),
 * 1 on succeeded but segmentation detected (require caller to exit but
 * input will continue in the next call.
 * </EN>
 */
static int
adin_callback_adinnet(SP16 *now, int len, Recog **recoglist, int recognum)
{
  int count;
  Recog *recog;

  recog = recoglist[0];

  /* erase "<<<please speak>>>" text on tty at trigger up */
  /* (moved from adin-cut.c) */
  if (recog->jconf->input.speech_input == SP_MIC && speechlen == 0) {
    fprintf(stderr, "\r                    \r");
  }
#ifdef WORDS_BIGENDIAN
  swap_sample_bytes(now, len);
#endif
  count = wt(fd, (char *)now, len * sizeof(SP16));
#ifdef WORDS_BIGENDIAN
  swap_sample_bytes(now, len);
#endif
  if (count < 0) perror("adintool: cannot write");
  /* accumulate sample num of this segment */
  speechlen += len;
  /* display progress in dots */
  fprintf(stderr, ".");
  return(0);
}

/**********************************************************************/
/** 
 * <JA>
 * adinnetサーバにセグメント終了信号を送信する
 * 
 * </JA>
 * <EN>
 * Send end-of-segment singal to adinnet server.
 * 
 * </EN>
 */
static void
adin_send_end_of_segment()
{
  char p;
  if (wt(fd, &p,  0) < 0) perror("adintool: cannot write");
}

/**********************************************************************/
/* reveice resume/pause command from adinnet server */
/* (for SPOUT_ADINNET only) */
/*'1' ... resume  '0' ... pause */

static int unknown_command_counter = 0;	///< Counter to detect broken connection
/** 
 * <JA>
 * 音声取り込み中にサーバからの中断/再開コマンドを受け取るための
 * コールバック関数
 * 
 * @return コマンド無しか再開コマンドで録音続行の場合 0,
 * エラー時 -2, 中断コマンドを受信して録音を中断すべきとき -1 を返す．
 * </JA>
 * <EN>
 * Callback function for A/D-in processing to check pause/resume
 * command from adinnet server.
 * 
 * @return 0 when no command or RESUME command to tell caller to
 * continue recording, -1 when received a PAUSE command and tell caller to
 * stop recording, or -2 when error.
 * </EN>
 */
static int
adinnet_check_command()
{
  fd_set rfds;
  struct timeval tv;
  int status;
  int cnt, ret;
  char com;
  
  /* check if some commands are waiting in queue */
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);
  tv.tv_sec = 0;
  tv.tv_usec = 1;
  status = select(fd+1, &rfds, NULL, NULL, &tv);
  if (status < 0) {           /* error */
    fprintf(stderr, "adintool: cannot check command from adinnet server\n");
    return -2;                        /* error return */
  }
  if (status > 0) {           /* there are some data */
    if (FD_ISSET(fd, &rfds)) {
      ret = rd(fd, &com, &cnt, 1); /* read in command */
      switch (com) {
      case '0':                       /* pause */
	fprintf(stderr, "<PAUSE>");
	stop_at_next = TRUE;	/* mark to pause at the end of this input */
	/* tell caller to stop recording */
	return -1;
      case '1':                       /* resume */
	fprintf(stderr, "<RESUME - already running>");
	/* we are already running, so just continue */
	break;
      case '2':                       /* terminate */
	fprintf(stderr, "<TERMINATE>");
	stop_at_next = TRUE;	/* mark to pause at the end of this input */
	/* tell caller to stop recording immediately */
	return -2;
	break;
      default:
	fprintf(stderr, "adintool: unknown command: %d\n", com);
	unknown_command_counter++;
	/* avoid infinite loop in that case... */
	/* this may happen when connection is terminated from server side  */
	if (unknown_command_counter > 100) {
	  fprintf(stderr, "killed by unknown command flood\n");
	  exit(1);
	}
      }
    }
  }
  return 0;			/* continue ad-in */
}

/** 
 * <JA>
 * サーバから再開コマンドを受信するまで待つ．再開コマンドを受信したら
 * 終了する．
 * 
 * @return エラー時 -1, 通常終了は 0 を返す．
 * </JA>
 * <EN>
 * Wait for resume command from server.
 * 
 * @return 0 on normal exit, or -1 on error.
 * </EN>
 */
static int
adinnet_wait_command()
{
  fd_set rfds;
  int status;
  int cnt, ret;
  char com;

  fprintf(stderr, "<<< waiting RESUME >>>");

  while(1) {
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    FD_SET(fd, &rfds);
    status = select(fd+1, &rfds, NULL, NULL, NULL); /* brock if no command */
    if (status < 0) {         /* error */
      fprintf(stderr, "adintool: cannot check command from adinnet server\n");
      return -1;                      /* error return */
    } else {                  /* there are some data */
      if (FD_ISSET(fd, &rfds)) {
	ret = rd(fd, &com, &cnt, 1);
	switch (com) {
	case '0':                       /* pause */
	  /* already paused, so just wait for next command */
	  fprintf(stderr, "<PAUSE - already paused>");
	  break;
	case '1':                       /* resume */
	  /* do resume */
	  fprintf(stderr, "<RESUME>\n");
	  return 1;		/* tell caller to restart */
	case '2':                       /* terminate */
	  /* already paused, so just wait for next command */
	  fprintf(stderr, "<TERMINATE - already paused>");
	  break;
	default:
	  fprintf(stderr, "adintool: unknown command: %d\n", com);
	  unknown_command_counter++;
	  /* avoid infinite loop in that case... */
	  /* this may happen when connection is terminated from server side  */
	  if (unknown_command_counter > 100) {
	    fprintf(stderr, "killed by unknown command flood\n");
	    exit(1);
	  }
	}
      }
    }
  }
  return 0;
} 

/* close file */
static boolean
close_files()
{
  size = sizeof(SP16) * speechlen;

  if (writing_file) {
    if (use_raw) {
      if (close(fd) != 0) {
	perror("adinrec");
	return FALSE;
      }
    } else {
      if (wrwav_close(fp) == FALSE) {
	fprintf(stderr, "adinrec: failed to close file\n");
	return FALSE;
      }
    }
    printf("%s: %d samples (%.2f sec.)\n", outpath, speechlen, (float)speechlen / (float)sfreq);
    
    writing_file = FALSE;
  }

  return TRUE;
}  

/* Interrupt signal handling */
static void
interrupt_record(int signum)
{
  fprintf(stderr, "[Interrupt]");
  if (speech_output == SPOUT_FILE) {
    /* close files */
    close_files();
  }
  /* terminate program */
  exit(1);
}


/** 
 * <JA>
 * メイン関数
 * 
 * @param argc [in] 引数列の長さ
 * @param argv [in] 引数列
 * 
 * @return 
 * </JA>エラー時 1，通常終了時 0 を返す．
 * <EN>
 * Main function.
 * 
 * @param argc [in] number of argument.
 * @param argv [in] array of arguments.
 * 
 * @return 1 on error, 0 on success.
 * </EN>
 */
int
main(int argc, char *argv[])
{
  Recog *recog;
  int ret;

  /* create instance */
  recog = j_recog_new();
  recog->jconf = j_jconf_new();

  /********************/
  /* setup parameters */
  /********************/
  /* register additional options */
  j_add_option("-in", 1, "input from", opt_in);
  j_add_option("-out", 1, "output to", opt_out);
  j_add_option("-server", 1, "hostname (-out adinnet)", opt_server);
  j_add_option("-NA", 1, "NetAudio server host:unit (-in netaudio)", opt_NA);
  j_add_option("-port", 1, "port number (-in/-out adinnet)", opt_port);
  j_add_option("-filename", 1, "(base) filename to record (-out file)", opt_filename);
  j_add_option("-startid", 1, "recording start id (-out file)", opt_startid);
  j_add_option("-freq", 1, "sampling frequency in Hz", opt_freq);
  j_add_option("-nosegment", 0, "not segment input speech, record all", opt_nosegment);
  j_add_option("-oneshot", 0, "exit after the first input", opt_oneshot);
  j_add_option("-raw", 0, "save in raw (BE) format", opt_raw);
  j_add_option("-h", 0, "display this help", opt_help);
  j_add_option("-help", 0, "display this help", opt_help);
  j_add_option("--help", 0, "display this help", opt_help);

  /* when no argument, output help and exit */
  if (argc <= 1) {
    opt_help(recog->jconf, NULL, 0);
    return 0;
  }

  /* read arguments and set parameters */
  if (j_config_load_args(recog->jconf, argc, argv) == -1) {
    fprintf(stderr, "Error reading arguments\n");
    return -1;
  }

  /* check needed arguments */
  if (speech_output == SPOUT_FILE && filename == NULL) {
    fprintf(stderr, "Error: output filename not specified\n");
    return(-1);
  }
  if (speech_output == SPOUT_ADINNET && adinnet_serv == NULL) {
    fprintf(stderr, "Error: adinnet server name for output not specified\n");
    return(-1);
  }
#ifdef USE_NETAUDIO
  if (recog->jconf->input.speech_input == SP_NETAUDIO && recog->jconf->input.netaudio_devname == NULL) {
    fprintf(stderr, "Error: NetAudio server name not specified\n");
    return(-1);
  }
#endif

  /* set Julius default parameters for unspecified acoustic parameters */
  apply_para(&(recog->jconf->analysis.para), &(recog->jconf->analysis.para_default));
  /* disable successive segmentation when no segmentation available */
  if (!recog->jconf->detect.silence_cut) continuous_segment = FALSE;
  /* store sampling rate locally */
  sfreq = recog->jconf->analysis.para.smp_freq;

  /********************/
  /* setup for output */
  /********************/
  if (speech_output == SPOUT_FILE) {
    /* allocate work area for output file name */
    if (continuous_segment) {
      outpath = (char *)mymalloc(strlen(filename) + 10);
    } else {
      if (use_raw) {
	outpath = filename;
      } else {
	outpath = new_output_filename(filename, ".wav");
      }
    }
  } else if (speech_output == SPOUT_ADINNET) {
    /* connect to adinnet server */
    fprintf(stderr, "connecting to %s:%d...", adinnet_serv, recog->jconf->input.adinnet_port);
    fd = make_connection(adinnet_serv, recog->jconf->input.adinnet_port);
    if (fd < 0) return 1;	/* on error */
    fprintf(stderr, "connected\n");
  } else if (speech_output == SPOUT_STDOUT) {
    /* output to stdout */
    fd = 1;
    fprintf(stderr,"[STDOUT]");
  }

  /**********************/
  /* interrupt handling */
  /**********************/
  if (signal(SIGINT, interrupt_record) == SIG_ERR) {
    fprintf(stderr, "Warning: signal intterupt may collapse output\n");
  }

  /***************************/
  /* initialize input device */
  /***************************/
  if (j_adin_init(recog) == FALSE) {
    fprintf(stderr, "Error in initializing adin device\n");
    return -1;
  }

  /**************************************/
  /* display input/output configuration */
  /**************************************/
  put_status(recog);

  /*******************/
  /* begin recording */
  /*******************/
  if (continuous_segment) {	/* reset parameter for successive output */
    total_speechlen = 0;
    sid = startid;
  }
  fprintf(stderr,"[start recording]\n");
  if (recog->jconf->input.speech_input == SP_RAWFILE) file_counter = 0;


  /*********************/
  /* input stream loop */
  /*********************/
  while(1) {

    /* begin A/D input of a stream */
    ret = j_open_stream(recog, NULL);
    switch(ret) {
    case 0:			/* succeeded */
      break;
    case -1:      		/* error */
      /* go on to next input */
      continue;
    case -2:			/* end of recognition process */
      switch(recog->jconf->input.speech_input) {
      case SP_RAWFILE:
	fprintf(stderr, "%d files processed\n", file_counter);
	break;
      case SP_STDIN:
	fprintf(stderr, "reached end of input on stdin\n");
	break;
      default:
	fprintf(stderr, "failed to begin input stream\n");
      }
      /* exit recording */
      break;
    }

    /*********************************/
    /* do VAD and recording */
    /*********************************/
    do {
      /* process one segment with segmentation */
      /* for incoming speech input, speech detection and segmentation are
	 performed and, adin_callback_* is called for speech output for each segment block.
      */
      /* adin_go() return when input segmented by long silence, or input stream reached to the end */
      speechlen = 0;
      stop_at_next = FALSE;
      if (recog->jconf->input.speech_input == SP_MIC) {
	fprintf(stderr, "<<< please speak >>>");
      }
      if (speech_output == SPOUT_ADINNET) {
	ret = adin_go(adin_callback_adinnet, adinnet_check_command, &recog, 1);
      } else {
	ret = adin_go(adin_callback_file, NULL, &recog, 1);
      }
      /* return value of adin_go:
	 -2: input terminated by pause command from adinnet server
	 -1: input device read error or callback process error
	 0:  paused by input stream (end of file, etc..)
	 >0: detected end of speech segment:
             by adin-cut, or by callback process
	 (or return value of ad_check (<0) (== not used in this program))
      */
      /* if PAUSE or TERMINATE command has been received while input,
	 stop_at_next is TRUE here  */
      switch(ret) {
      case -2:	     /* terminated by terminate command from server */
	fprintf(stderr, "[terminated by server]\n");
	break;
      case -1:		     /* device read error or callback error */
	fprintf(stderr, "[error]\n");
	break;
      case 0:			/* reached to end of input */
	fprintf(stderr, "[eof]\n");
	break;
      default:	  /* input segmented by silence or callback process */
	fprintf(stderr, "[segmented]\n", ret);
	break;
      }
      
      if (ret == -1) {
	/* error in input device or callback function, so terminate program here */
	return 1;
      }

      /*************************/
      /* one segment processed */
      /*************************/
      if (speech_output == SPOUT_FILE) {
	/* close output files */
	if (close_files() == FALSE) return 1;
      } else if (speech_output == SPOUT_ADINNET) {
	if (speechlen > 0) {
	  if (ret >= 0 || stop_at_next) { /* segmented by adin-cut or end of stream or server-side command */
	    /* send end-of-segment ack to client */
	    adin_send_end_of_segment();
	  }
	  /* output info */
	  printf("sent: %d samples (%.2f sec.)\n", speechlen, (float)speechlen / (float)sfreq);
	}
      }

      /*************************************/
      /* increment ID and total sample len */
      /*************************************/
      if (continuous_segment) {
	total_speechlen += speechlen;
	sid++;
      }

      /***************************************************/
      /* with adinnet server, if terminated by           */
      /* server-side PAUSE command, wait for RESUME here */
      /***************************************************/
      if (speech_output == SPOUT_ADINNET && stop_at_next) {
	if (adinnet_wait_command() < 0) {
	  /* command error: terminate program here */
	  return 1;
	}
      }
      
    } while (continuous_segment && ret > 0); /* to the next segment in this input stream */

    /***********************/
    /* end of input stream */
    /***********************/
    adin_end(recog->adin);

  } /* to the next input stream (i.e. next input file in SP_RAWFILE) */

  if (speech_output == SPOUT_FILE) {
    if (continuous_segment) {
      printf("recorded total %d samples (%.2f sec.) segmented to %s.%04d - %s.%04d files\n", total_speechlen, (float)total_speechlen / (float)sfreq, filename, 0, filename, sid-1);
    }
  }

  return 0;
}
