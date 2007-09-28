/**
 * @file   result_tty.c
 * @author Akinobu Lee
 * @date   Tue Sep 06 17:18:46 2005
 * 
 * <JA>
 * @brief  認識結果を標準出力へ出力する．
 * </JA>
 * 
 * <EN>
 * @brief  Output recoginition result to standard output
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

#include "app.h"

#define CALLBACK_DEBUG

/// Grammar status to be processed in the next reload timing.
static char *hookstr[] = {"", "delete", "activate", "deactivate"};

#ifdef SP_BREAK_CURRENT_FRAME
static WORD_ID confword[MAXSEQNUM];
static int confwordnum;
#endif

#ifdef CHARACTER_CONVERSION
#define MAXBUFLEN 4096 ///< Maximum line length of a message sent from a client
static char inbuf[MAXBUFLEN];
static char outbuf[MAXBUFLEN];
void
myprintf(char *fmt, ...)
{
  va_list ap;
  int ret;
  
  va_start(ap,fmt);
  ret = vsnprintf(inbuf, MAXBUFLEN, fmt, ap);
  va_end(ap);
  if (ret > 0) {		/* success */
    printf("%s", charconv(inbuf, outbuf, MAXBUFLEN));
  }
}
#else
#define myprintf printf
#endif


/**********************************************************************/
/* process online/offline status  */

/** 
 * <JA>
 * 起動が終わったとき，あるいは中断状態から復帰したときに
 * メッセージを表示する．
 * 
 * </JA>
 * <EN>
 * Output message when processing was started or resumed.
 * 
 * </EN>
 */
static void
status_process_online(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("<ONLINE>\n");
#endif
}
/** 
 * <JA>
 * プロセスが中断状態へ移行したときにメッセージを表示する．モジュールモード
 * で呼ばれる．
 * 
 * </JA>
 * <EN>
 * Output message when the process paused by external command.
 * 
 * </EN>
 */
static void
status_process_offline(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("<OFFLINE>\n");
#endif
}

/**********************************************************************/
/* output recording status changes */

/** 
 * <JA>
 * 準備が終了して、認識可能状態（入力待ち状態）に入ったときの出力
 * 
 * </JA>
 * <EN>
 * Output when ready to recognize and start waiting speech input.
 * 
 * </EN>
 */
static void
status_recready(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("[SPEECH_READY]\n");
#endif
  if (recog->jconf->input.speech_input == SP_MIC || recog->jconf->input.speech_input == SP_NETAUDIO) {
    /* moved from adin-cut.c */
    fprintf(stderr, "<<< please speak >>>");
  }
}
/** 
 * <JA>
 * 入力の開始を検出したときの出力
 * 
 * </JA>
 * <EN>
 * Output when input starts.
 * 
 * </EN>
 */
static void
status_recstart(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("[SPEECH_START]\n");
#endif
  if (recog->jconf->input.speech_input == SP_MIC || recog->jconf->input.speech_input == SP_NETAUDIO) {
    fprintf(stderr, "\r                    \r");
  }
}
/** 
 * <JA>
 * 入力終了を検出したときの出力
 * 
 * </JA>
 * <EN>
 * Output when input ends.
 * 
 * </EN>
 */
static void
status_recend(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("[SPEECH_STOP]\n");
#endif
}
/** 
 * <JA>
 * 入力長などの入力パラメータ情報を出力．
 * 
 * @param param [in] 入力パラメータ構造体
 * </JA>
 * <EN>
 * Output input parameter status such as length.
 * 
 * @param param [in] input parameter structure
 * </EN>
 */
static void
status_param(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("<STATUS_PARAM>\n");
#endif
  if (verbose_flag) {
    put_param_info(stdout, recog->param);
  }
}

/**********************************************************************/
/* recognition begin / end */

/** 
 * <JA>
 * 音声入力が検知され認識処理を開始した時点でメッセージを表示する．
 * ショートポーズセグメンテーション時は，最初のセグメント開始時点で出力される．
 * 
 * </JA>
 * <EN>
 * Output message when recognition is just started for an incoming input.
 * When short-pause segmentation mode, this will be called for the first input.
 * segment.
 * 
 * </EN>
 */
static void
status_recognition_begin(Recog *recog, void *dummy) 
{
#ifdef CALLBACK_DEBUG
  printf("<RECOGNITION_BEGIN>\n");
#endif
#ifdef SP_BREAK_CURRENT_FRAME
  if (recog->jconf->output.progout_flag) { 
    confwordnum = 0;
  }
#endif
}

