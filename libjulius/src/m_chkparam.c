/**
 * @file   m_chkparam.c
 * @author Akinobu LEE
 * @date   Fri Mar 18 16:31:45 2005
 * 
 * <JA>
 * @brief  指定オプションの整合性チェック，およびデフォルト値の設定．
 * </JA>
 * 
 * <EN>
 * @brief  Check option parameters and set default if needed.
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
 * ファイルが存在して読み込み可能かチェックする．
 * 
 * @param filename [in] ファイルパス名
 * </JA>
 * <EN>
 * Check if a file actually exist and is readable.
 * 
 * @param filename [in] file path name
 * </EN>
 */
boolean
checkpath(char *filename)
{
  if (access(filename, R_OK) == -1) {
    jlog("ERROR: m_chkparam: cannot access %s\n", filename);
    return FALSE;
  }
  return TRUE;
}

/** 
 * <JA>
 * @brief  指定されたパラメータをチェックする．
 *
 * ファイルの存在チェックやパラメータ指定の整合性，モデルとの対応
 * などについてチェックを行なう．重要な誤りが見つかった場合エラー終了する．
 * 
 * </JA>
 * <EN>
 * @brief  Check the user-specified parameters.
 *
 * This functions checks whether the specified files actually exist,
 * and also the mutual coherence of the parameters and their correspondence
 * with used model is also checked.  If a serious error is found, it
 * produces error and exits.
 * 
 * </EN>
 */
