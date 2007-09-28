/**
 * @file   m_fusion.c
 * @author Akinobu Lee
 * @date   Thu May 12 13:31:47 2005
 * 
 * <JA>
 * @brief  モデルの読み込みとデータの構築を行ない，認識の準備をする．
 * </JA>
 * 
 * <EN>
 * @brief  Read all models and build data for recognition.
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
 * 音響HMMをファイルから読み込んでセットアップする．
 * </JA>
 * <EN>
 * Read in an acoustic HMM from file and setup for recognition.
 * </EN>
 */
static HTK_HMM_INFO *
initialize_HMM(Jconf *jconf)
{
  HTK_HMM_INFO *hmminfo;

  /* at here, global variable "para" holds values specified by user or
     by user-specified HTK config file */
  
  /* allocate new hmminfo */
  hmminfo = hmminfo_new();
  /* load hmmdefs */
  if (init_hmminfo(hmminfo, jconf->am.hmmfilename, jconf->am.mapfilename, &jconf->analysis.para_hmm) == FALSE) {
    hmminfo_free(hmminfo);
    return NULL;
  }
  /* set multipath mode flag */
  if (jconf->am.force_multipath) {
    jlog("STAT: m_fusion: force multipath HMM handling by user request\n");
    hmminfo->multipath = TRUE;
  } else {
    hmminfo->multipath = hmminfo->need_multipath;
  }

  /* only MFCC is supported for audio input */
  /* MFCC_{0|E}[_D][_A][_Z][_N] is supported */
  /* check parameter type of this acoustic HMM */
  if (jconf->input.speech_input != SP_MFCFILE) {
    /* Decode parameter extraction type according to the training
       parameter type in the header of the given acoustic HMM */
    if ((hmminfo->opt.param_type & F_BASEMASK) != F_MFCC) {
      jlog("ERROR: m_fusion: for direct speech input, only HMM trained by MFCC is supported\n");
      hmminfo_free(hmminfo);
      return NULL;
    }
    /* set acoustic analysis parameters from HMM header */
    calc_para_from_header(&(jconf->analysis.para), hmminfo->opt.param_type, hmminfo->opt.vec_size);
  }
  /* check if tied_mixture */
  if (hmminfo->is_tied_mixture && hmminfo->codebooknum <= 0) {
    jlog("ERROR: m_fusion: this tied-mixture model has no codebook!?\n");
    hmminfo_free(hmminfo);
    return NULL;
  }
  /* set flag for context dependent handling (if not specified in command arg) */
  if (!jconf->am.ccd_flag_force) {
    if (hmminfo->is_triphone) {
      jconf->am.ccd_flag = TRUE;
    } else {
      jconf->am.ccd_flag = FALSE;
    }
  }
#ifdef PASS1_IWCD
  /* make state clusters of same context for inter-word triphone approx. */
  if (hmminfo->is_triphone && jconf->am.ccd_flag) {
    jlog("STAT: making pseudo bi/mono-phone for IW-triphone...\n");
    if (make_cdset(hmminfo) == FALSE) {
      jlog("ERROR: m_fusion: failed to make context-dependent state set\n");
      hmminfo_free(hmminfo);
      return NULL;
    }
    /* add those `pseudo' biphone and monophone to the logical HMM names */
    /* they points not to the defined HMM, but to the CD_Set structure */
    hmm_add_pseudo_phones(hmminfo);
    jlog("STAT: done\n");
  }
#endif

  /* find short pause model and set to hmminfo->sp */
  htk_hmm_set_pause_model(hmminfo, jconf->am.spmodel_name);

  /* set which iwcd1 method to use */
  hmminfo->cdset_method = jconf->search.pass1.iwcdmethod;
  hmminfo->cdmax_num = jconf->search.pass1.iwcdmaxn;

  if (jconf->lm.enable_iwsp) {
    if (hmminfo->multipath) {
    /* find short-pause model */
      if (hmminfo->sp == NULL) {
	jlog("ERROR: iwsp enabled but no short pause model \"%s\" in hmmdefs\n", jconf->am.spmodel_name);
	hmminfo_free(hmminfo);
	return NULL;
      }
      hmminfo->iwsp_penalty = jconf->lm.iwsp_penalty;
    } else {
      jlog("Warning: \"-iwsp\" is supported on multi-path mode, ignored\n");
    }
  }

  return(hmminfo);
  
}

