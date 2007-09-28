/**
 * @file   default.c
 * @author Akinobu Lee
 * @date   Fri Feb 16 15:05:43 2007
 * 
 * <JA>
 * @brief  デフォルト値の設定
 *
 * Julius/Julian の設定可能なパラメータの初期値をセットします．
 * </JA>
 * 
 * <EN>
 * @brief  Set system default values for configuration parameters
 *
 * This file contains a function to set system default values for all the
 * configuration parameters.  This will be called at initialization phase.
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
 * ユーザ設定構造体に初期値を代入する．
 * 
 * @param j [in] ユーザ設定
 * </JA>
 * <EN>
 * Fill in the system default values to a user configuration structure.
 * 
 * @param j [in] user configuration data.
 * </EN>
 */
void
jconf_set_default_values(Jconf *j)
{
  j->input.speech_input			= SP_MFCFILE;
  j->input.use_ds48to16			= FALSE;
  j->input.inputlist_filename		= NULL;
  j->input.adinnet_port			= ADINNET_PORT;
#ifdef USE_NETAUDIO
  j->input.netaudio_devname		= NULL;
#endif

  j->detect.level_thres			= 2000;
  j->detect.head_margin_msec		= 300;
  j->detect.tail_margin_msec		= 400;
  j->detect.zero_cross_num		= 60;
  j->detect.silence_cut			= 2; /* accept device default */

  undef_para(&(j->analysis.para));
  undef_para(&(j->analysis.para_hmm));
  undef_para(&(j->analysis.para_default));
  undef_para(&(j->analysis.para_htk));
  make_default_para(&(j->analysis.para_default));
  make_default_para_htk(&(j->analysis.para_htk));
  j->analysis.paramtype_check_flag	= TRUE;

  j->frontend.cmnload_filename		= NULL;
  j->frontend.cmn_update		= TRUE;
  j->frontend.cmnsave_filename		= NULL;
  j->frontend.cmn_map_weight		= 100.0;
  j->frontend.sscalc			= FALSE;
  j->frontend.sscalc_len		= 300;
  j->frontend.ssload_filename		= NULL;
  j->frontend.strip_zero_sample		= TRUE;
  j->frontend.use_zmean			= FALSE;

  j->am.hmmfilename			= NULL;
  j->am.mapfilename			= NULL;
  j->am.gprune_method			= GPRUNE_SEL_UNDEF;
  j->am.mixnum_thres			= 2;
  j->am.spmodel_name			= NULL;
  j->am.hmm_gs_filename			= NULL;
  j->am.gs_statenum			= 24;
  j->am.ccd_flag_force			= FALSE;
  j->am.force_multipath			= FALSE;

  j->lm.dictfilename			= NULL;

  j->lm.forcedict_flag			= FALSE;

  j->lm.enable_iwsp			= FALSE;
  j->lm.iwsp_penalty			= -1.0;

  /* n-gram related */
  j->lm.head_silname			= NULL;
  j->lm.tail_silname			= NULL;
  strcpy(j->lm.wordrecog_head_silence_model_name, "silB");
  strcpy(j->lm.wordrecog_tail_silence_model_name, "silE");
  j->lm.wordrecog_silence_context_name[0] = '\0';
  j->lm.ngram_filename			= NULL;
  j->lm.ngram_filename_lr_arpa		= NULL;
  j->lm.ngram_filename_rl_arpa		= NULL;
  j->lm.enable_iwspword			= FALSE;
  j->lm.iwspentry			= NULL;
  /* 
    default values below are assigned later using HMM information:
	j->lm.lm_weight
	j->lm.lm_penalty
	j->lm.lm_weight2
	j->lm.lm_penalty2
  */
  j->lm.lm_penalty_trans		= 0.0;

  /* dfa related */
  j->lm.dfa_filename			= NULL;
  j->lm.gramlist_root			= NULL;
  j->lm.wordlist_root			= NULL;
  j->lm.penalty1			= 0.0;
  j->lm.penalty2			= 0.0;

  j->search.pass1.specified_trellis_beam_width = -1;
  j->search.pass1.forced_realtime	= FALSE;
  j->search.pass1.force_realtime_flag	= FALSE;

  j->search.pass1.iwcdmethod		= IWCD_UNDEF;
  j->search.pass1.iwcdmaxn		= 3;
#ifdef SEPARATE_BY_UNIGRAM
  j->search.pass1.separate_wnum		= 150;
#endif
#if defined(WPAIR) && defined(WPAIR_KEEP_NLIMIT)
  j->search.pass1.wpair_keep_nlimit	= 3;
#endif
#ifdef HASH_CACHE_IW
  j->search.pass1.iw_cache_rate		= 10;
#endif
  j->search.pass1.old_tree_function_flag = FALSE;
#ifdef DETERMINE
  j->search.pass1.determine_score_thres = 10.0;
  j->search.pass1.determine_duration_thres = 6;
#endif

  if (strmatch(JULIUS_SETUP, "fast")) {
    j->search.pass2.nbest		= 1;
    j->search.pass2.enveloped_bestfirst_width = 30;
  } else {
    j->search.pass2.nbest		= 10;
    j->search.pass2.enveloped_bestfirst_width = 100;
  }
#ifdef SCAN_BEAM
  j->search.pass2.scan_beam_thres	= 80.0;
#endif
  j->search.pass2.hypo_overflow		= 2000;
  j->search.pass2.stack_size		= 500;
  j->search.pass2.lookup_range		= 5;
  j->search.pass2.looktrellis_flag	= FALSE; /* dfa */

  j->search.sp_segment                  = FALSE;

  j->graph.enabled			= FALSE;
  j->graph.lattice			= FALSE;
  j->graph.confnet			= FALSE;
  j->graph.graph_merge_neighbor_range	= 0;
#ifdef   GRAPHOUT_DEPTHCUT
  j->graph.graphout_cut_depth		= 80;
#endif
#ifdef   GRAPHOUT_LIMIT_BOUNDARY_LOOP
  j->graph.graphout_limit_boundary_loop_num = 20;
#endif
#ifdef   GRAPHOUT_SEARCH_DELAY_TERMINATION
  j->graph.graphout_search_delay	= FALSE;
#endif

#ifdef CONFIDENCE_MEASURE
  j->annotate.cm_alpha			= 0.05;
#ifdef   CM_MULTIPLE_ALPHA
  j->annotate.cm_alpha_bgn		= 0.03;
  j->annotate.cm_alpha_end		= 0.15;
  j->annotate.cm_alpha_num		= 5;
  j->annotate.cm_alpha_step		= 0.03;
#endif
#ifdef   CM_SEARCH_LIMIT
  j->annotate.cm_cut_thres		= 0.03;
#endif
#ifdef   CM_SEARCH_LIMIT_POPO
  j->annotate.cm_cut_thres_pop		= 0.1;
#endif
#endif /* CONFIDENCE_MEASURE */

  j->annotate.align_result_word_flag	= FALSE;
  j->annotate.align_result_phoneme_flag	= FALSE;
  j->annotate.align_result_state_flag	= FALSE;

  j->output.output_hypo_maxnum		= 1;
  j->output.progout_flag		= FALSE;
  j->output.progout_interval		= 300;
  j->output.separate_score_flag		= FALSE; /* n-gram */
  j->output.multigramout_flag		= FALSE; /* dfa */

  j->reject.gmm_filename		= NULL;
  j->reject.gmm_gprune_num		= 10;
  j->reject.gmm_reject_cmn_string	= NULL;
  j->reject.rejectshortlen		= 0;

#ifdef SP_BREAK_CURRENT_FRAME
  j->successive.sp_frame_duration	= 10;
#endif

  j->sw.compute_only_1pass		= FALSE;
  j->sw.trellis_check_flag		= FALSE;
  j->sw.triphone_check_flag		= FALSE;
  j->sw.wchmm_check_flag		= FALSE;

  /* initialize lm switch */
  j->lmtype = LM_UNDEF;
  j->lmvar  = LM_UNDEF;
}
