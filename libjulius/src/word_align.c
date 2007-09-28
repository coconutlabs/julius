/**
 * @file   word_align.c
 * @author Akinobu Lee
 * @date   Sat Sep 24 16:09:46 2005
 * 
 * <JA>
 * @brief  ñ�졦���ǡ����֥��饤����Ȥ��롥
 *
 * �����Ǥϡ�ǧ����̤��Ф������ϲ����Υ��饤����Ȥ���Ϥ��뤿���
 * �ؿ����������Ƥ��ޤ���
 *
 * Julius/Julian �Ǥϡ�ǧ����̤ˤ����Ƥ���ñ��䲻�ǡ����뤤��HMM�ξ��֤�
 * ���줾�����ϲ����Τɤζ�֤˥ޥå������Τ����Τ뤳�Ȥ��Ǥ��ޤ���
 * ������Τʥ��饤����Ȥ���뤿��ˡ�Julius/Julian �Ǥ�ǧ�����
 * �����ޤ������Ѥ����ˡ�ǧ��������ä��������줿ǧ����̤�ñ�����
 * �Ф��ơ����餿��� forced alignment ��¹Ԥ��Ƥ��ޤ���
 * </JA>
 * 
 * <EN>
 * @brief  Do Viterbi alignment per word / phoneme/ state.
 *
 * This file defines functions for performing forced alignment of
 * recognized words.  The forced alignment is implimented in Julius/Julian
 * to get the best matching segmentation of recognized word sequence
 * upon input speech.  Word-level, phoneme-level and HMM state-level
 * alignment can be obtained.
 *
 * Julius/Julian performs the forced alignment as a post-processing of
 * recognition process.  Recomputation of Viterbi path on the recognized
 * word sequence toward input speech will be done after the recognition
 * to get better alignment.
 *
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
 * <JA>
 * Ϳ����줿ñ���󤫤�HMM��Ϣ�뤷��ʸ���Τ�HMM���ۤ��롥
 * 
 * @param wseq [in] ñ����
 * @param num [in] @a wseq �ο�
 * @param num_ret [out] ���ۤ��줿HMM�˴ޤޤ�벻��HMM�ο�
 * @param end_ret [out] ���饤����Ȥζ��ڤ�Ȥʤ�����ֹ����
 * @param per_what [in] ñ�졦���ǡ����֤Τɤ�ñ�̤ǥ��饤����Ȥ��뤫�����
 * 
 * @return ���餿�˳���դ���줿ʸ���Τ򤢤�魯HMM��ǥ���ؤΥݥ��󥿤��֤���
 * </JA>
 * <EN>
 * Make the whole sentence HMM from given word sequence by connecting
 * each phoneme HMM.
 * 
 * @param wseq [in] word sequence to align
 * @param num [in] number of @a wseq
 * @param num_ret [out] number of HMM contained in the generated sentence HMM
 * @param end_ret [out] sequence of state location as alignment unit
 * @param per_what [in] specify the alignment unit (word / phoneme / state)
 * 
 * @return newly malloced HMM sequences.
 * </EN>
 */