/** 
 * <JA>
 * Gaussian Mixture Selection のための状態選択用モノフォンHMMを読み込む．
 * </JA>
 * <EN>
 * Initialize context-independent HMM for state selection with Gaussian
 * Mixture Selection.
 * </EN>
 */
static HTK_HMM_INFO *
initialize_GSHMM(Jconf *jconf)
{
  HTK_HMM_INFO *hmm_gs;
  Value para_dummy;

  jlog("STAT: Reading GS HMMs:\n");
  hmm_gs = hmminfo_new();
  undef_para(&para_dummy);
  if (init_hmminfo(hmm_gs, jconf->am.hmm_gs_filename, NULL, &para_dummy) == FALSE) {
    hmminfo_free(hmm_gs);
    return NULL;
  }
  return(hmm_gs);
}

/* initialize GMM for utterance verification */
/** 
 * <JA>
 * 発話検証・棄却用の1状態 GMM を読み込んで初期化する．
 * 
 * </JA>
 * <EN>
 * Read and initialize an 1-state GMM for utterance verification and
 * rejection.
 * 
 * </EN>
 */
static HTK_HMM_INFO *
initialize_GMM(Jconf *jconf)
{
  HTK_HMM_INFO *gmm;
  Value para_dummy;
  
  jlog("STAT: reading GMM:\n");
  gmm = hmminfo_new();
  undef_para(&para_dummy);
  if (init_hmminfo(gmm, jconf->reject.gmm_filename, NULL, &para_dummy) == FALSE) {
    hmminfo_free(gmm);
    return NULL;
  }
  return(gmm);
}

/* initialize word dictionary */
/** 
 * <JA>
 * 単語辞書をファイルから読み込んでセットアップする．
 * 
 * </JA>
 * <EN>
 * Read in word dictionary from a file and setup for recognition.
 * 
 * </EN>
 */
static WORD_INFO *
initialize_dict(Jconf *jconf, HTK_HMM_INFO *hmminfo)
{
  WORD_INFO *winfo;

  /* allocate new word dictionary */
  winfo = word_info_new();
  /* read in dictinary from file */
  if ( ! 
#ifdef MONOTREE
      /* leave winfo monophone for 1st pass lexicon tree */
       init_voca(winfo, jconf->lm.dictfilename, hmminfo, TRUE, jconf->lm.forcedict_flag)
#else 
       init_voca(winfo, jconf->lm.dictfilename, hmminfo, FALSE, jconf->lm.forcedict_flag)
#endif
       ) {
    jlog("ERROR: m_fusion: failed to read dictionary, terminated\n");
    word_info_free(winfo);
    return NULL;
  }

  if (jconf->lmtype == LM_PROB) {
    /* if necessary, append a IW-sp word to the dict if "-iwspword" specified */
    if (jconf->lm.enable_iwspword) {
      if (
#ifdef MONOTREE
	  voca_append_htkdict(jconf->lm.iwspentry, winfo, hmminfo, TRUE)
#else 
	  voca_append_htkdict(jconf->lm.iwspentry, winfo, hmminfo, FALSE)
#endif
	  == FALSE) {
	jlog("ERROR: m_fusion: failed to make IW-sp word entry \"%s\"\n", jconf->lm.iwspentry);
	word_info_free(winfo);
	return NULL;
      } else {
	jlog("STAT: 1 IW-sp word entry added\n");
      }
    }
    /* set {head,tail}_silwid */
    winfo->head_silwid = voca_lookup_wid(jconf->lm.head_silname, winfo);
    if (winfo->head_silwid == WORD_INVALID) { /* not exist */
      jlog("ERROR: m_fusion: head sil word \"%s\" not exist in voca\n", jconf->lm.head_silname);
      word_info_free(winfo);
      return NULL;
    }
    winfo->tail_silwid = voca_lookup_wid(jconf->lm.tail_silname, winfo);
    if (winfo->tail_silwid == WORD_INVALID) { /* not exist */
      jlog("ERROR: m_fusion: tail sil word \"%s\" not exist in voca\n", jconf->lm.tail_silname);
      word_info_free(winfo);
      return NULL;
    }
  }
  
  return(winfo);

}