/** 
 * <JA>
 * 入力終了し認識処理が終了した時点でメッセージを表示する．
 * ショートポーズセグメンテーション時は，１入力の最後のセグメントの終了時に
 * 呼ばれる．
 * </JA>
 * <EN>
 * Output message when the whole recognition procedure was just finished for
 * an input.  When short-pause segmentation mode, this will be called
 * after all the segmentd input fragments are recognized.
 * </EN>
 */
static void
status_recognition_end(Recog *recog, void *dummy) 
{
#ifdef CALLBACK_DEBUG
  printf("<RECOGNITION_END>\n");
#endif
#ifdef SP_BREAK_CURRENT_FRAME
  if (recog->jconf->output.progout_flag) {
    if (confwordnum > 0) {
      printf("\n");
    }
  }
#endif
}

#ifdef SP_BREAK_CURRENT_FRAME
/** 
 * <JA>
 * ショートポーズセグメンテーション時に，区切られたある入力断片に対して
 * 認識を開始したときにメッセージを出力する．
 * </JA>
 * <EN>
 * Output a message when recognition was just started for a segment in a
 * input.
 * </EN>
 */
static void
status_segment_begin(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("<SEGMENT_BEGIN>\n");
#endif
  /* no output */
}

/** 
 * <JA>
 * ショートポーズセグメンテーション時に，区切られたある入力断片に対して
 * 認識を終了したときにメッセージを出力する．
 * </JA>
 * <EN>
 * Output a message when recognition was just finished for a segment in a
 * input.
 * </EN>
 */
static void
status_segment_end(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("<SEGMENT_END>\n");
#endif
}

#endif /* SP_BREAK_CURRENT_FRAME */

/**********************************************************************/
/* 1st pass output */

static int wst;			///< Number of words at previous output line
#ifdef SP_BREAK_CURRENT_FRAME
static int writelen;
#endif

/** 
 * <JA>
 * 第1パス：音声認識を開始する際の出力（音声入力開始時に呼ばれる）．
 * 
 * </JA>
 * <EN>
 * 1st pass: output when recognition begins (will be called at input start).
 * 
 * </EN>
 */
static void
status_pass1_begin(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("<PASS1_BEGIN>\n");
#endif
  if (!recog->jconf->search.pass1.realtime_flag) {
    if (recog->lmtype == LM_NGRAM) {
      VERMES("### Recognition: 1st pass (LR beam with 2-gram)\n");
    } else if (recog->lmtype == LM_DFA) {
      VERMES("### Recognition: 1st pass (LR beam with word-pair grammar)\n");
    }
  }

  wst = 0;
  
#ifdef SP_BREAK_CURRENT_FRAME
  if (recog->jconf->output.progout_flag) { 
    writelen = 0;
  }
#endif

}


/** 
 * <JA>
 * 第1パス：途中結果を出力する（第1パスの一定時間ごとに呼ばれる）
 * 
 * @param t [in] 現在の時間フレーム
 * @param seq [in] 現在の一位候補単語列
 * @param num [in] @a seq の長さ
 * @param score [in] 上記のこれまでの累積スコア
 * @param LMscore [in] 上記の最後の単語の信頼度
 * @param winfo [in] 単語辞書
 * </JA>
 * <EN>
 * 1st pass: output current result while search (called periodically while 1st pass).
 * 
 * @param t [in] current time frame
 * @param seq [in] current best word sequence at time @a t.
 * @param num [in] length of @a seq.
 * @param score [in] accumulated score of the current best sequence at @a t.
 * @param LMscore [in] confidence score of last word on the sequence
 * @param winfo [in] word dictionary
 * </EN>
 */
