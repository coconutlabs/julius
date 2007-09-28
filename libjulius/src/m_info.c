/**
 * @file   m_info.c
 * @author Akinobu Lee
 * @date   Thu May 12 14:14:01 2005
 * 
 * <JA>
 * @brief  起動時に認識システムの全情報を出力する．
 * </JA>
 * 
 * <EN>
 * @brief  Output all information of recognition system to standard out.
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
 * 読み込んだモデルファイル名を出力する．
 * 
 * </JA>
 * <EN>
 * Output all file names of the models.
 * 
 * </EN>
 */
void
print_setting(Jconf *jconf)
{
#ifdef USE_NETAUDIO
  char *p;
#endif
  GRAMLIST *g;
  int n;
  
  jlog("    hmmfilename=%s\n",jconf->am.hmmfilename);
  if (jconf->am.mapfilename != NULL) {
    jlog("    hmmmapfilename=%s\n",jconf->am.mapfilename);
  }

  if (jconf->lmtype == LM_PROB) {
    jlog("    vocabulary filename=%s\n",jconf->lm.dictfilename);
    if (jconf->lm.ngram_filename != NULL) {
      jlog("    n-gram  filename=%s (binary format)\n",jconf->lm.ngram_filename);
    } else {
      if (jconf->lm.ngram_filename_rl_arpa != NULL) {
	jlog("    backward n-gram filename=%s\n",jconf->lm.ngram_filename_rl_arpa);
	if (jconf->lm.ngram_filename_lr_arpa != NULL) {
	  jlog("    forward 2-gram for pass1=%s\n",jconf->lm.ngram_filename_lr_arpa);
	}
      } else if (jconf->lm.ngram_filename_lr_arpa != NULL) {
	jlog("    forward n-gram filename=%s\n",jconf->lm.ngram_filename_lr_arpa);
      }
    }
  }

  if (jconf->lmtype == LM_DFA) {
    switch(jconf->lmvar) {
    case LM_DFA_GRAMMAR:
      n = 1;
      for(g = jconf->lm.gramlist_root; g; g = g->next) {
	jlog("    grammar #%d:\n", n++);
	jlog("        dfa  = %s\n", g->dfafile);
	jlog("        dict = %s\n", g->dictfile);
      }
      break;
    case LM_DFA_WORD:
      n = 1;
      for(g = jconf->lm.wordlist_root; g; g = g->next) {
	jlog("    wordlist #%d: %s\n", n++, g->dictfile);
      }
      break;
    }
  }

  if (jconf->am.hmm_gs_filename != NULL) {
    jlog("    hmmfile for Gaussian Selection: %s\n", jconf->am.hmm_gs_filename);
  }
  if (jconf->reject.gmm_filename != NULL) {
    jlog("    GMM file for utterance verification: %s\n", jconf->reject.gmm_filename);
  }
}

/** 
 * <JA>
 * 全てのシステム情報を出力する．
 * 
 * </JA>
 * <EN>
 * Output full system information.
 * 
 * </EN>
 */