/** 
 * <JA>
 * 単語N-gramをファイルから読み込んでセットアップする．
 * 
 * </JA>
 * <EN>
 * Read in word N-gram from file and setup for recognition.
 * 
 * </EN>
 */
static NGRAM_INFO *
initialize_ngram(Jconf *jconf, WORD_INFO *winfo)
{
  NGRAM_INFO *ngram;
  boolean ret;

  /* allocate new */
  ngram = ngram_info_new();
  /* load LM */
  if (jconf->lm.ngram_filename != NULL) {	/* binary format */
    ret = init_ngram_bin(ngram, jconf->lm.ngram_filename);
  } else {			/* ARPA format */
    /* if either forward or backward N-gram is specified, read it */
    /* if both specified, use backward N-gram as main and
       use forward 2-gram only for 1st pass (this is an old behavior) */
    if (jconf->lm.ngram_filename_rl_arpa) {
      ret = init_ngram_arpa(ngram, jconf->lm.ngram_filename_rl_arpa, DIR_RL);
      if (ret == FALSE) {
	ngram_info_free(ngram);
	return NULL;
      }
      if (jconf->lm.ngram_filename_lr_arpa) {
	ret = init_ngram_arpa_additional(ngram, jconf->lm.ngram_filename_lr_arpa);
	if (ret == FALSE) {
	  ngram_info_free(ngram);
	  return NULL;
	}
      }
    } else if (jconf->lm.ngram_filename_lr_arpa) {
      ret = init_ngram_arpa(ngram, jconf->lm.ngram_filename_lr_arpa, DIR_LR);
    }
  }
  if (ret == FALSE) {
    ngram_info_free(ngram);
    return NULL;
  }

  /* map dict item to N-gram entry */
  make_voca_ref(ngram, winfo);

  return(ngram);
}

/* set params whose default will change by models and not specified in arg */
/** 
 * <JA>
 * @brief モデルに依存したデフォルト値の設定
 * 
 * 言語重みや Gaussian pruning アルゴリズムの選択など，モデルによって
 * デフォルト設定が異なるパラメータをここで決定する．なおユーザによって
 * 明示的に指定されている場合はそちらを優先する．
 * </JA>
 * <EN>
 * @brief Set model-dependent default values.
 *
 * The default values of parameters which depends on the using models,
 * such as language weights, insertion penalty, gaussian pruning
 * methods and so on, are determined at this function.  If values are
 * explicitly defined in jconf file or command argument at run time,
 * they will be used instead.
 * </EN>
 */
static void
configure_param(Jconf *jconf, Model *model)
{
  if (jconf->lmtype == LM_PROB) {
    /* set default lm parameter */
    if (!jconf->lm.lmp_specified) set_lm_weight(jconf, model);
    if (!jconf->lm.lmp2_specified) set_lm_weight2(jconf, model);
    if (jconf->lm.lmp_specified != jconf->lm.lmp2_specified) {
      jlog("WARNING: m_fusion: only -lmp or -lmp2 specified, LM weights may be unbalanced\n");
    }
  }
  /* select Gaussian pruning function */
  if (jconf->am.gprune_method == GPRUNE_SEL_UNDEF) {/* set default if not specified */
    if (model->hmminfo->is_tied_mixture) {
      /* enabled by default for tied-mixture models */
#ifdef GPRUNE_DEFAULT_SAFE
      jconf->am.gprune_method = GPRUNE_SEL_SAFE;
#elif GPRUNE_DEFAULT_HEURISTIC
      jconf->am.gprune_method = GPRUNE_SEL_HEURISTIC;
#elif GPRUNE_DEFAULT_BEAM
      jconf->am.gprune_method = GPRUNE_SEL_BEAM;
#endif
    } else {
      /* disabled by default for non tied-mixture model */
      jconf->am.gprune_method = GPRUNE_SEL_NONE;
    }
  }
}