static void
result_pass1_current(Recog *recog, void *dummy)
{
  int i,bgn;
  int len, num;
  WORD_INFO *winfo;
  WORD_ID *seq;

#ifdef CALLBACK_DEBUG
  //  printf("<PASS1_INTERIM>\n");
#endif

  winfo = recog->model->winfo;
  seq = recog->result.pass1.word;
  num = recog->result.pass1.word_num;

  /* update output line with folding */
  printf("\r");

#ifdef SP_BREAK_CURRENT_FRAME

  if (recog->jconf->output.progout_flag) { 
    len = 0;
    for(i=0;i<confwordnum;i++) {
      myprintf("%s", winfo->woutput[confword[i]]);
      len += strlen(winfo->woutput[confword[i]]);
    }
    printf("|");
    len++;
    /* the first word of a segment is the same as last segment, so do not output */
    if (confwordnum > 0) {
      bgn = 1;
    } else {
      bgn = 0;
    }
    for(i=bgn;i<num;i++) {
      myprintf("%s", winfo->woutput[seq[i]]);
      len += strlen(winfo->woutput[seq[i]]);
    }
    printf("|");
    len++;

    fflush(stdout);
    if (writelen < len) writelen = len;

    return;
  }

#endif

  len = 0;
  if (wst == 0) {		/* first line */
    len += 11;
    printf("pass1_best:");
  }
  
  bgn = wst;			/* output only the last line */
  for (i=bgn;i<num;i++) {
    len += strlen(winfo->woutput[seq[i]]) + 1;
    if (len > FILLWIDTH) {	/* fold line */
      wst = i;
      printf("\n");
      len = 0;
    }
    myprintf(" %s",winfo->woutput[seq[i]]);
  }
  
  fflush(stdout);		/* flush */
}

static void
result_pass1_determined(Recog *recog, void *dummy)
{
  printf("determined at %d: %s\n", recog->result.num_frame, recog->model->winfo->woutput[recog->result.pass1.word[0]]);
}



/** 
 * <JA>
 * 第1パス：終了時に第1パスの結果を出力する（第1パス終了後、第2パスが
 * 始まる前に呼ばれる．認識に失敗した場合は呼ばれない）．
 * 
 * @param seq [in] 第1パスの1位候補の単語列
 * @param num [in] 上記の長さ
 * @param score [in] 1位の累積仮説スコア
 * @param LMscore [in] @a score のうち言語スコア
 * @param winfo [in] 単語辞書
 * </JA>
 * <EN>
 * 1st pass: output final result of the 1st pass (will be called just after
 * the 1st pass ends and before the 2nd pass begins, and will not if search
 * failed).
 * 
 * @param seq [in] word sequence of the best hypothesis at the 1st pass.
 * @param num [in] length of @a seq.
 * @param score [in] accumulated hypothesis score of @a seq.
 * @param LMscore [in] language score in @a score.
 * @param winfo [in] word dictionary.
 * </EN>
 */
static void
result_pass1(Recog *recog, void *dummy)
{
  int i,j;
  static char buf[MAX_HMMNAME_LEN];
  WORD_INFO *winfo;
  WORD_ID *seq;
  int num;

#ifdef CALLBACK_DEBUG
  printf("<RESULT_PASS1>\n");
#endif

  winfo = recog->model->winfo;
  seq = recog->result.pass1.word;
  num = recog->result.pass1.word_num;

  /* words */
  printf("\npass1_best:");
  for (i=0;i<num;i++) {
    myprintf(" %s",winfo->woutput[seq[i]]);
  }
  printf("\n");

  if (verbose_flag) {		/* output further info */
    /* N-gram entries */
    printf("pass1_best_wordseq:");
    for (i=0;i<num;i++) {
      myprintf(" %s",winfo->wname[seq[i]]);
    }
    printf("\n");
    /* phoneme sequence */
    printf("pass1_best_phonemeseq:");
    for (i=0;i<num;i++) {
      for (j=0;j<winfo->wlen[seq[i]];j++) {
	center_name(winfo->wseq[seq[i]][j]->name, buf);
	myprintf(" %s", buf);
      }
      if (i < num-1) printf(" |");
    }
    printf("\n");
    if (debug2_flag) {
      /* logical HMMs */
      printf("pass1_best_HMMseq_logical:");
      for (i=0;i<num;i++) {
	for (j=0;j<winfo->wlen[seq[i]];j++) {
	  myprintf(" %s", winfo->wseq[seq[i]][j]->name);
	}
	if (i < num-1) printf(" |");
      }
      printf("\n");
    }
  }
  /* score */
  printf("pass1_best_score: %f", recog->result.pass1.score);
  if (recog->lmtype == LM_NGRAM) {
    if (recog->jconf->output.separate_score_flag) {
      printf(" (AM: %f  LM: %f)", 
	       recog->result.pass1.score - recog->result.pass1.score_lm,
	       recog->result.pass1.score_lm);
    }
  }
  printf("\n");
}

