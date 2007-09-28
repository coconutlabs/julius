/**
 * @file   jfunc.c
 * @author Akinobu Lee
 * @date   Wed Aug  8 15:04:28 2007
 * 
 * <JA>
 * @brief  Julius ライブラリ呼出用関数
 * </JA>
 * 
 * <EN>
 * @brief  JuliusLib interface functions
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

#include <julius/juliuslib.h>

void
j_request_pause(Recog *recog)
{
  /* pause recognition: will stop when the current input ends */
  if (recog->process_active) {
    recog->process_want_terminate = FALSE;
    recog->process_want_reload = TRUE;
    recog->process_active = FALSE;
  }
  if (recog->jconf->input.speech_input == SP_ADINNET) {
    /* when taking speech from adinnet client,
       always tell the client to stop recording */
    adin_tcpip_send_pause();
  }
}

void
j_request_terminate(Recog *recog)
{
  /* terminate recognition: input will terminate immidiately */
  /* set flags to stop adin to terminate immediately, and
     stop process */
  if (recog->process_active) {
    recog->process_want_terminate = TRUE;
    recog->process_want_reload = TRUE;
    recog->process_active = FALSE;
  }
  if (recog->jconf->input.speech_input == SP_ADINNET) {
    /* when taking speech input from adinnet client,
       always tell the client to stop recording immediately */
    adin_tcpip_send_terminate();
  }
}

void 
j_request_resume(Recog *recog)
{
  if (recog->process_active == FALSE) {
    recog->process_want_terminate = FALSE;
    recog->process_active = TRUE;
  }
  if (recog->jconf->input.speech_input == SP_ADINNET) {
    /* when taking speech from adinnet client,
       tell the client to restart recording */
    adin_tcpip_send_resume();
  }
}

void
schedule_grammar_update(Recog *recog)
{
  if (recog->process_active) {
    /* if recognition is currently running, tell engine how/when to
       re-construct global lexicon. */
    switch(recog->gram_switch_input_method) {
    case SM_TERMINATE:	/* discard input now and change (immediate) */
      recog->process_want_terminate = TRUE;
      recog->process_want_reload = TRUE;
      break;
    case SM_PAUSE:		/* segment input now, recognize it, and then change */
      recog->process_want_terminate = FALSE;
      recog->process_want_reload = TRUE;
      break;
    case SM_WAIT:		/* wait until the current input end and recognition completed */
      recog->process_want_terminate = FALSE;
      recog->process_want_reload = FALSE;
      break;
    }
    /* After the update, recognition will restart without sleeping. */
  } else {
    /* If recognition is currently not running, the received
       grammars are merely stored in memory here.  The re-construction of
       global lexicon will be delayed: it will be re-built just before
       the recognition process starts next time. */
  }
}

/** 
 * <JA>
 * recog->process_want_reload フラグをクリアする．
 * 
 * </JA>
 * <EN>
 * Clear the recog->process_want_reload flag.
 * 
 * </EN>
 */
void
j_reset_reload(Recog *recog)
{
  recog->process_want_reload = FALSE;
}

void
j_enable_debug_message()
{
  debug2_flag = TRUE;
}
void
j_disable_debug_message()
{
  debug2_flag = FALSE;
}
void
j_enable_verbose_message()
{
  verbose_flag = TRUE;
}
void
j_disable_verbose_message()
{
  verbose_flag = FALSE;
}
	      

/** 
 * Output error message and exit the program.
 * 
 * @param fmt [in] format string, like printf.
 * @param ... [in] variable length argument like printf.
 */
void
j_internal_error(char *fmt, ...)
{
  va_list ap;
  int ret;

  va_start(ap,fmt);
  ret = vfprintf(stderr, fmt, ap);
  va_end(ap);

  /* clean up socket if already opened */
  cleanup_socket();

  exit(1);
}

Model *
j_model_new()
{
  Model *model;

  /* allocate memory */
  model = (Model *)mymalloc(sizeof(Model));
  memset(model, 0, sizeof(Model));

  /* initialize some values other than 0 (NULL) */
  model->gram_maxid = 0;
  model->lmtype = LM_UNDEF;
  model->lmvar = LM_UNDEF;

  return model;
}