static HMM_Logical **
make_phseq(WORD_ID *wseq, short num, boolean **has_sp_ret, int *num_ret, int **end_ret, int per_what, 
	   Recog *recog)
{
  HMM_Logical **ph;		/* phoneme sequence */
  boolean *has_sp;
  int k;
  int phnum;			/* num of above */
  WORD_ID tmpw, w;
  int i, j, pn, st, endn;
  HMM_Logical *tmpp, *ret;
  WORD_INFO *winfo;
  HTK_HMM_INFO *hmminfo;
  boolean enable_iwsp;		/* for multipath */

  winfo = recog->model->winfo;
  hmminfo = recog->model->hmminfo;
  if (hmminfo->multipath) enable_iwsp = recog->jconf->lm.enable_iwsp;

  /* make ph[] from wseq[] */
  /* 1. calc total phone num and malloc */
  phnum = 0;
  for (w=0;w<num;w++) phnum += winfo->wlen[wseq[w]];
  ph = (HMM_Logical **)mymalloc(sizeof(HMM_Logical *) * phnum);
  
  if (hmminfo->multipath) {
    has_sp = (boolean *)mymalloc(sizeof(boolean) * phnum);
  }
  /* 2. make phoneme sequence */
  st = 0;
  if (hmminfo->multipath) st++;
  pn = 0;
  endn = 0;
  for (w=0;w<num;w++) {
    tmpw = wseq[w];
    for (i=0;i<winfo->wlen[tmpw];i++) {
      tmpp = winfo->wseq[tmpw][i];
      /* handle cross-word context dependency */
      if (recog->ccd_flag) {
	if (w > 0 && i == 0) {	/* word head */
	  
	  if ((ret = get_left_context_HMM(tmpp, ph[pn-1]->name, hmminfo)) != NULL) {
	    tmpp = ret;
	  }
	  /* if triphone not found, fallback to bi/mono-phone  */
	  /* use pseudo phone when no bi-phone found in alignment... */
	}
	if (w < num-1 && i == winfo->wlen[tmpw] - 1) { /* word tail */
	  if ((ret = get_right_context_HMM(tmpp, winfo->wseq[wseq[w+1]][0]->name, hmminfo)) != NULL) {
	    tmpp = ret;
	  }
	}
      }
      ph[pn] = tmpp;
      if (hmminfo->multipath) {
	if (enable_iwsp && i == winfo->wlen[tmpw] - 1) {
	  has_sp[pn] = TRUE;
	} else {
	  has_sp[pn] = FALSE;
	}
      }
      if (per_what == PER_STATE) {
	for (j=0;j<hmm_logical_state_num(tmpp)-2;j++) {
	  (*end_ret)[endn++] = st + j;
	}
	if (hmminfo->multipath && enable_iwsp && has_sp[pn]) {
	  for (k=0;k<hmm_logical_state_num(hmminfo->sp)-2;k++) {
	    (*end_ret)[endn++] = st + j + k;
	  }
	}
      }
      st += hmm_logical_state_num(tmpp) - 2;
      if (hmminfo->multipath && enable_iwsp && has_sp[pn]) {
	st += hmm_logical_state_num(hmminfo->sp) - 2;
      }
      if (per_what == PER_PHONEME) (*end_ret)[endn++] = st - 1;
      pn++;
    }
    if (per_what == PER_WORD) (*end_ret)[endn++] = st - 1;
  }
  *num_ret = phnum;
  if (hmminfo->multipath) *has_sp_ret = has_sp;
  return ph;
}


/** 
 * <JA>
 * ʸ���Τ�HMM���ۤ���Viterbi���饤����Ȥ�¹Ԥ�����̤���Ϥ��롥
 * 
 * @param words [in] ʸ����򤢤�魯ñ����
 * @param wnum [in] @a words ��Ĺ��
 * @param param [in] ������ħ�ѥ�᡼����
 * @param per_what [in] ñ�졦���ǡ����֤Τɤ�ñ�̤ǥ��饤����Ȥ��뤫�����
 * </JA>
 * <EN>
 * Build sentence HMM, call viterbi_segment() and output result.
 * 
 * @param words [in] word sequence of the sentence
 * @param wnum [in] number of words in @a words
 * @param param [in] input parameter vector
 * @param per_what [in] specify the alignment unit (word / phoneme / state)
 * </EN>
 */