#ifdef WORD_GRAPH
static void
result_pass1_graph(Recog *recog, void *dummy)
{
  WordGraph *wg;
  WORD_INFO *winfo;
  winfo = recog->model->winfo;
  int n;
  
  printf("--- begin wordgraph data pass1 ---\n");

  n = 0;
  for(wg=recog->result.wg1;wg;wg=wg->next) {
    myprintf("%d: [%d..%d] wid=%d name=\"%s\" lname=\"%s\" score=%f",
	     n, wg->lefttime, wg->righttime, wg->wid, winfo->woutput[wg->wid], winfo->wname[wg->wid], wg->fscore_head);
    printf(" lscore=%f", wg->lscore);
    printf(" AMavg=%f\n", wg->amavg);
    n++;
  }
  printf("--- end wordgraph data pass1 ---\n");
}
#endif

/** 
 * <JA>
 * 第1パス：終了時の出力（第1パスの終了時に必ず呼ばれる）
 * 
 * </JA>
 * <EN>
 * 1st pass: end of output (will be called at the end of the 1st pass).
 * 
 * </EN>
 */
static void
status_pass1_end(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("<PASS1_END>\n");
#endif
#ifdef SP_BREAK_CURRENT_FRAME
  if (recog->jconf->output.progout_flag) return;
#endif
  /* no op */
  printf("\n");
}

/**********************************************************************/
/* 2nd pass output */

/** 
 * <JA>
 * 仮説中の単語情報を出力する
 * 
 * @param hypo [in] 仮説
 * @param winfo [in] 単語辞書
 * </JA>
 * <EN>
 * Output word sequence of a hypothesis.
 * 
 * @param hypo [in] sentence hypothesis
 * @param winfo [in] word dictionary
 * </EN>
 */
static void
put_hypo_woutput(WORD_ID *seq, int n, WORD_INFO *winfo)
{
  int i;

  if (seq != NULL) {
    for (i=0;i<n;i++) {
      myprintf(" %s",winfo->woutput[seq[i]]);
    }
  }
  printf("\n");  
}

/** 
 * <JA>
 * 仮説のN-gram情報（Julianではカテゴリ番号列）を出力する．
 * 
 * @param hypo [in] 文仮説
 * @param winfo [in] 単語辞書
 * </JA>
 * <EN>
 * Output LM word sequence (N-gram entry/DFA category) of a hypothesis.
 * 
 * @param hypo [in] sentence hypothesis
 * @param winfo [in] word dictionary
 * </EN>
 */
static void
put_hypo_wname(WORD_ID *seq, int n, WORD_INFO *winfo)
{
  int i;

  if (seq != NULL) {
    for (i=0;i<n;i++) {
      myprintf(" %s",winfo->wname[seq[i]]);
    }
  }
  printf("\n");  
}

/** 
 * <JA>
 * 仮説の音素系列を出力する．
 * 
 * @param hypo [in] 文仮説
 * @param winfo [in] 単語情報
 * </JA>
 * <EN>
 * Output phoneme sequence of a hypothesis.
 * 
 * @param hypo [in] sentence hypothesis
 * @param winfo [in] word dictionary
 * </EN>
 */
static void
put_hypo_phoneme(WORD_ID *seq, int n, WORD_INFO *winfo)
{
  int i,j;
  WORD_ID w;
  static char buf[MAX_HMMNAME_LEN];

  if (seq != NULL) {
    for (i=0;i<n;i++) {
      w = seq[i];
      for (j=0;j<winfo->wlen[w];j++) {
	center_name(winfo->wseq[w][j]->name, buf);
	myprintf(" %s", buf);
      }
      if (i > 0) printf(" |");
    }
  }
  printf("\n");  
}
#ifdef CONFIDENCE_MEASURE
/** 
 * <JA>
 * 仮説の単語ごとの信頼度を出力する．
 * 
 * @param hypo [in] 文仮説
 * </JA>
 * <EN>
 * Output confidence score of words in a sentence hypothesis.
 * 
 * @param hypo 
 * </EN>
 */
#ifdef CM_MULTIPLE_ALPHA
static void
put_hypo_cmscore(NODE *hypo, int id)
{
  int i;
  int j;
  
  if (hypo != NULL) {
    for (i=hypo->seqnum-1;i>=0;i--) {
      printf(" %5.3f", hypo->cmscore[i][id]);
    }
  }
  printf("\n");  
}
#else
static void
put_hypo_cmscore(LOGPROB *cmscore, int n)
{
  int i;
  
  if (cmscore != NULL) {
    for (i=0;i<n;i++) {
      printf(" %5.3f", cmscore[i]);
    }
  }
  printf("\n");
}
#endif
#endif /* CONFIDENCE_MEASURE */