void
j_model_free(Model *model)
{
  if (model->hmminfo) hmminfo_free(model->hmminfo);
  if (model->hmm_gs) hmminfo_free(model->hmm_gs);
  if (model->gmm) hmminfo_free(model->gmm);
  if (model->winfo) word_info_free(model->winfo);
  if (model->ngram) ngram_info_free(model->ngram);
  if (model->dfa) dfa_info_free(model->dfa);
  if (model->grammars) multigram_free_all(model->grammars);
  free(model);
}


Jconf *
j_jconf_new()
{
  Jconf *jconf;

  /* allocate memory */
  jconf = (Jconf *)mymalloc(sizeof(Jconf));

  /* set default values */
  jconf_set_default_values(jconf);

  return(jconf);
}

void
j_jconf_free(Jconf *jconf)
{
  opt_release(jconf);
  free(jconf);
}

Recog *
j_recog_new()
{
  Recog *recog;

  /* allocate memory */
  recog = (Recog *)mymalloc(sizeof(Recog));

  /* clear all values to 0 (NULL)  */
  memset(recog, 0, sizeof(Recog));

  /* initialize some values */
  recog->pass1.nodes_malloced = FALSE;
  recog->process_online = FALSE;
  recog->process_active = TRUE;
  recog->process_want_terminate = FALSE;
  recog->process_want_reload = FALSE;
  recog->gram_switch_input_method = SM_PAUSE;
#ifdef SP_BREAK_CURRENT_FRAME
  recog->process_segment = FALSE;
#endif
  recog->lmtype = LM_UNDEF;
  recog->lmvar = LM_UNDEF;

  /* set default function for vector calculation to RealTimeMFCC() */
  recog->calc_vector = RealTimeMFCC;

  /* clear callback func. */
  callback_init(recog);

  recog->adin = (ADIn *)mymalloc(sizeof(ADIn));
  memset(recog->adin, 0, sizeof(ADIn));

  return(recog);
}

void
j_recog_free(Recog *recog)
{
  /* free wchmm */
  if (recog->wchmm) wchmm_free(recog->wchmm);
  /* free backtrellis */
  if (recog->backtrellis) bt_free(recog->backtrellis);
  /* free adin work area */
  adin_free_param(recog);
  /* free GMM calculation work area if any */
  gmm_free(recog);


  /* free param */
  if (recog->mfccwrk) WMP_free(recog->mfccwrk);
  if (recog->mfccwrk_ss) WMP_free(recog->mfccwrk_ss);
  if (recog->param) free_param(recog->param);
  if (recog->ssbuf) free(recog->ssbuf);

#ifdef SP_BREAK_CURRENT_FRAME
  /* rest_param */
  if (recog->rest_param) free_param(recog->rest_param);
#endif

  /* Output result -> free just after malloced and used */
  /* StackDecode pass2 -> allocate and free within search */

  /* RealBeam real */
  realbeam_free(recog);

  /* FSBeam pass1 -> allocate and free within 1st pass */
  /* HMMWork hmmwrk */
  outprob_free(&(recog->hmmwrk));

  /* adin */
  if (recog->adin) free(recog->adin);

  /* jconf */
  if (recog->jconf) {
    j_jconf_free(recog->jconf);
  }
  /* model */
  if (recog->model) {
    j_model_free(recog->model);
  }

  free(recog);
}

int
j_config_load_args(Jconf *jconf, int argc, char *argv[])
{
  /* parse options and set variables */
  if (opt_parse(argc, argv, NULL, jconf) == FALSE) {
    return -1;
  }
  return 0;
}

int
j_config_load_file(Jconf *jconf, char *filename)
{
  /* parse options and set variables */
  if (config_file_parse(filename, jconf) == FALSE) {
    return -1;
  }
  return 0;
}

Jconf *
j_config_load_args_new(int argc, char *argv[])
{
  Jconf *j;

  j = j_jconf_new();
  /* parse options and set variables */
  if (opt_parse(argc, argv, NULL, j) == FALSE) {
    j_jconf_free(j);
    return NULL;
  }
  return j;
}