boolean
j_jconf_finalize(Jconf *jconf)
{
  boolean ok_p;

  jlog("STAT: ###### check configurations\n");

  ok_p = TRUE;

  if (jconf->lmtype == LM_UNDEF) {
    /* determine LM type from the specified LM files */
    if (jconf->lm.ngram_filename_lr_arpa || jconf->lm.ngram_filename_rl_arpa || jconf->lm.ngram_filename) {
      /* n-gram specified */
      jconf->lmtype = LM_PROB;
      jconf->lmvar  = LM_NGRAM;
    }
    if (jconf->lm.gramlist_root) {
      /* DFA grammar specified */
      if (jconf->lmtype != LM_UNDEF) {
	jlog("ERROR: m_chkparam: LM conflicts: several LM of different type specified?\n");
	return FALSE;
      }
      jconf->lmtype = LM_DFA;
      jconf->lmvar  = LM_DFA_GRAMMAR;
    }
    if (jconf->lm.dfa_filename) {
      /* DFA grammar specified by "-dfa" */
      if (jconf->lmtype != LM_UNDEF && jconf->lmvar != LM_DFA_GRAMMAR) {
	jlog("ERROR: m_chkparam: LM conflicts: several LM of different type specified?\n");
	return FALSE;
      }
      jconf->lmtype = LM_DFA;
      jconf->lmvar  = LM_DFA_GRAMMAR;
    }
    if (jconf->lm.wordlist_root) {
      /* word list specified */
      if (jconf->lmtype != LM_UNDEF) {
	jlog("ERROR: m_chkparam: LM conflicts: several LM of different type specified?\n");
      return FALSE;
      }
      jconf->lmtype = LM_DFA;
      jconf->lmvar  = LM_DFA_WORD;
    }
  }

  /* check if needed files are specified */
  if (jconf->am.hmmfilename == NULL) {
    jlog("ERROR: m_chkparam: needs HMM definition file (-h hmmdef_file)\n");
    ok_p = FALSE;
  }

  if (jconf->lmtype == LM_PROB) {
    if (jconf->lm.dictfilename == NULL) {
      jlog("ERROR: m_chkparam: needs dictionary file (-v dict_file)\n");
      ok_p = FALSE;
    }
  }

  if (jconf->lmtype == LM_DFA && jconf->lmvar == LM_DFA_WORD) {
    /* disable inter-word context dependent handling ("-no_ccd") */
    jconf->am.ccd_flag = FALSE;
    jconf->am.ccd_flag_force = TRUE;
    /* force 1pass ("-1pass") */
    jconf->sw.compute_only_1pass = TRUE;
  }

  /* file existence check */
  if (jconf->am.hmmfilename != NULL) 
    if (!checkpath(jconf->am.hmmfilename)) ok_p = FALSE;
  if (jconf->am.mapfilename != NULL) 
    if (!checkpath(jconf->am.mapfilename)) ok_p = FALSE;
  if (jconf->lm.dictfilename != NULL) 
    if (!checkpath(jconf->lm.dictfilename)) ok_p = FALSE;
  if (jconf->lm.ngram_filename != NULL) 
    if (!checkpath(jconf->lm.ngram_filename)) ok_p = FALSE;
  if (jconf->lm.ngram_filename_lr_arpa != NULL)
    if (!checkpath(jconf->lm.ngram_filename_lr_arpa)) ok_p = FALSE;
  if (jconf->lm.ngram_filename_rl_arpa != NULL)
    if (!checkpath(jconf->lm.ngram_filename_rl_arpa)) ok_p = FALSE;
  if (jconf->lm.dfa_filename != NULL) 
    if (!checkpath(jconf->lm.dfa_filename)) ok_p = FALSE;
  if (jconf->am.hmm_gs_filename != NULL) 
    if (!checkpath(jconf->am.hmm_gs_filename)) ok_p = FALSE;
  if (jconf->reject.gmm_filename != NULL) 
    if (!checkpath(jconf->reject.gmm_filename)) ok_p = FALSE;
  if (jconf->input.inputlist_filename != NULL) {
    if (jconf->input.speech_input != SP_RAWFILE && jconf->input.speech_input != SP_MFCFILE) {
      jlog("WARNING: m_chkparam: not file input, \"-filelist %s\" ignored\n", jconf->input.inputlist_filename);
    } else {
      if (!checkpath(jconf->input.inputlist_filename)) ok_p = FALSE;
    }
  }
  /* cmn{save,load}_filename allows missing file (skipped if missing) */
  if (jconf->frontend.ssload_filename != NULL) 
    if (!checkpath(jconf->frontend.ssload_filename)) ok_p = FALSE;

  /* set default realtime flag according to input mode */
  if (jconf->search.pass1.force_realtime_flag) {
    if (jconf->input.speech_input == SP_MFCFILE) {
      jlog("WARNING: m_chkparam: realtime decoding of mfcfile is not supported yet\n");
      jlog("WARNING: m_chkparam: realtime turned off\n");
      jconf->search.pass1.realtime_flag = FALSE;
    } else {
      jconf->search.pass1.realtime_flag = jconf->search.pass1.forced_realtime;
    }
  }

  /* check for cmn */
  if (jconf->search.pass1.realtime_flag) {
    if (jconf->frontend.cmn_update == FALSE && jconf->frontend.cmnload_filename == NULL) {
      jlog("ERROR: m_chkparam: when \"-cmnnoupdate\", initial cepstral mean should be given by \"-cmnload\"\n");
      ok_p = FALSE;
    }
  }

  if (jconf->search.pass1.iwcdmethod == IWCD_UNDEF) {
    switch(jconf->lmtype) {
    case LM_PROB:
      jconf->search.pass1.iwcdmethod = IWCD_NBEST; break;
    case LM_DFA:
      jconf->search.pass1.iwcdmethod = IWCD_AVG; break;
    }
  }

  /* check option validity with the current lm type */
  /* just a warning message for user */
  if (jconf->lmtype != LM_PROB) {
    /* in case not a probabilistic model */
    if (jconf->output.separate_score_flag) {
      jlog("WARNING: m_chkparam: \"-separatescore\" only for N-gram, ignored\n");
    }
    if (jconf->lm.lmp_specified) {
      jlog("WARNING: m_chkparam: \"-lmp\" only for N-gram, ignored\n");
    }
    if (jconf->lm.lmp2_specified) {
      jlog("WARNING: m_chkparam: \"-lmp2\" only for N-gram, ignored\n");
    }
    if (jconf->lm.lm_penalty_trans != 0.0) {
      jlog("WARNING: m_chkparam: \"-transp\" only for N-gram, ignored\n");
    }
    if (jconf->lm.head_silname && !strmatch(jconf->lm.head_silname, BEGIN_WORD_DEFAULT)) {
      jlog("WARNING: m_chkparam: \"-silhead\" only for N-gram, ignored\n");
    }
    if (jconf->lm.tail_silname && !strmatch(jconf->lm.tail_silname, END_WORD_DEFAULT)) {
      jlog("WARNING: m_chkparam: \"-siltail\" only for N-gram, ignored\n");
    }
    if (jconf->lm.enable_iwspword) {
      jlog("WARNING: m_chkparam: \"-iwspword\" only for N-gram, ignored\n");
    }
    if (jconf->lm.iwspentry && !strmatch(jconf->lm.iwspentry, IWSPENTRY_DEFAULT)) {
      jlog("WARNING: m_chkparam: \"-iwspentry\" only for N-gram, ignored\n");
    }
#ifdef HASH_CACHE_IW
    if (jconf->search.pass1.iw_cache_rate != 10) {
      jlog("WARNING: m_chkparam: \"-iwcache\" only for N-gram, ignored\n");
    }
#endif
#ifdef SEPARATE_BY_UNIGRAM
    if (jconf->search.pass1.separate_wnum != 150) {
      jlog("WARNING: m_chkparam: \"-sepnum\" only for N-gram, ignored\n");
    }
#endif
  }  
  if (jconf->lmtype != LM_DFA) {
    /* in case not a deterministic model */
    if (jconf->search.pass2.looktrellis_flag) {
      jlog("WARNING: m_chkparam: \"-looktrellis\" only for grammar, ignored\n");
    }
    if (jconf->output.multigramout_flag) {
      jlog("WARNING: m_chkparam: \"-multigramout\" only for grammar, ignored\n");
    }
    if (jconf->lm.penalty1 != 0.0) {
      jlog("WARNING: m_chkparam: \"-penalty1\" only for grammar, ignored\n");
    }
    if (jconf->lm.penalty2 != 0.0) {
      jlog("WARNING: m_chkparam: \"-penalty2\" only for grammar, ignored\n");
    }
  }

  if (!ok_p) {
    jlog("ERROR: m_chkparam: could not pass parameter check\n");
  }

  return ok_p;
}