static void
do_align(WORD_ID *words, short wnum, HTK_Param *param, int per_what, Sentence *s, Recog *recog)
{
  HMM_Logical **phones;		/* phoneme sequence */
  boolean *has_sp;		/* whether phone can follow short pause */
  int k;
  int phonenum;			/* num of above */
  HMM *shmm;			/* sentence HMM */
  int *end_state;		/* state number of word ends */
  int *end_frame;		/* segmented last frame of words */
  LOGPROB *end_score;		/* normalized score of each words */
  LOGPROB allscore;		/* total score of this word sequence */
  WORD_ID w;
  int i, rlen;
  int end_num = 0;
  int *id_seq, *phloc = NULL, *stloc = NULL;
  int j,n,p;
  WORD_INFO *winfo;
  HTK_HMM_INFO *hmminfo;
  boolean enable_iwsp;		/* for multipath */

  winfo = recog->model->winfo;
  hmminfo = recog->model->hmminfo;
  if (hmminfo->multipath) enable_iwsp = recog->jconf->lm.enable_iwsp;

  /* initialize result storage buffer */
  switch(per_what) {
  case PER_WORD:
    jlog("ALIGN: === word alignment begin ===\n");
    end_num = wnum;
    phloc = (int *)mymalloc(sizeof(int)*wnum);
    i = 0;
    for(w=0;w<wnum;w++) {
      phloc[w] = i;
      i += winfo->wlen[words[w]];
    }
    break;
  case PER_PHONEME:
    jlog("ALIGN: === phoneme alignment begin ===\n");
    end_num = 0;
    for(w=0;w<wnum;w++) end_num += winfo->wlen[words[w]];
    break;
  case PER_STATE:
    jlog("ALIGN: === state alignment begin ===\n");
    end_num = 0;
    for(w=0;w<wnum;w++) {
      for (i=0;i<winfo->wlen[words[w]]; i++) {
	end_num += hmm_logical_state_num(winfo->wseq[words[w]][i]) - 2;
      }
      if (hmminfo->multipath && enable_iwsp) {
	end_num += hmm_logical_state_num(hmminfo->sp) - 2;
      }
    }
    phloc = (int *)mymalloc(sizeof(int)*end_num);
    stloc = (int *)mymalloc(sizeof(int)*end_num);
    {
      n = 0;
      p = 0;
      for(w=0;w<wnum;w++) {
	for(i=0;i<winfo->wlen[words[w]]; i++) {
	  for(j=0; j<hmm_logical_state_num(winfo->wseq[words[w]][i]) - 2; j++) {
	    phloc[n] = p;
	    stloc[n] = j + 1;
	    n++;
	  }
	  if (hmminfo->multipath && enable_iwsp && i == winfo->wlen[words[w]] - 1) {
	    for(k=0;k<hmm_logical_state_num(hmminfo->sp)-2;k++) {
	      phloc[n] = p;
	      stloc[n] = j + 1 + k + end_num;
	      n++;
	    }
	  }
	  p++;
	}
      }
    }
    
    break;
  }
  end_state = (int *)mymalloc(sizeof(int) * end_num);

  /* make phoneme sequence word sequence */
  phones = make_phseq(words, wnum, hmminfo->multipath ? &has_sp : NULL, &phonenum, &end_state, per_what, recog);
  /* build the sentence HMMs */
  shmm = new_make_word_hmm(hmminfo, phones, phonenum, hmminfo->multipath ? has_sp : NULL);
  if (shmm == NULL) {
    j_internal_error("Error: failed to make word hmm for alignment\n");
  }

  /* call viterbi segmentation function */
  allscore = viterbi_segment(shmm, param, recog->wchmm->hmmwrk, hmminfo->multipath, end_state, end_num, &id_seq, &end_frame, &end_score, &rlen);

  /* store result to s */
  s->align.num = rlen;
  s->align.unittype = per_what;
  s->align.begin_frame = (int *)mymalloc(sizeof(int) * rlen);
  s->align.end_frame   = (int *)mymalloc(sizeof(int) * rlen);
  s->align.avgscore    = (LOGPROB *)mymalloc(sizeof(LOGPROB) * rlen);
  for(i=0;i<rlen;i++) {
    s->align.begin_frame[i] = (i == 0) ? 0 : end_frame[i-1] + 1;
    s->align.end_frame[i]   = end_frame[i];
    s->align.avgscore[i]    = end_score[i];
  }
  switch(per_what) {
  case PER_WORD:
    s->align.w = (WORD_ID *)mymalloc(sizeof(WORD_ID) * rlen);
    for(i=0;i<rlen;i++) {
      s->align.w[i] = words[id_seq[i]];
    }
    break;
  case PER_PHONEME:
    s->align.ph = (HMM_Logical **)mymalloc(sizeof(HMM_Logical *) * rlen);
    for(i=0;i<rlen;i++) {
      s->align.ph[i] = phones[id_seq[i]];
    }
    break;
  case PER_STATE:
    s->align.ph = (HMM_Logical **)mymalloc(sizeof(HMM_Logical *) * rlen);
    s->align.loc = (short *)mymalloc(sizeof(short) * rlen);
    if (hmminfo->multipath) s->align.is_iwsp = (boolean *)mymalloc(sizeof(boolean) * rlen);
    for(i=0;i<rlen;i++) {
      s->align.ph[i]  = phones[phloc[id_seq[i]]];
      if (hmminfo->multipath) {
	if (enable_iwsp && stloc[id_seq[i]] > end_num) {
	  s->align.loc[i] = stloc[id_seq[i]] - end_num;
	  s->align.is_iwsp[i] = TRUE;
	} else {
	  s->align.loc[i] = stloc[id_seq[i]];
	  s->align.is_iwsp[i] = FALSE;
	}
      } else {
	s->align.loc[i] = stloc[id_seq[i]];
      }
    }
    break;
  }

  s->align.allscore = allscore;

  s->align.filled = TRUE;

  free_hmm(shmm);
  free(id_seq);
  free(phones);
  if (hmminfo->multipath) free(has_sp);
  free(end_score);
  free(end_frame);
  free(end_state);

  switch(per_what) {
  case PER_WORD:
    free(phloc);
    break;
  case PER_PHONEME:
    break;
  case PER_STATE:
    free(phloc);
    free(stloc);
  }
  
}

/* entry functions */
/** 
 * <JA>
 * ñ�줴�Ȥ� forced alignment ��Ԥ���
 * 
 * @param words [in] ñ����
 * @param wnum [in] @a words ��ñ���
 * @param param [in] ������ħ�٥��ȥ���
 * </JA>
 * <EN>
 * Do forced alignment per word for the given word sequence.
 * 
 * @param words [in] word sequence
 * @param wnum [in] length of @a words
 * @param param [in] input parameter vectors
 * </EN>
 */