void
print_info(Recog *recog)
{
  Jconf *jconf;
  Model *model;
  FILE *fp;

  fp = jlog_get_fp();
  if (fp == NULL) return;


  jconf = recog->jconf;
  model = recog->model;

  jlog("------------- System Info begin -------------\n");
  j_put_header(fp);
  if (verbose_flag) {
    j_put_compile_defs(fp);
    jlog("\n");
    if (recog->lmtype == LM_PROB) {
      switch(recog->lmvar) {
      case LM_NGRAM:
	jlog("Large Vocabulary Continuous Speech Recognition based on N-gram\n\n");
	break;
      case LM_NGRAM_USER:
	jlog("Large Vocabulary Continuous Speech Recognition using user-supplied LC\n\n");
	break;
      }
    } else if (recog->lmtype == LM_DFA) {
      switch(recog->lmvar) {
      case LM_DFA_GRAMMAR:
	jlog("Continuous Speech Recognition Parser based on automaton grammar\n\n");
	break;
      case LM_DFA_WORD:
	jlog("Continuous Speech Recognition Parser for Isolated Word Recognition\n\n");
	break;
      }
    }
  }
  
  /* print current argument setting to log */
  jlog("Files:\n");
  print_setting(jconf);
  jlog("\n");

  /* for backward compatibility with scoring tool (IPA99)... :-( */
  if (jconf->input.speech_input == SP_RAWFILE) {
    jlog("Speech input source: file\n\n");
  } else if (jconf->input.speech_input == SP_MFCFILE) {
    jlog("Speech input source: MFCC parameter file (HTK format)\n\n");
  }

  if (jconf->input.speech_input != SP_MFCFILE) {

    put_para(fp, &jconf->analysis.para);

    jlog("\t base setup from =");
    if (jconf->analysis.para_htk.loaded == 1 || jconf->analysis.para_hmm.loaded == 1) {
      if (jconf->analysis.para_hmm.loaded == 1) {
	jlog(" binhmm-embedded");
	if (jconf->analysis.para_htk.loaded == 1) {
	  jlog(", then overridden by HTK Config and defaults");
	}
      } else {
	if (jconf->analysis.para_htk.loaded == 1) {
	  jlog(" HTK Config (and HTK defaults)");
	}
      }
    } else {
      jlog(" Julius defaults");
    }
    jlog("\n");


    jlog("    spectral subtraction = ");
    if (jconf->frontend.ssload_filename || jconf->frontend.sscalc) {
      if (jconf->frontend.sscalc) {
	jlog("use head silence of each input\n");
	jlog("\t head sil length = %d msec\n", jconf->frontend.sscalc_len);
      } else {			/* ssload_filename != NULL */
	jlog("use a constant value from file\n");
	jlog("     noise spectrum file = \"%s\"\n", jconf->frontend.ssload_filename);
      }
      jlog("\t     alpha coef. = %f\n", jconf->analysis.para.ss_alpha);
      jlog("\t  spectral floor = %f\n", jconf->analysis.para.ss_floor);
    } else {
      jlog("off\n");
    }
    jlog("\n");
  }
    
  print_hmmdef_info(fp, model->hmminfo); jlog("\n");
  if (jconf->am.hmm_gs_filename != NULL) {
    jlog("GS ");
    print_hmmdef_info(fp, model->hmm_gs); jlog("\n");
  }
  if (model->winfo != NULL) {
    print_voca_info(fp, model->winfo); jlog("\n");
  }
  if (recog->wchmm != NULL) {
    print_wchmm_info(recog->wchmm); jlog("\n");
  }

  if (recog->lmtype == LM_PROB) {
    print_ngram_info(fp, model->ngram); jlog("\n");
  } else if (recog->lmtype == LM_DFA && recog->lmvar == LM_DFA_GRAMMAR) {
    if (model->dfa != NULL) {
      print_dfa_info(fp, model->dfa); jlog("\n");
      if (debug2_flag) print_dfa_cp(fp, model->dfa); jlog("\n");
    }
  }

  if (recog->lmtype == LM_PROB) {
    jlog("Inter-word N-gram cache: \n");
    {
      int num, len;
#ifdef UNIGRAM_FACTORING
      len = recog->wchmm->isolatenum;
      jlog("\troot node to be cached = %d / %d (isolated only)\n",
	       len, recog->wchmm->startnum);
#else
      len = recog->wchmm->startnum;
      jlog("\troot node to be cached = %d (all)\n", len);
#endif
#ifdef HASH_CACHE_IW
      num = (jconf->search.pass1.iw_cache_rate * ngram->max_word_num) / 100;
      jlog("\tword ends to be cached = %d / %d\n", num, ngram->max_word_num);
#else
      num = model->ngram->max_word_num;
      jlog("\tword ends to be cached = %d (all)\n", num);
#endif
      jlog("\t  max. allocation size = %dMB\n", num * len / 1000 * sizeof(LOGPROB) / 1000);
    }
  }

  jlog("\nLikelihood weights and special words: \n");

  if (recog->lmtype == LM_PROB) {
    jlog("\t(-lmp)  pass1 LM weight = %2.1f  ins. penalty = %+2.1f\n", jconf->lm.lm_weight, jconf->lm.lm_penalty);
    jlog("\t(-lmp2) pass2 LM weight = %2.1f  ins. penalty = %+2.1f\n", jconf->lm.lm_weight2, jconf->lm.lm_penalty2);
    jlog("\t(-transp)trans. penalty = %+2.1f per word\n", jconf->lm.lm_penalty_trans);
    jlog("\t(-silhead)head sil word = ");
    put_voca(fp, model->winfo, model->winfo->head_silwid);
    jlog("\t(-siltail)tail sil word = ");
    put_voca(fp, model->winfo, model->winfo->tail_silwid);
  } else if (recog->lmtype == LM_DFA && recog->lmvar == LM_DFA_GRAMMAR) {
    jlog("\t(-penalty1) IW penalty1 = %+2.1f\n", jconf->lm.penalty1);
    jlog("\t(-penalty2) IW penalty2 = %+2.1f\n", jconf->lm.penalty2);
  }

#ifdef CONFIDENCE_MEASURE
#ifdef CM_MULTIPLE_ALPHA
  jlog("\t(-cmalpha)CM alpha coef = from %f to %f by step of %f (%d outputs)\n", jconf->annotate.cm_alpha_bgn, jconf->annotate.cm_alpha_end, jconf->annotate.cm_alpha_step, jconf->annotate.cm_alpha_num);
#else
  jlog("\t(-cmalpha)CM alpha coef = %f\n", jconf->annotate.cm_alpha);
#endif
#ifdef CM_SEARCH_LIMIT
  jlog("\t(-cmthres) CM cut thres = %f for hypo generation\n", jconf->annotate.cm_cut_thres);
#endif
#ifdef CM_SEARCH_LIMIT_POP
  jlog("\t(-cmthres2)CM cut thres = %f for popped hypo\n", jconf->annotate.cm_cut_thres_pop);
#endif
#endif /* CONFIDENCE_MEASURE */
  jlog("\t(-sp)shortpause HMM name= \"%s\" specified", jconf->am.spmodel_name);
  if (model->hmminfo->sp != NULL) {
    jlog(", \"%s\" applied", model->hmminfo->sp->name);
    if (model->hmminfo->sp->is_pseudo) {
      jlog(" (pseudo)");
    } else {
      jlog(" (physical)");
    }
  }
  jlog("\n");

  if (recog->lmtype == LM_DFA && recog->lmvar == LM_DFA_GRAMMAR) {
    if (model->dfa != NULL) {
      int i;
      jlog("\t  found sp category IDs =");
      for(i=0;i<model->dfa->term_num;i++) {
	if (model->dfa->is_sp[i]) {
	  jlog(" %d", i);
	}
      }
      jlog("\n");
    }
  }

  if (model->hmminfo->multipath) {
    if (jconf->lm.enable_iwsp) {
      jlog("\t inter-word short pause = on (append \"%s\" for each word tail)\n", model->hmminfo->sp->name);
      jlog("\t  sp transition penalty = %+2.1f\n", jconf->lm.iwsp_penalty);
    }
  }

  if (recog->lmtype == LM_PROB) {
    if (jconf->lm.enable_iwspword) {
      jlog("\tIW-sp word added to dict= \"%s\"\n", jconf->lm.iwspentry);
    }
  }

  if (recog->lmvar == LM_DFA_WORD) {
    jlog("\nIsolated Word Recognition:\n");
    jlog("    silence model names to add at word head / tail:  (-wsil)\n");
    jlog("\tword head          = \"%s\"\n", jconf->lm.wordrecog_head_silence_model_name);
    jlog("\tword tail          = \"%s\"\n", jconf->lm.wordrecog_tail_silence_model_name);
    jlog("\ttheir context name = \"%s\"\n", (jconf->lm.wordrecog_silence_context_name[0] == '\0') ? "NULL (blank)" : jconf->lm.wordrecog_silence_context_name);
#ifdef DETERMINE
    jlog("    early word determination:  (-wed)\n");
    jlog("\tscore threshold    = %f\n", jconf->search.pass1.determine_score_thres);
    jlog("\tframe dur. thres   = %d\n", jconf->search.pass1.determine_duration_thres);
#endif
  }
  
  if (model->gmm != NULL) {
    jlog("\n");
    jlog("Utterance verification by GMM\n");
    jlog("           GMM defs file = %s\n", jconf->reject.gmm_filename);
    jlog("          GMM gprune num = %d\n", jconf->reject.gmm_gprune_num);
    if (jconf->reject.gmm_reject_cmn_string != NULL) {
      jlog("     GMM names to reject = %s\n", jconf->reject.gmm_reject_cmn_string);
    }
    jlog("    GMM ");
    print_hmmdef_info(fp, model->gmm);
  }

  if (jconf->search.pass1.realtime_flag && jconf->analysis.para.cmn) {
    jlog("\n");
    jlog("MAP-CMN on realtime input: \n");
    if (jconf->frontend.cmnload_filename) {
      if (recog->cmn_loaded) {
	jlog("\t      initial CMN param = from \"%s\"\n", jconf->frontend.cmnload_filename);
      } else {
	jlog("\t      initial CMN param = from \"%s\" (failed, ignored)\n", jconf->frontend.cmnload_filename);
      }
    } else {
      jlog("\t      initial CMN param = not specified\n");
    }
    jlog("\t    initial mean weight = %6.2f\n", jconf->frontend.cmn_map_weight);
    if (jconf->frontend.cmn_update) {
      jlog("\t       CMN param update = yes, update from last inputs\n");
    } else {
      jlog("\t       CMN param update = no, keep initial\n");
    }
    if (jconf->frontend.cmnsave_filename) {
      if (jconf->search.pass1.realtime_flag) {
	jlog("\t      save CMN param to = %s\n", jconf->frontend.cmnsave_filename);
      } else {
	jlog("\t      save CMN param to = %s (not realtime CMN, ignored)\n", jconf->frontend.cmnsave_filename);
      }
    }
  }

  jlog("\n");
  jlog("Search parameters: \n");

  jlog("\t      1st pass decoding = ");
  if (jconf->search.pass1.force_realtime_flag) jlog("(forced) ");
  if (jconf->search.pass1.realtime_flag) {
    jlog("on-the-fly");
    if (jconf->input.speech_input != SP_MFCFILE && jconf->analysis.para.cmn) jlog(" with MAP-CMN");
    jlog("\n");
  } else {
    jlog("batch");
    if (jconf->input.speech_input != SP_MFCFILE && jconf->analysis.para.cmn) jlog(" with sentence CMN");
    jlog("\n");
  }
  jlog("\t        1st pass method = ");
#ifdef WPAIR
# ifdef WPAIR_KEEP_NLIMIT
  jlog("word-pair approx., keeping only N tokens ");
# else
  jlog("word-pair approx. ");
# endif
#else
  jlog("1-best approx. ");
#endif
#ifdef WORD_GRAPH
  jlog("generating word_graph\n");
#else
  jlog("generating indexed trellis\n");
#endif

  jlog("\t    multi-path handling = ");
  if (recog->model->hmminfo->multipath) {
    jlog("yes, multi-path mode enabled\n");
  } else {
    jlog("no\n");
  }

  jlog("\t(-b) trellis beam width = %d", recog->trellis_beam_width);
  if (jconf->search.pass1.specified_trellis_beam_width == -1) {
    jlog(" (-1 or not specified - guessed)\n");
  } else if (jconf->search.pass1.specified_trellis_beam_width == 0) {
    jlog(" (0 - full)\n");
  } else {
    jlog("\n");
  }
  jlog("\t(-n)search candidate num= %d\n", jconf->search.pass2.nbest);
  jlog("\t(-s)  search stack size = %d\n", jconf->search.pass2.stack_size);
  jlog("\t(-m)    search overflow = after %d hypothesis poped\n", jconf->search.pass2.hypo_overflow);
  jlog("\t        2nd pass method = ");
  if (jconf->graph.enabled) {
#ifdef GRAPHOUT_DYNAMIC
#ifdef GRAPHOUT_SEARCH
    jlog("searching graph, generating dynamic graph\n");
#else
    jlog("searching sentence, generating dynamic graph\n");
#endif /* GRAPHOUT_SEARCH */
#else  /* ~GRAPHOUT_DYNAMIC */
    jlog("searching sentence, generating static graph from N-best\n");
#endif
  } else {
    jlog("searching sentence, generating N-best\n");
  }
  if (jconf->search.pass2.enveloped_bestfirst_width >= 0) {
    jlog("\t(-b2)  pass2 beam width = %d\n", jconf->search.pass2.enveloped_bestfirst_width);
  }
  jlog("\t(-lookuprange)lookup range= %d  (tm-%d <= t <tm+%d)\n",jconf->search.pass2.lookup_range,jconf->search.pass2.lookup_range,jconf->search.pass2.lookup_range);
#ifdef SCAN_BEAM
  jlog("\t(-sb)2nd scan beamthres = %.1f (in logscore)\n",jconf->search.pass2.scan_beam_thres);
#endif
  jlog("\t(-gprune)Gauss. pruning = ");
  switch(jconf->am.gprune_method){
  case GPRUNE_SEL_NONE: jlog("none (full computation)\n"); break;
  case GPRUNE_SEL_BEAM: jlog("beam\n"); break;
  case GPRUNE_SEL_HEURISTIC: jlog("heuristic\n"); break;
  case GPRUNE_SEL_SAFE: jlog("safe\n"); break;
  }
  if (jconf->am.gprune_method != GPRUNE_SEL_NONE) {
    jlog("\t(-tmix)   mixture thres = %d / %d\n", jconf->am.mixnum_thres, model->hmminfo->maxcodebooksize);
  }
  if (jconf->am.hmm_gs_filename != NULL) {
    jlog("\t(-gsnum)   GS state num = %d / %d selected\n", jconf->am.gs_statenum, model->hmm_gs->totalstatenum);
  }

  jlog("\t(-n)        search till = %d candidates found\n", jconf->search.pass2.nbest);
  jlog("\t(-output)    and output = %d candidates out of above\n", jconf->output.output_hypo_maxnum);
  if (jconf->am.ccd_flag) {
    jlog("\t IWCD handling:\n");
#ifdef PASS1_IWCD
    jlog("\t   1st pass: approximation ");
    switch(model->hmminfo->cdset_method) {
    case IWCD_AVG:
      jlog("(use average prob. of same LC)\n");
      break;
    case IWCD_MAX:
      jlog("(use max. prob. of same LC)\n");
      break;
    case IWCD_NBEST:
      jlog("(use %d-best of same LC)\n", model->hmminfo->cdmax_num);
      break;
    }
#else
    jlog("\t   1st pass: ignored\n");
#endif
#ifdef PASS2_STRICT_IWCD
    jlog("\t   2nd pass: strict (apply when expanding hypo. )\n");
#else
    jlog("\t   2nd pass: loose (apply when hypo. is popped and scanned)\n");
#endif
  }
  
  if (recog->lmtype == LM_PROB) {
    jlog("\t factoring score: ");
#ifdef UNIGRAM_FACTORING
    jlog("1-gram prob. (statically assigned beforehand)\n");
#else
    jlog("2-gram prob. (dynamically computed while search)\n");
#endif
  }

  if (jconf->annotate.align_result_word_flag) {
    jlog("\t output word alignments\n");
  }
  if (jconf->annotate.align_result_phoneme_flag) {
    jlog("\t output phoneme alignments\n");
  }
  if (jconf->annotate.align_result_state_flag) {
    jlog("\t output state alignments\n");
  }

  if (recog->lmtype == LM_DFA && recog->lmvar == LM_DFA_GRAMMAR) {
    if (jconf->search.pass2.looktrellis_flag) {
      jlog("\t only words in backtrellis will be expanded in 2nd pass\n");
    } else {
      jlog("\t all possible words will be expanded in 2nd pass\n");
    }
  }

  if (recog->wchmm->category_tree) {
    if (jconf->search.pass1.old_tree_function_flag) {
      jlog("\t build_wchmm() used\n");
    } else {
      jlog("\t build_wchmm2() used\n");
    }
#ifdef PASS1_IWCD
#ifdef USE_OLD_IWCD
    jlog("\t full lcdset used\n");
#else
    jlog("\t lcdset limited by word-pair constraint\n");
#endif
#endif /* PASS1_IWCD */
  }
  if (jconf->output.progout_flag) jlog("\tprogressive output on 1st pass\n");
  /* if (param_kind != NULL) {
    jlog("Selectively use input parameter vector as: %s\n", param_kind);
  } */
  if (jconf->sw.compute_only_1pass) {
    jlog("\tCompute only 1-pass\n");
  }
#ifdef CONFIDENCE_MEASURE
  jlog("\t output word confidence measure ");
#ifdef CM_NBEST
  jlog("based on N-best candidates\n");
#endif
#ifdef CM_SEARCH
  jlog("based on search-time scores\n");
#endif
#endif /* CONFIDENCE_MEASURE */
  
  if (jconf->graph.enabled) {
    jlog("\n");
    jlog("Graph-based output with graph-oriented search:\n");
    jlog("\t(-lattice)      word lattice = %s\n", jconf->graph.lattice ? "yes" : "no");
    jlog("\t(-confnet) confusion network = %s\n", jconf->graph.confnet ? "yes" : "no");
    if (jconf->graph.lattice == TRUE) {
      jlog("\t(-graphrange)         margin = %d frames", jconf->graph.graph_merge_neighbor_range);
      if (jconf->graph.graph_merge_neighbor_range < 0) {
	jlog(" (all post-marging disabled)\n");
      } else if (jconf->graph.graph_merge_neighbor_range == 0) {
	jlog(" (merge same word with the same boundary)\n");
      } else {
	jlog(" (merge same words around this margin)\n");
      }
    }
#ifdef GRAPHOUT_DEPTHCUT
    jlog("\t(-graphcut)cutoff depth      = ");
    if (jconf->graph.graphout_cut_depth < 0) {
      jlog("disabled (-1)\n");
    } else {
      jlog("%d words\n",jconf->graph.graphout_cut_depth);
    }
#endif
#ifdef GRAPHOUT_LIMIT_BOUNDARY_LOOP
    jlog("\t(-graphboundloop)loopmax     = %d for boundary adjustment\n",jconf->graph.graphout_limit_boundary_loop_num);
#endif
#ifdef GRAPHOUT_SEARCH_DELAY_TERMINATION
    jlog("\tInhibit graph search termination before 1st sentence found = ");
    if (jconf->graph.graphout_search_delay) {
      jlog("enabled\n");
    } else {
      jlog("disabled\n");
    }
#endif
  }
  
  jlog("\n");
  jlog("System I/O configuration:\n");
  jlog("\t    speech input source = ");
  if (jconf->input.speech_input == SP_RAWFILE) {
    jlog("speech file\n");
    jlog("\t          input filelist = ");
    if (jconf->input.inputlist_filename == NULL) {
      jlog("(none, enter filenames from stdin)\n");
    } else {
      jlog("%s\n", jconf->input.inputlist_filename);
    }
  } else if (jconf->input.speech_input == SP_MFCFILE) {
    jlog("MFCC parameter file (HTK format)\n");
    jlog("\t                filelist = ");
    if (jconf->input.inputlist_filename == NULL) {
      jlog("(none, enter filenames from stdin)\n");
    } else {
      jlog("%s\n", jconf->input.inputlist_filename);
    }
  } else if (jconf->input.speech_input == SP_STDIN) {
    jlog("standard input\n");
  } else if (jconf->input.speech_input == SP_ADINNET) {
    jlog("adinnet client\n");
#ifdef USE_NETAUDIO
  } else if (jconf->input.speech_input == SP_NETAUDIO) {
    char *p;
    jlog("NetAudio server on ");
    if (jconf->input.netaudio_devname != NULL) {
      jlog("%s\n", jconf->input.netaudio_devname);
    } else if ((p = getenv("AUDIO_DEVICE")) != NULL) {
      jlog("%s\n", p);
    } else {
      jlog("local port\n");
    }
#endif
  } else if (jconf->input.speech_input == SP_MIC) {
    jlog("microphone\n");
  }
  if (jconf->input.speech_input != SP_MFCFILE) {
    if (jconf->input.speech_input == SP_RAWFILE || jconf->input.speech_input == SP_STDIN || jconf->input.speech_input == SP_ADINNET) {
      if (jconf->input.use_ds48to16) {
	jlog("\t          sampling freq. = assume 48000Hz, then down to %dHz\n", jconf->analysis.para.smp_freq);
      } else {
	jlog("\t          sampling freq. = %d Hz required\n", jconf->analysis.para.smp_freq);
      }
    } else {
      if (jconf->input.use_ds48to16) {
	jlog("\t          sampling freq. = 48000Hz, then down to %d Hz\n", jconf->analysis.para.smp_freq);
      } else {
 	jlog("\t          sampling freq. = %d Hz\n", jconf->analysis.para.smp_freq);
      }
    }
  }
  if (jconf->input.speech_input != SP_MFCFILE) {
    jlog("\t         threaded A/D-in = ");
#ifdef HAVE_PTHREAD
    if (recog->adin->enable_thread) {
      jlog("supported, on\n");
    } else {
      jlog("supported, off\n");
    }
#else
    jlog("not supported (live input may be dropped)\n");
#endif
  }
  if (jconf->frontend.strip_zero_sample) {
    jlog("\t   zero frames stripping = on\n");
  } else {
    jlog("\t   zero frames stripping = off\n");
  }
  if (jconf->input.speech_input != SP_MFCFILE) {
    if (recog->adin->adin_cut_on) {
      jlog("\t         silence cutting = on\n");
      jlog("\t             level thres = %d / 32767\n", jconf->detect.level_thres);
      jlog("\t         zerocross thres = %d / sec.\n", jconf->detect.zero_cross_num);
      jlog("\t             head margin = %d msec.\n", jconf->detect.head_margin_msec);
      jlog("\t             tail margin = %d msec.\n", jconf->detect.tail_margin_msec);
    } else {
      jlog("\t         silence cutting = off\n");
    }
    if (jconf->frontend.use_zmean || jconf->analysis.para.zmeanframe) {
      jlog("\t        remove DC offset = on");
      if (jconf->analysis.para.zmeanframe) {
	jlog(" (frame-wise)\n");
      }
      if (jconf->input.speech_input == SP_RAWFILE) {
	jlog(" (will compute for each file)\n");
      } else {
	jlog(" (will compute from first %.1f sec)\n",
		 (float)ZMEANSAMPLES / (float)jconf->analysis.para.smp_freq);
      }
    } else {
      jlog("\t        remove DC offset = off\n");
    }
  }
  jlog("\t      reject short input = ");
  if (jconf->reject.rejectshortlen > 0) {
    jlog("< %d msec\n", jconf->reject.rejectshortlen);
  } else {
    jlog("off\n");
  }
#ifdef SP_BREAK_CURRENT_FRAME
  jlog("\tshort pause segmentation = on\n");
  jlog("\t      sp duration length = %d frames\n", jconf->successive.sp_frame_duration);
#else
  jlog("\tshort pause segmentation = off\n");
#endif
  if (jconf->output.progout_flag) {
    jlog("\t        progout interval = %d msec\n", jconf->output.progout_interval);
  }
  jlog("\n");
  jlog("------------- System Info end -------------\n");

#ifdef USE_MIC
  if (jconf->search.pass1.realtime_flag) {
    if (jconf->analysis.para.cmn) {
      if (recog->cmn_loaded) {
	jlog("\n");
	jlog("initial CMN parameter loaded from file\n");
      } else {
	jlog("\n");
	jlog("\t*************************************************************\n");
	jlog("\t* NOTICE: The first input may not be recognized, since      *\n");
	jlog("\t*         no initial CMN parameter is available on startup. *\n");
	jlog("\t*************************************************************\n");
      }
    }
    if (jconf->analysis.para.energy && jconf->analysis.para.enormal) {
      jlog("\t*************************************************************\n");
      jlog("\t* NOTICE: Energy normalization is activated on live input:  *\n");
      jlog("\t*         maximum energy of LAST INPUT will be used for it. *\n");
      jlog("\t*         So, the first input will not be recognized.       *\n");
      jlog("\t*************************************************************\n");
    }
  }
#endif
}