/******* set default params suitable for the models and setting *******/

/** 
 * <JA>
 * @brief  あらかじめ定められた第1パスのデフォルトビーム幅を返す．
 *
 * デフォルトのビーム幅は，認識エンジンのコンパイル時設定や
 * 使用する音響モデルに従って選択される．これらの値は，20k の
 * IPA 評価セットで得られた最適値（精度を保ちつつ最大速度が得られる値）
 * である．
 * 
 * @return 実行時の条件によって選択されたビーム幅
 * </JA>
 * <EN>
 * @brief  Returns the pre-defined default beam width on 1st pass of
 * beam search.
 * 
 * The default beam width will be selected from the pre-defined values
 * according to the compilation-time engine setting and the type of
 * acoustic model.  The pre-defined values were determined from the
 * development experiments on IPA evaluation testset of Japanese 20k-word
 * dictation task.
 * 
 * @return the selected default beam width.
 * </EN>
 */
static int
default_width(HTK_HMM_INFO *hmminfo)
{
  if (strmatch(JULIUS_SETUP, "fast")) { /* for fast setup */
    if (hmminfo->is_triphone) {
      if (hmminfo->is_tied_mixture) {
	/* tied-mixture triphones (PTM etc.) */
	return(600);
      } else {
	/* shared-state triphone */
#ifdef PASS1_IWCD
	return(800);
#else
	/* v2.1 compliant (no IWCD on 1st pass) */
	return(1000);		
#endif
      }
    } else {
      /* monophone */
      return(400);
    }
  } else {			/* for standard / v2.1 setup */
    if (hmminfo->is_triphone) {
      if (hmminfo->is_tied_mixture) {
	/* tied-mixture triphones (PTM etc.) */
	return(800);
      } else {
	/* shared-state triphone */
#ifdef PASS1_IWCD
	return(1500);
#else
	return(1500);		/* v2.1 compliant (no IWCD on 1st pass) */
#endif
      }
    } else {
      /* monophone */
      return(700);
    }
  }
}

