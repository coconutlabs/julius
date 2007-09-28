/**
 * @file   main.c
 * @author Akinobu Lee
 * @date   Wed May 18 15:02:55 2005
 * 
 * <JA>
 * @brief  Julius/Julian メイン
 * </JA>
 * 
 * <EN>
 * @brief  Main function of Julius/Julian
 * </EN>
 * 
 * $Revision: 1.1 $
 * 
 */
/*
 * Copyright (c) 1991-2006 Kawahara Lab., Kyoto University
 * Copyright (c) 1997-2000 Information-technology Promotion Agency, Japan
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2006 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#include "app.h"

/**
 * Define to allow multiple jconf
 * 
 */
#undef MULTI

#define MULTI_N 10


#undef USER_LM_TEST

#ifdef USER_LM_TEST

static LOGPROB
my_uni(WORD_INFO *winfo, WORD_ID w, LOGPROB ngram_prob)
{
  //printf("[%s] %f -> %f\n", winfo->woutput[w], ngram_prob, ngram_prob);
  if (strmatch(winfo->woutput[w], "年寄り")) {
    ngram_prob += 2.0;
  }
  return ngram_prob;
}

static LOGPROB
my_bi(WORD_INFO *winfo, WORD_ID context, WORD_ID w, LOGPROB ngram_prob)
{
  //printf("[%s|%s] %f -> %f\n", winfo->woutput[context], winfo->woutput[w], ngram_prob, ngram_prob);
  if (strmatch(winfo->woutput[w], "年寄り")) {
    ngram_prob += 2.0;
  }
  return ngram_prob;
}

static LOGPROB
my_lm(WORD_INFO *winfo, WORD_ID *contexts, int context_len, WORD_ID w, LOGPROB ngram_prob)
{
  int i;
/* 
 *   for(i=0;i<context_len;i++) {
 *     if (i == 0) printf("[%s", winfo->woutput[contexts[i]]);
 *     else printf(" %s", winfo->woutput[contexts[i]]);
 *   }
 *   printf("|%s] %f -> %f\n", winfo->woutput[w], ngram_prob, ngram_prob);
 */
  if (strmatch(winfo->woutput[w], "年寄り")) {
    ngram_prob += 2.0;
  }
  return ngram_prob;
}

#endif /* USER_LM_TEST */

/************************************************************************/
/**
 * Callbacks for application option handling.
 * 
 */