/** 
 * <JA>
 * 第2パス：得られた文仮説候補を1つ出力する．
 * 
 * @param hypo [in] 得られた文仮説
 * @param rank [in] @a hypo の順位
 * @param winfo [in] 単語辞書
 * </JA>
 * <EN>
 * 2nd pass: output a sentence hypothesis found in the 2nd pass.
 * 
 * @param hypo [in] sentence hypothesis to be output
 * @param rank [in] rank of @a hypo
 * @param winfo [in] word dictionary
 * </EN>
 */
static void
//ttyout_pass2(NODE *hypo, int rank, Recog *recog)
result_pass2(Recog *recog, void *dummy)
{
#ifdef SP_BREAK_CURRENT_FRAME
  int i, j;
  int len;
#endif
  char ec[5] = {0x1b, '[', '1', 'm', 0};
  WORD_INFO *winfo;
  WORD_ID *seq;
  int seqnum;
  int n, num;
  Sentence *s;

#ifdef CALLBACK_DEBUG
  printf("<RESULT_PASS2>\n");
#endif

#ifdef SP_BREAK_CURRENT_FRAME
  if (recog->result.status < 0 && recog->jconf->output.progout_flag) {
    //printf("\r");
    //for(i=0;i<writelen;i++) printf(" ");
    //printf("\r");
    /* output pass1 result as final */

    winfo = recog->model->winfo;
    seq = recog->result.pass1.word;
    seqnum = recog->result.pass1.word_num;

    printf("\r");
    len = 0;
#if 0
    j = seqnum - 1;
    if (confwordnum > 0) j--;	/* head word is the same as previous segment */
    for (i=0;i<j;i++) {
      confword[confwordnum++] = seq[i];
    }
#else
    j = 0;
    if (confwordnum > 0) j++;	/* head word is the same as previous segment */
    for (i=j;i<seqnum;i++) {
      confword[confwordnum++] = seq[i];
    }
#endif
    for(i=0;i<confwordnum;i++) {
      myprintf("%s", winfo->woutput[confword[i]]);
      len += strlen(winfo->woutput[confword[i]]);
    }
    for(i=len;i<writelen;i++) printf(" ");
    fflush(stdout);
    
    return;
  }
#endif

  if (recog->result.status < 0) {
    switch(recog->result.status) {
    case -3:			/* input rejected by GMM */
      printf("input rejected by GMM\n");
      break;
    case -2:
      printf("input rejected by short input\n");
      break;
    case -1:
      printf("search failed\n");
      break;
    }
    return;
  }

  winfo = recog->model->winfo;
  num = recog->result.sentnum;

  for(n=0;n<num;n++) {
    s = &(recog->result.sent[n]);
    seq = s->word;
    seqnum = s->word_num;

#ifdef SP_BREAK_CURRENT_FRAME
    if (recog->jconf->output.progout_flag) {
      printf("\r");
      len = 0;
#if 0
      j = seqnum - 1;
      if (confwordnum > 0) j--;	/* head word is the same as previous segment */
      for (i=0;i<j;i++) {
	confword[confwordnum++] = seq[i];
      }
#else
      j = 0;
      if (confwordnum > 0) j++;	/* head word is the same as previous segment */
      for (i=j;i<seqnum;i++) {
	confword[confwordnum++] = seq[i];
      }
#endif
      for(i=0;i<confwordnum;i++) {
	myprintf("%s", winfo->woutput[confword[i]]);
	len += strlen(winfo->woutput[confword[i]]);
      }
      for(i=len;i<writelen;i++) printf(" ");

      break;
    }

#endif

    if (debug2_flag) {
      printf("\n%s",ec);		/* newline & bold on */
    }
    printf("sentence%d:", n+1);
    put_hypo_woutput(seq, seqnum, winfo);
    if (verbose_flag) {
      printf("wseq%d:", n+1);
      put_hypo_wname(seq, seqnum, winfo);
      printf("phseq%d:", n+1);
      put_hypo_phoneme(seq, seqnum, winfo);
#ifdef CONFIDENCE_MEASURE
#ifdef CM_MULTIPLE_ALPHA
      {
	int i;
	for(i=0;i<recog->jconf->annotate.cm_alpha_num;i++) {
	  printf("cmscore%d[%f]:", rank, recog->jconf->annotate.cm_alpha_bgn + i * recog->jconf->annotate.cm_alpha_step);
	  put_hypo_cmscore(hypo, i);
	}
      }
#else
      printf("cmscore%d:", n+1);
      put_hypo_cmscore(s->confidence, seqnum);
#endif
#endif /* CONFIDENCE_MEASURE */
    }
    if (debug2_flag) {
      ec[2] = '0';
      printf("%s\n",ec);		/* bold off & newline */
    }
    if (verbose_flag) {
      printf("score%d: %f", n+1, s->score);
      if (recog->lmtype == LM_NGRAM) {
	if (recog->jconf->output.separate_score_flag) {
	  printf(" (AM: %f  LM: %f)", s->score_lm, s->score_lm);
	}
      }
      printf("\n");
      if (recog->lmtype == LM_DFA) {
	/* output which grammar the hypothesis belongs to on multiple grammar */
	/* determine only by the last word */
	if (multigram_get_all_num(recog) > 1) {
	  printf("grammar%d: %d\n", n+1, s->gram_id);
	}
      }
    }

    /* output alignment result if exist */
    if (s->align.filled) {
      HMM_Logical *p;
      int i;

      printf("=== begin forced alignment ===\n");
      printf(" id: from  to    n_score    unit\n");
      printf(" ----------------------------------------\n");
      for(i=0;i<s->align.num;i++) {
	printf("[%4d %4d]  %f  ", s->align.begin_frame[i], s->align.end_frame[i], s->align.avgscore[i]);
	switch(s->align.unittype) {
	case PER_WORD:
	  myprintf("%s\t[%s]\n", winfo->wname[s->align.w[i]], winfo->woutput[s->align.w[i]]);
	  break;
	case PER_PHONEME:
	  p = s->align.ph[i];
	  if (p->is_pseudo) {
	    printf("{%s}\n", p->name);
	  } else if (strmatch(p->name, p->body.defined->name)) {
	    printf("%s\n", p->name);
	  } else {
	    printf("%s[%s]\n", p->name, p->body.defined->name);
	  }
	  break;
	case PER_STATE:
	  p = s->align.ph[i];
	  if (p->is_pseudo) {
	    printf("{%s}", p->name);
	  } else if (strmatch(p->name, p->body.defined->name)) {
	    printf("%s", p->name);
	  } else {
	    printf("%s[%s]", p->name, p->body.defined->name);
	  }
	  if (recog->model->hmminfo->multipath) {
	    if (s->align.is_iwsp[i]) {
	      printf(" #%d (sp)\n", s->align.loc[i]);
	    } else {
	      printf(" #%d\n", s->align.loc[i]);
	    }
	  } else {
	    printf(" #%d\n", s->align.loc[i]);
	  }
	  break;
	}
      }

      printf("re-computed AM score: %f\n", s->align.allscore);

      printf("=== end forced alignment ===\n");
    }
  }

  fflush(stdout);

}