/**********************************************************************/
/** 
 * <JA>
 * @brief  全てのモデルを読み込み，認識の準備を行なう．
 *
 * 認識で用いる全てのモデルを読み込む．
 * 
 * </JA>
 * <EN>
 * @brief  Read in all models
 *
 * This function reads in all the models needed for recognition.
 * 
 * </EN>
 */
boolean
j_model_load_all(Model *model, Jconf *jconf)
{
  /* HMM */
  if ((model->hmminfo = initialize_HMM(jconf)) == NULL) {
    jlog("ERROR: m_fusion: failed to initialize AM\n");
    return FALSE;
  }
  if (jconf->am.hmm_gs_filename != NULL) {
    if ((model->hmm_gs = initialize_GSHMM(jconf)) == NULL) {
      jlog("ERROR: m_fusion: failed to initialize GS HMM\n");
      return FALSE;
    }
  }
  if (jconf->reject.gmm_filename != NULL) {
    if ((model->gmm = initialize_GMM(jconf)) == NULL) {
      jlog("ERROR: m_fusion: failed to initialize GMM\n");
      return FALSE;
    }
  }

  /* LM (N-gram) */
  if (jconf->lmtype == LM_PROB) {
    if ((model->winfo = initialize_dict(jconf, model->hmminfo)) == NULL) {
      jlog("ERROR: m_fusion: failed to initialize dictionary\n");
      return FALSE;
    }
    if (jconf->sw.triphone_check_flag && model->hmminfo->is_triphone) {
      /* go into interactive triphone HMM check mode */
      hmm_check(jconf, model->winfo, model->hmminfo);
    }
    if (jconf->lm.ngram_filename_lr_arpa || jconf->lm.ngram_filename_rl_arpa || jconf->lm.ngram_filename) {
      if ((model->ngram = initialize_ngram(jconf, model->winfo)) == NULL) {
	jlog("ERROR: m_fusion: failed to initialize N-gram\n");
	return FALSE;
      }
    }
  }

  /* fixate params */
  /* set params whose default will change by models and not specified in arg */
  configure_param(jconf, model);
  /* 
     gather all the MFCC configuration parameters to form final config.
       preference: Julian option > HTK config > HMM > Julian default
     With HTK config, the default values are overridden to HTK values.
  */
  if (jconf->analysis.para_htk.loaded == 1) apply_para(&(jconf->analysis.para), &(jconf->analysis.para_htk));
  if (jconf->analysis.para_hmm.loaded == 1) apply_para(&(jconf->analysis.para), &(jconf->analysis.para_hmm));
  apply_para(&(jconf->analysis.para), &(jconf->analysis.para_default));

  /* Grammar */
  if (jconf->lmtype == LM_DFA) {
    if (jconf->lm.dfa_filename != NULL && jconf->lm.dictfilename != NULL) {
      /* add grammar specified by "-dfa" and "-v" to grammar list */
      multigram_add_gramlist(jconf->lm.dfa_filename, jconf->lm.dictfilename, jconf, LM_DFA_GRAMMAR);
    }
    /* load all the specified grammars */
    if (multigram_read_all_gramlist(jconf, model) == FALSE) {
      jlog("ERROR: m_fusion: some error occured in reading grammars\n");
      return FALSE;
    }
  }

  model->lmtype = jconf->lmtype;
  model->lmvar  = jconf->lmvar;

  return TRUE;

}