static boolean
opt_help(Jconf *jconf, char *arg[], int argnum)
{
  fprintf(stderr, "Julius rev.%s - based on ", JULIUS_VERSION);
  j_output_argument_help(stderr);
  exit(1);			/* terminates here! */
  return TRUE;
}

   
/**********************************************************************/
int
main(int argc, char *argv[])
{
  Recog *recog[MULTI_N];
  Jconf *jconf[MULTI_N];
  Model *model[MULTI_N];
  int i, num;

  /* inihibit system log output (default: stdout) */
  //jlog_set_output(NULL);
  /* output system log to a file */
  // FILE *fp = fopen(logfile, "w"); jlog_set_output(fp);

  /* if no option argument, output julius usage and exit */
  if (argc == 1) {
    fprintf(stderr, "Julius rev.%s - based on ", JULIUS_VERSION);
    j_put_version(stderr);
    fprintf(stderr, "Try '-setting' for built-in engine configuration.\n");
    fprintf(stderr, "Try '-help' for run time options.\n");
    return -1;
  }

  /* add application options */
  record_add_option();
  module_add_option();
  charconv_add_option();
  j_add_option("-help", 0, "display this help", opt_help);
  j_add_option("--help", 0, "display this help", opt_help);

  /* load configurations from arguments or jconf file to the container */
#ifdef MULTI
  num = 0;
  for(i=1;i<argc;i++) {
    jconf[num] = j_jconf_new();
    if (j_config_load_file(jconf[num], argv[i]) == -1) {
      fprintf(stderr, "Try `-help' for more information.\n");
      return -1;
    }
    num++;
  }
#else
  /* create a configuration variables container */
  jconf[0] = j_jconf_new();
  // j_config_load_file(jconf, jconffile);
  if (j_config_load_args(jconf[0], argc, argv) == -1) {
    fprintf(stderr, "Try `-help' for more information.\n");
    return -1;
  }
  num = 1;
#endif /* ~MULTI */

  /* here you can set/modify any parameter in the jconf before setup */
  // jconf->input.input_speech = SP_MIC;

  /* Fixate jconf parameters: it checks whether the jconf parameters
     are suitable for recognition or not, and set some internal
     parameters according to the values for recognition.  Modifying
     a value in jconf after this function may be errorous.
  */
  for(i=0;i<num;i++) {
    if (j_jconf_finalize(jconf[i]) == FALSE) {
      return -1;
    }
  }

  /* create a model container */
  for(i=0;i<num;i++) {
    model[i] = j_model_new();
    /* load all models into memory according to the jconf configurations */
    if (j_model_load_all(model[i], jconf[i]) == FALSE) {
      fprintf(stderr, "Error loading model\n");
      return -1;
    }
  }

  for(i=0;i<num;i++) {
    /* create a recognition instance */
    recog[i] = j_recog_new();
    /* assign model to the instance */
    recog[i]->model = model[i];
    /* assign configuration to the instance */
    recog[i]->jconf = jconf[i];
#ifdef USER_LM_TEST
    j_regist_user_lm_func(recog[i], my_uni, my_bi, my_lm);
#endif
    /* checkout for recognition: build lexicon tree, allocate cache */
    j_final_fusion(recog[i]);

    /* Set up some application functions */
    /* set character conversion mode */
    if (charconv_setup() == FALSE) return -1;
    if (is_module_mode()) {
      /* set up for module mode */
      /* register result output callback functions to network module */
      module_setup(recog[i], NULL);
    } else {
      /* register result output callback functions to stdout */
      setup_output_tty(recog[i], NULL);
    }
  }
  /* setup recording if option was specified */
  record_setup(recog[0], NULL);

  /* initialize and standby the specified audio input source */
  /* for microphone or other threaded input, ad-in thread starts at this time */
#ifdef MULTI
  if (j_adin_init_multi(recog, num) == FALSE) {
    /* error */
    return -1;
  }
#else
  if (j_adin_init(recog[0]) == FALSE) {
    /* error */
    return -1;
  }
#endif /* MULTI */

  /* output system information to log */
  for(i=0;i<num;i++) {
    jlog("######## SETUP %d\n", i+1);
    j_recog_info(recog[i]);
  }

#ifdef VISUALIZE
  /* Visualize: initialize GTK */
  visual_init(recog[0]);
  callback_add(recog[0], CALLBACK_EVENT_RECOGNITION_END, visual_show, NULL);
  callback_add(recog[0], CALLBACK_EVENT_PASS2_BEGIN, visual2_init, NULL);
  callback_add(recog[0], CALLBACK_DEBUG_PASS2_POP, visual2_popped, NULL);
  callback_add(recog[0], CALLBACK_DEBUG_PASS2_PUSH, visual2_next_word, NULL);
  /* below should be called at result */
  visual2_best(now, winfo);
  /* 音声取り込みはコールバックで新規作成 */
  /* 第2パスで認識結果出力時に以下を実行 */
  visual2_best(now, recog[0]->model->winfo);
#endif

#ifndef MULTI
  if (recog[0]->lmtype == LM_DFA) {
    /* if no grammar specified on startup, start with pause status */
    if (recog[0]->model->dfa == NULL || recog[0]->model->winfo == NULL) { /* stop when no grammar found */
      j_request_pause(recog[0]);
    }
  }
#endif

  /* enter recongnition loop */
  if (is_module_mode()) {
    module_recognition_stream_loop(recog, num);
  } else {
    main_recognition_stream_loop(recog, num);
  }

  /* release all */
  for(i=0;i<num;i++) j_recog_free(recog[i]);

  return(0);
}