/** 
 * <JA>
 * @brief  第1パスのビーム幅を決定する．
 *
 * ユーザが "-b" オプションでビーム幅を指定しなかった場合は，
 * 下記のうち小さい方がビーム幅として採用される．
 *   - default_width() の値
 *   - sqrt(語彙数) * 15
 * 
 * @param wchmm [in] 木構造化辞書
 * @param specified [in] ユーザ指定ビーム幅(0: 全探索 -1: 未指定)
 * 
 * @return 採用されたビーム幅．
 * </JA>
 * <EN>
 * @brief  Determine beam width on the 1st pass.
 * 
 * @param wchmm [in] tree lexicon data
 * @param specified [in] user-specified beam width (0: full search,
 * -1: not specified)
 * 
 * @return the final beam width to be used.
 * </EN>
 */
int
set_beam_width(WCHMM_INFO *wchmm, int specified)
{
  int width;
  int standard_width;
  
  if (specified == 0) { /* full search */
    jlog("WARNING: doing full search (can be extremely slow)\n");
    width = wchmm->n;
  } else if (specified == -1) { /* not specified */
    standard_width = default_width(wchmm->hmminfo); /* system default */
    width = (int)(sqrt(wchmm->winfo->num) * 15.0); /* heuristic value!! */
    if (width > standard_width) width = standard_width;
    /* 2007/1/20 bgn */
    if (width < MINIMAL_BEAM_WIDTH) {
      width = MINIMAL_BEAM_WIDTH;
    }
    /* 2007/1/20 end */
  } else {			/* actual value has been specified */
    width = specified;
  }
  if (width > wchmm->n) width = wchmm->n;

  return(width);
}

/** 
 * <JA>
 * 第1パスの言語モデルの重みと単語挿入ペナルティのデフォルト値を，
 * 音響モデルの型に従ってセットする．
 * 
 * </JA>
 * <EN>
 * Set default values of LM weight and word insertion penalty on the 1st pass
 * depending on the acoustic model type.
 * 
 * </EN>
 */
void
set_lm_weight(Jconf *jconf, Model *model)
{
  if (model->hmminfo->is_triphone) {
    jconf->lm.lm_weight = DEFAULT_LM_WEIGHT_TRI_PASS1;
    jconf->lm.lm_penalty = DEFAULT_LM_PENALTY_TRI_PASS1;
  } else {
    jconf->lm.lm_weight = DEFAULT_LM_WEIGHT_MONO_PASS1;
    jconf->lm.lm_penalty = DEFAULT_LM_PENALTY_MONO_PASS1;
  }
}

/** 
 * <JA>
 * 第2パスの言語モデルの重みと単語挿入ペナルティのデフォルト値を，
 * 音響モデルの型に従ってセットする．
 * 
 * </JA>
 * <EN>
 * Set default values of LM weight and word insertion penalty on the 2nd pass
 * depending on the acoustic model type.
 * 
 * </EN>
 */
void
set_lm_weight2(Jconf *jconf, Model *model)
{
  if (model->hmminfo->is_triphone) {
    jconf->lm.lm_weight2 = DEFAULT_LM_WEIGHT_TRI_PASS2;
    jconf->lm.lm_penalty2 = DEFAULT_LM_PENALTY_TRI_PASS2;
  } else {
    jconf->lm.lm_weight2 = DEFAULT_LM_WEIGHT_MONO_PASS2;
    jconf->lm.lm_penalty2 = DEFAULT_LM_PENALTY_MONO_PASS2;
  }
}