/** 
 * <JA>
 * 第2パス：音声認識結果の出力を開始する際の出力．認識結果を出力する際に、
 * 一番最初に出力される．
 * 
 * </JA>
 * <EN>
 * 2nd pass: output at the start of result output (will be called before
 * all the result output in the 2nd pass).
 * 
 * </EN>
 */
static void
status_pass2_begin(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("<PASS2_BEGIN>\n");
#endif

  if (recog->lmtype == LM_NGRAM) {
    VERMES("### Recognition: 2nd pass (RL heuristic best-first with 3-gram)\n");
  } else if (recog->lmtype == LM_DFA) {
    VERMES("### Recognition: 2nd pass (RL heuristic best-first with DFA)\n");
  }
  if (verbose_flag) printf("samplenum=%d\n", recog->result.num_frame);
  if (debug2_flag) VERMES("getting %d candidates...\n", recog->jconf->search.pass2.nbest);
}

/** 
 * <JA>
 * 第2パス：終了時
 * 
 * </JA>
 * <EN>
 * 2nd pass: end output
 * 
 * </EN>
 */
static void
status_pass2_end(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("<PASS2_END>\n");
#endif

#ifdef SP_BREAK_CURRENT_FRAME
  if (recog->rest_param != NULL) {
    if (verbose_flag) {
      printf("Segmented by short pause, continue to next...\n");
    } else {
      //printf("-->\n");
    }
  }
  fflush(stdout);
#endif
}

/**********************************************************************/
/* word graph output */

#define TEXTWIDTH 70

/** 
 * <JA>
 * 得られた単語グラフ全体を出力する．
 * 
 * @param root [in] グラフ単語集合の先頭要素へのポインタ
 * @param winfo [in] 単語辞書
 * </JA>
 * <EN>
 * Output the whole word graph.
 * 
 * @param root [in] pointer to the first element of graph words
 * @param winfo [in] word dictionary
 * </EN>
 */