boolean
j_final_fusion(Recog *recog)
{
  Jconf *jconf;
  Model *model;

  jconf = recog->jconf;
  model = recog->model;

  /* set lm type */
  recog->lmtype = model->lmtype;
  recog->lmvar  = model->lmvar;

  /* copy param */
  recog->graphout = jconf->graph.enabled;

  if (model->gmm != NULL) {
    if (gmm_init(recog) == FALSE) {
      jlog("ERROR: m_fusion: error in initializing GMM\n");
      return FALSE;
    }
  }

  if (recog->lmtype == LM_DFA) {
    /* execute generation of global grammar and (re)building of wchmm */
    multigram_exec(recog); /* some modification occured if return TRUE */
  }

  if (recog->lmtype == LM_PROB) {
    /* build wchmm with N-gram */
    recog->wchmm = wchmm_new();
    recog->wchmm->lmtype = recog->lmtype;
    recog->wchmm->lmvar  = recog->lmvar;
    recog->wchmm->category_tree = FALSE;
    recog->wchmm->hmmwrk = &(recog->hmmwrk);
    /* assign models */
    recog->wchmm->ngram = model->ngram;
    if (recog->lmvar == LM_NGRAM_USER) {
      /* register LM functions for 1st pass here */
      recog->wchmm->uni_prob_user = recog->lmfunc.uniprob;
      recog->wchmm->bi_prob_user = recog->lmfunc.biprob;
    }
    recog->wchmm->winfo = model->winfo;
    recog->wchmm->hmminfo = model->hmminfo;
    if (recog->wchmm->category_tree) {
      if (jconf->search.pass1.old_tree_function_flag) {
	if (build_wchmm(recog->wchmm, recog->jconf) == FALSE) {
	  jlog("ERROR: m_fusion: error in bulding wchmm\n");
	  return FALSE;
	}
      } else {
	if (build_wchmm2(recog->wchmm, recog->jconf) == FALSE) {
	  jlog("ERROR: m_fusion: error in bulding wchmm\n");
	  return FALSE;
	}
      }
    } else {
      if (build_wchmm2(recog->wchmm, recog->jconf) == FALSE) {
	jlog("ERROR: m_fusion: error in bulding wchmm\n");
	return FALSE;
      }
    }
    /* set beam width */
    /* guess beam width from models, when not specified */
    recog->trellis_beam_width = set_beam_width(recog->wchmm, jconf->search.pass1.specified_trellis_beam_width);
  }

  /* stage 4: setup output function */
  if (jconf->am.hmm_gs_filename != NULL) {/* with GMS */
    outprob_init(&(recog->hmmwrk), model->hmminfo, model->hmm_gs, jconf->am.gs_statenum, jconf->am.gprune_method, jconf->am.mixnum_thres);
  } else {
    outprob_init(&(recog->hmmwrk), model->hmminfo, NULL, 0, jconf->am.gprune_method, jconf->am.mixnum_thres);
  }

  /* stage 5: initialize work area and misc. */
  /* allocate parameter holder */
  recog->param = new_param();
  /* backtrellis initialization */
  recog->backtrellis = (BACKTRELLIS *)mymalloc(sizeof(BACKTRELLIS));
  bt_init(recog->backtrellis);
  /* initialize cache for factoring */
  if (recog->lmtype == LM_PROB) {
    max_successor_cache_init(recog->wchmm);
  }
  if (jconf->input.speech_input != SP_MFCFILE) {
    /* initialize MFCC calculation work area */
    recog->mfccwrk = WMP_work_new(jconf->analysis.para);
    if (recog->mfccwrk == NULL) {
      jlog("ERROR: m_fusion: failed to initialize MFCC computation\n");
      return -1;
    }
    if (jconf->frontend.sscalc) {
      recog->mfccwrk_ss = WMP_work_new(jconf->analysis.para);
      if (recog->mfccwrk_ss == NULL) {
	jlog("ERROR: m_fusion: failed to initialize MFCC computation for SS\n");
	return -1;
      }
    }
  }
  if (jconf->search.pass1.realtime_flag) {
    /* prepare for 1st pass pipeline processing */
    if (RealTimeInit(recog) == FALSE) {
      jlog("ERROR: m_fusion: failed to initialize recognition process\n");
      return FALSE;
    }
  }

  /* finished! */
  jlog("STAT: All init successfully done\n\n");

  return TRUE;
}