Jconf *
j_config_load_file_new(char *filename)
{
  Jconf *j;

  j = j_jconf_new();
  /* parse options and set variables */
  if (config_file_parse(filename, j) == FALSE) {
    j_jconf_free(j);
    return NULL;
  }
  return j;
}

boolean
j_adin_init(Recog *recog)
{
  boolean ret;

  ret = adin_initialize(recog);

    /* initialize A/D-in device */
    /* create A/D-in thread here */
#ifdef HAVE_PTHREAD
  if (ret) {
    if (recog->adin->enable_thread) {
      if (adin_thread_create(&recog, 1) == FALSE) {
	return FALSE;
      }
    }
  }
#endif

  return(ret);
}
boolean
j_adin_init_multi(Recog **recoglist, int recognum)
{
  boolean ret;

  ret = adin_initialize(recoglist[0]);

    /* initialize A/D-in device */
    /* create A/D-in thread here */
#ifdef HAVE_PTHREAD
  if (ret) {
    if (recoglist[0]->adin->enable_thread) {
      if (adin_thread_create(recoglist, recognum) == FALSE) {
	return FALSE;
      }
    }
  }
#endif

  return(ret);
}

boolean
j_adin_init_user(Recog *recog, void *arg)
{
  boolean ret;

  ret = adin_initialize_user(recog, arg);

    /* initialize A/D-in device */
    /* create A/D-in thread here */
#ifdef HAVE_PTHREAD
  if (ret) {
    if (recog->adin->enable_thread) {
      if (adin_thread_create(&recog, 1) == FALSE) {
	return FALSE;
      }
    }
  }
#endif
  return(ret);
}
boolean
j_adin_init_user_multi(Recog **recoglist, int recognum, void *arg)
{
  boolean ret;

  ret = adin_initialize_user(recoglist[0], arg);

    /* initialize A/D-in device */
    /* create A/D-in thread here */
#ifdef HAVE_PTHREAD
  if (ret) {
    if (recoglist[0]->adin->enable_thread) {
      if (adin_thread_create(recoglist, recognum) == FALSE) {
	return FALSE;
      }
    }
  }
#endif

  return(ret);
}

void
j_recog_info(Recog *recog)
{
  /* print out system information */
  print_info(recog);
}

Recog *
j_create_instance_from_jconf(Jconf *jconf)
{
  Model *model;
  Recog *recog;

  /* check option values and set parameters needed for model loading */
  if (j_jconf_finalize(jconf) == FALSE) {
    return NULL;
  }

  /* create a model container */
  model = j_model_new();
  /* load all models into memory according to the configurations */
  if (j_model_load_all(model, jconf) == FALSE) {
    jlog("ERROR: Error in loading model\n");
    /* j_model_free(model); */
    return NULL;
  }
  /* create a recognition instance */
  recog = j_recog_new();
  /* assign model to the instance */
  recog->model = model;
  /* assign configuration to the instance */
  recog->jconf = jconf;
  /* checkout for recognition: build lexicon tree, allocate cache */
  if (j_final_fusion(recog) == FALSE) {
    jlog("ERROR: Error while setup work area for recognition\n");
    j_recog_free(recog);
    return NULL;
  }

  return recog;
}

/* this should be called after recog instance creation and before
   j_final_fusion() is called.  You should also specify "-userlm"
   option at jconf. */
boolean
j_regist_user_lm_func(Recog *recog, 
	  LOGPROB (*unifunc)(WORD_INFO *winfo, WORD_ID w, LOGPROB ngram_prob), 
	  LOGPROB (*bifunc)(WORD_INFO *winfo, WORD_ID context, WORD_ID w, LOGPROB ngram_prob),
	  LOGPROB (*probfunc)(WORD_INFO *winfo, WORD_ID *contexts, int context_len, WORD_ID w, LOGPROB ngram_prob))
{
  recog->lmfunc.uniprob = unifunc;
  recog->lmfunc.biprob = bifunc;
  recog->lmfunc.lmprob = probfunc;
}

boolean
j_regist_user_param_func(Recog *recog, boolean (*user_calc_vector)(VECT *, SP16 *, int, Value *, Recog *))
{
  recog->calc_vector = user_calc_vector;
}