static void
result_graph(Recog *recog, void *dummy)
{
  WordGraph *wg;
  int tw1, tw2, i;
  WORD_INFO *winfo;

#ifdef CALLBACK_DEBUG
  printf("<RESULT_GRAPH>\n");
#endif

  winfo = recog->model->winfo;

  /* debug: output all graph word info */
  wordgraph_dump(stdout, recog->result.wg, winfo);

  printf("-------------------------- begin wordgraph show -------------------------\n");
  for(wg=recog->result.wg;wg;wg=wg->next) {
    tw1 = (TEXTWIDTH * wg->lefttime) / recog->peseqlen;
    tw2 = (TEXTWIDTH * wg->righttime) / recog->peseqlen;
    printf("%4d:", wg->id);
    for(i=0;i<tw1;i++) printf(" ");
    myprintf(" %s\n", winfo->woutput[wg->wid]);
    printf("%4d:", wg->lefttime);
    for(i=0;i<tw1;i++) printf(" ");
    printf("|");
    for(i=tw1+1;i<tw2;i++) printf("-");
    printf("|\n");
  }
  printf("-------------------------- end wordgraph show ---------------------------\n");
}

/** 
 * <JA>
 * 得られたコンフュージョンネットワークを出力する．
 * 
 * </JA>
 * <EN>
 * Output the obtained confusion network.
 * 
 * </EN>
 */
static void
result_confnet(Recog *recog, void *dummy)
{
  CN_CLUSTER *c;
  int i;

#ifdef CALLBACK_DEBUG
  printf("<RESULT_CONFNET>\n");
#endif

  printf("---- begin confusion network ---\n");
  for(c=recog->result.confnet;c;c=c->next) {
    for(i=0;i<c->wordsnum;i++) {
      myprintf("(%s:%.3f)", (c->words[i] == WORD_INVALID) ? "-" : recog->model->winfo->woutput[c->words[i]], c->pp[i]);
      if (i == 0) printf("  ");
    }
    printf("\n");
#if 0
    /* output details - break down all words clustered into this class */
    for(i=0;i<c->wgnum;i++) {
      printf("    ");
      put_wordgraph(stdout, c->wg[i], recog->model->winfo);
    }
#endif
  }
  printf("---- end confusion network ---\n");
}

/********************* RESULT OUTPUT FOR GMM *************************/
/** 
 * <JA>
 * GMMの計算結果を標準出力に出力する．("-result tty" 用)
 * </JA>
 * <EN>
 * Output result of GMM computation to standard out.
 * (for "-result tty" option)
 * </EN>
 */
static void
result_gmm(Recog *recog, void *dummy)
{
  HTK_HMM_Data *d;
  GMMCalc *gc;
  int i;

#ifdef CALLBACK_DEBUG
  printf("<RESULT_GMM>\n");
#endif

  gc = recog->gc;

  if (debug2_flag) {
    printf("--- GMM result begin ---\n");
    i = 0;
    for(d=recog->model->gmm->start;d;d=d->next) {
      myprintf("  [%8s: total=%f avg=%f]\n", d->name, gc->gmm_score[i], gc->gmm_score[i] / (float)gc->framecount);
      i++;
    }
    myprintf("  max = \"%s\"", gc->max_d->name);
#ifdef CONFIDENCE_MEASURE
    printf(" (CM: %f)", gc->gmm_max_cm);
#endif
    printf("\n");
    printf("--- GMM result end ---\n");
  } else if (verbose_flag) {
    myprintf("GMM: max = \"%s\"", gc->max_d->name);
#ifdef CONFIDENCE_MEASURE
    printf(" (CM: %f)", gc->gmm_max_cm);
#endif
    printf("\n");
  } else {
#ifdef SP_BREAK_CURRENT_FRAME
    if (!recog->jconf->output.progout_flag) {
      myprintf("[GMM: %s]\n", gc->max_d->name);
    }
#else
    myprintf("[GMM: %s]\n", gc->max_d->name);
#endif
  }
}

/** 
 * <JA>
 * 現在保持している文法のリストを標準出力に出力する．
 * 
 * </JA>
 * <EN>
 * Output current list of grammars to stdout.
 * 
 * </EN>
 */
