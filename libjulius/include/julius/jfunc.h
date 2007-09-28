/*
 * Copyright (c) 1991-2006 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2006 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#include <julius/julius.h>
#include <stdarg.h>

#ifndef __J_JFUNC_H__
#define __J_JFUNC_H__

/* recogmain.c */
int j_open_stream(Recog *recog, char *file_or_dev_name);
int j_recognize_stream(Recog *recog);

/* jfunc.c */
void j_request_pause(Recog *recog);
void j_request_terminate(Recog *recog);
void j_request_resume(Recog *recog);
void schedule_grammar_update(Recog *recog);
void j_reset_reload(Recog *recog);
void j_enable_debug_message();
void j_disable_debug_message();
void j_enable_verbose_message();
void j_disable_verbose_message();
void j_internal_error(char *fmt, ...);
Model *j_model_new();
void j_model_free(Model *model);
Jconf *j_jconf_new();
void j_jconf_free(Jconf *jconf);
Recog *j_recog_new();
void j_recog_free(Recog *recog);
int j_config_load_args(Jconf *jconf, int argc, char *argv[]);
int j_config_load_file(Jconf *jconf, char *filename);
Jconf *j_config_load_args_new(int argc, char *argv[]);
Jconf *j_config_load_file_new(char *filename);
boolean j_adin_init(Recog *recog);
boolean j_adin_init_user(Recog *recog, void *arg);
void j_recog_info(Recog *recog);
Recog *j_create_instance_from_jconf(Jconf *jconf);

boolean j_regist_user_lm_func(Recog *recog, LOGPROB (*unifunc)(WORD_INFO *winfo, WORD_ID w, LOGPROB ngram_prob), LOGPROB (*bifunc)(WORD_INFO *winfo, WORD_ID context, WORD_ID w, LOGPROB ngram_prob), LOGPROB (*probfunc)(WORD_INFO *winfo, WORD_ID *contexts, int context_len, WORD_ID w, LOGPROB ngram_prob));
boolean j_regist_user_param_func(Recog *recog, boolean (*user_calc_vector)(VECT *, SP16 *, int, Value *, Recog *));

#endif /* __J_JFUNC_H__ */