void
word_align(WORD_ID *words, short wnum, HTK_Param *param, Sentence *s, Recog *recog)
{
  do_align(words, wnum, param, PER_WORD, s, recog);
}

/** 
 * <JA>
 * ñ�줴�Ȥ� forced alignment ��Ԥ���ñ�줬�ս��Ϳ���������
 * 
 * @param revwords [in] ñ����ʵս��
 * @param wnum [in] @a revwords ��ñ���
 * @param param [in] ������ħ�٥��ȥ���
 * </JA>
 * <EN>
 * Do forced alignment per word for the given word sequence (reversed order).
 * 
 * @param revwords [in] word sequence in reversed direction
 * @param wnum [in] length of @a revwords
 * @param param [in] input parameter vectors
 * </EN>
 */
void
word_rev_align(WORD_ID *revwords, short wnum, HTK_Param *param, Sentence *s, Recog *recog)
{
  WORD_ID *words;		/* word sequence (true order) */
  int w;
  words = (WORD_ID *)mymalloc(sizeof(WORD_ID) * wnum);
  for (w=0;w<wnum;w++) words[w] = revwords[wnum-w-1];
  do_align(words, wnum, param, PER_WORD, s, recog);
  free(words);
}

/** 
 * <JA>
 * ���Ǥ��Ȥ� forced alignment ��Ԥ���
 * 
 * @param words [in] ñ����
 * @param num [in] @a words ��ñ���
 * @param param [in] ������ħ�٥��ȥ���
 * </JA>
 * <EN>
 * Do forced alignment per phoneme for the given word sequence.
 * 
 * @param words [in] word sequence
 * @param num [in] length of @a words
 * @param param [in] input parameter vectors
 * </EN>
 */
void
phoneme_align(WORD_ID *words, short num, HTK_Param *param, Sentence *s, Recog *recog)
{
  do_align(words, num, param, PER_PHONEME, s, recog);
}

/** 
 * <JA>
 * ���Ǥ��Ȥ� forced alignment ��Ԥ���ñ�줬�ս��Ϳ���������
 * 
 * @param revwords [in] ñ����ʵս��
 * @param num [in] @a revwords ��ñ���
 * @param param [in] ������ħ�٥��ȥ���
 * </JA>
 * <EN>
 * Do forced alignment per phoneme for the given word sequence (reversed order).
 * 
 * @param revwords [in] word sequence in reversed direction
 * @param num [in] length of @a revwords
 * @param param [in] input parameter vectors
 * </EN>
 */
void
phoneme_rev_align(WORD_ID *revwords, short num, HTK_Param *param, Sentence *s, Recog *recog)
{
  WORD_ID *words;		/* word sequence (true order) */
  int p;
  words = (WORD_ID *)mymalloc(sizeof(WORD_ID) * num);
  for (p=0;p<num;p++) words[p] = revwords[num-p-1];
  do_align(words, num, param, PER_PHONEME, s, recog);
  free(words);
}

/** 
 * <JA>
 * HMM���֤��Ȥ� forced alignment ��Ԥ���
 * 
 * @param words [in] ñ����
 * @param num [in] @a words ��ñ���
 * @param param [in] ������ħ�٥��ȥ���
 * </JA>
 * <EN>
 * Do forced alignment per HMM state for the given word sequence.
 * 
 * @param words [in] word sequence
 * @param num [in] length of @a words
 * @param param [in] input parameter vectors
 * </EN>
 */
void
state_align(WORD_ID *words, short num, HTK_Param *param, Sentence *s, Recog *recog)
{
  do_align(words, num, param, PER_STATE, s, recog);
}

/** 
 * <JA>
 * HMM���֤��Ȥ� forced alignment ��Ԥ���ñ�줬�ս��Ϳ���������
 * 
 * @param revwords [in] ñ����ʵս��
 * @param num [in] @a revwords ��ñ���
 * @param param [in] ������ħ�٥��ȥ���
 * </JA>
 * <EN>
 * Do forced alignment per state for the given word sequence (reversed order).
 * 
 * @param revwords [in] word sequence in reversed direction
 * @param num [in] length of @a revwords
 * @param param [in] input parameter vectors
 * </EN>
 */
void
state_rev_align(WORD_ID *revwords, short num, HTK_Param *param, Sentence *s, Recog *recog)
{
  WORD_ID *words;		/* word sequence (true order) */
  int p;
  words = (WORD_ID *)mymalloc(sizeof(WORD_ID) * num);
  for (p=0;p<num;p++) words[p] = revwords[num-p-1];
  do_align(words, num, param, PER_STATE, s, recog);
  free(words);
}