void 
print_all_gram(Recog *recog)
{
  MULTIGRAM *m;

  printf("[grammars]\n");
  for(m=recog->model->grammars;m;m=m->next) {
    printf("  #%2d: [%-11s] %4d words, %3d categories, %4d nodes",
	     m->id,
	     m->active ? "active" : "inactive",
	     m->winfo->num, m->dfa->term_num, m->dfa->state_num);
    if (m->newbie) printf(" (new)");
    if (m->hook != MULTIGRAM_DEFAULT) {
      printf(" (next: %s)", hookstr[m->hook]);
    }
    myprintf(" \"%s\"\n", m->name);
  }
  if (recog->model->dfa != NULL) {
    printf("  Global:            %4d words, %3d categories, %4d nodes\n", recog->model->winfo->num, recog->model->dfa->term_num, recog->model->dfa->state_num);
  }
}

static void
levelmeter(Recog *recog, SP16 *buf, int len, void *dummy)
{
  float d;
  int level;
  int i, n;

  level = 0;
  for(i=0;i<len;i++) {
    if (level < buf[i]) level = buf[i];
  }

  d = log((float)(level+1)) / 10.3971466; /* 10.3971466 = log(32767) */

  n = d * 8.0;
  fprintf(stderr, "\r");
  for(i=0;i<n;i++) fprintf(stderr, ">");

}


static void
status_gram(Recog *recog, void *dummy)
{
#ifdef CALLBACK_DEBUG
  printf("<GRAMMAR_UPDATE>\n");
#endif
  print_all_gram(recog);
}
  
static void
frame_indicator(Recog *recog, void *dummy)
{
#ifdef SP_BREAK_CURRENT_FRAME
  if (recog->pass1.in_sparea) {
    fprintf(stderr, ".");
  } else {
    fprintf(stderr, "-");
  }
#else  /* normal */
  fprintf(stderr, ".");
#endif /* SP_BREAK_CURRENT_FRAME */
}
  

void
setup_output_tty(Recog *recog, void *data)
{
  callback_add(recog, CALLBACK_EVENT_PROCESS_ONLINE, status_process_online, data);
  callback_add(recog, CALLBACK_EVENT_PROCESS_OFFLINE, status_process_offline, data);
  callback_add(recog, CALLBACK_EVENT_SPEECH_READY, status_recready, data);
  callback_add(recog, CALLBACK_EVENT_SPEECH_START, status_recstart, data);
  callback_add(recog, CALLBACK_EVENT_SPEECH_STOP, status_recend, data);
  callback_add(recog, CALLBACK_EVENT_RECOGNITION_BEGIN, status_recognition_begin, data);
  callback_add(recog, CALLBACK_EVENT_RECOGNITION_END, status_recognition_end, data);
#ifdef SP_BREAK_CURRENT_FRAME
  callback_add(recog, CALLBACK_EVENT_SEGMENT_BEGIN, status_segment_begin, data);
  callback_add(recog, CALLBACK_EVENT_SEGMENT_END, status_segment_end, data);
#endif
  callback_add(recog, CALLBACK_EVENT_PASS1_BEGIN, status_pass1_begin, data);
  if (!recog->jconf->search.pass1.realtime_flag && verbose_flag && (!recog->jconf->output.progout_flag)) {
    callback_add(recog, CALLBACK_EVENT_PASS1_FRAME, frame_indicator, data);
  }
  callback_add(recog, CALLBACK_RESULT_PASS1_INTERIM, result_pass1_current, data);
  callback_add(recog, CALLBACK_RESULT_PASS1, result_pass1, data);
#ifdef WORD_GRAPH
  callback_add(recog, CALLBACK_RESULT_PASS1_GRAPH, result_pass1_graph, data);
#endif
  callback_add(recog, CALLBACK_EVENT_PASS1_END, status_pass1_end, data);
  callback_add(recog, CALLBACK_STATUS_PARAM, status_param, data);
  callback_add(recog, CALLBACK_EVENT_PASS2_BEGIN, status_pass2_begin, data);
  callback_add(recog, CALLBACK_EVENT_PASS2_END, status_pass2_end, data);
  callback_add(recog, CALLBACK_RESULT, result_pass2, data); // rejected, failed
  callback_add(recog, CALLBACK_RESULT_GMM, result_gmm, data);
  /* below will be called when "-lattice" is specified */
  callback_add(recog, CALLBACK_RESULT_GRAPH, result_graph, data);
  /* below will be called when "-confnet" is specified */
  callback_add(recog, CALLBACK_RESULT_CONFNET, result_confnet, data);

  //callback_add_adin(CALLBACK_ADIN_CAPTURED, levelmeter, data);

  callback_add(recog, CALLBACK_EVENT_GRAMMAR_UPDATE, status_gram, data);

  callback_add(recog, CALLBACK_RESULT_PASS1_DETERMINED, result_pass1_determined, data);

}
