/**
 * @file   extern.h
 * @author Akinobu LEE
 * @date   Mon Mar  7 23:19:14 2005
 * 
 * <JA>
 * @brief  外部関数宣言
 * </JA>
 * 
 * <EN>
 * @brief  External function declarations
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

/* should be included after all include files */

/* backtrellis.c */
void bt_init(BACKTRELLIS *bt);
void bt_prepare(BACKTRELLIS *bt);
void bt_free(BACKTRELLIS *bt);
TRELLIS_ATOM *bt_new(BACKTRELLIS *bt);
void bt_store(BACKTRELLIS *bt, TRELLIS_ATOM *aotm);
void bt_relocate_rw(BACKTRELLIS *bt);
#ifdef SP_BREAK_CURRENT_FRAME
void set_terminal_words(Recog *recog);
#endif
void bt_discount_pescore(WCHMM_INFO *wchmm, BACKTRELLIS *bt, HTK_Param *param);
void bt_discount_lm(BACKTRELLIS *bt);
void bt_sort_rw(BACKTRELLIS *bt);
TRELLIS_ATOM *bt_binsearch_atom(BACKTRELLIS *bt, int time, WORD_ID wkey);

/* factoring_sub.c */
void make_iwcache_index(WCHMM_INFO *wchmm);
void adjust_sc_index(WCHMM_INFO *wchmm);
void make_successor_list(WCHMM_INFO *wchmm);
void max_successor_cache_init(WCHMM_INFO *wchmm);
void max_successor_cache_free(WCHMM_INFO *wchmm);
LOGPROB max_successor_prob(WCHMM_INFO *wchmm, WORD_ID lastword, int node);
LOGPROB *max_successor_prob_iw(WCHMM_INFO *wchmm, WORD_ID lastword);
void  calc_all_unigram_factoring_values(WCHMM_INFO *wchmm);
boolean can_succeed(WCHMM_INFO *wchmm, WORD_ID lastword, int node);

/* beam.c */
boolean get_back_trellis_init(HTK_Param *param, Recog *recog);
boolean get_back_trellis_proceed(int t, HTK_Param *param, Recog *recog, boolean final_for_multipath);
void get_back_trellis_end(HTK_Param *param, Recog *recog);
boolean get_back_trellis(Recog *recog);
LOGPROB finalize_1st_pass(Recog *recog, int len);
#ifdef SP_BREAK_CURRENT_FRAME
boolean is_sil(WORD_ID w, WORD_INFO *winfo, HTK_HMM_INFO *hmm);
void finalize_segment(Recog *recog, HTK_Param *param, int len);
#endif

/* outprob_style.c */
#ifdef PASS1_IWCD
void outprob_style_cache_init(WCHMM_INFO *wchmm);
CD_Set *lcdset_lookup_with_category(WCHMM_INFO *wchmm, HMM_Logical *hmm, WORD_ID category);
void lcdset_register_with_category_all(WCHMM_INFO *wchmm);
void lcdset_remove_with_category_all(WCHMM_INFO *wchmm);
#endif
LOGPROB outprob_style(WCHMM_INFO *wchmm, int node, int last_wid, int t, HTK_Param *param);
void error_missing_right_triphone(HMM_Logical *base, char *rc_name);
void error_missing_left_triphone(HMM_Logical *base, char *lc_name);

/* ngram_decode.c */
#include "search.h"
int ngram_firstwords(NEXTWORD **nw, int peseqlen, int maxnw, Recog *recog);
int ngram_nextwords(NODE *hypo, NEXTWORD **nw, int maxnw, Recog *recog);
boolean ngram_acceptable(NODE *hypo, Recog *recog);
int dfa_firstwords(NEXTWORD **nw, int peseqlen, int maxnw, Recog *recog);
int dfa_nextwords(NODE *hypo, NEXTWORD **nw, int maxnw, Recog *recog);
boolean dfa_acceptable(NODE *hypo, Recog *recog);
boolean dfa_look_around(NEXTWORD *nword, NODE *hypo, Recog *recog);

/* search_bestfirst_main.c */
void sp_segment_set_last_nword(NODE *hypo, Recog *recog);
void wchmm_fbs(HTK_Param *param, Recog *recog, int cate_bgn, int cate_num);

/* search_bestfirst_v?.c */
void clear_stocker(StackDecode *s);
void free_node(NODE *node);
NODE *cpy_node(NODE *dst, NODE *src);
NODE *newnode(Recog *recog);
void malloc_wordtrellis(Recog *recog);
void free_wordtrellis();
void scan_word(NODE *now, HTK_Param *param, Recog *recog);
void next_word(NODE *now, NODE *new, NEXTWORD *nword, HTK_Param *param, Recog *recog);
void start_word(NODE *new, NEXTWORD *nword, HTK_Param *param, Recog *recog);
void last_next_word(NODE *now, NODE *new, HTK_Param *param, Recog *recog);

/* wav2mfcc.c */
boolean wav2mfcc(SP16 speech[], int speechlen, Recog *recog);

/* version.c */
void j_put_header(FILE *stream);
void j_put_version(FILE *stream);
void j_put_compile_defs(FILE *stream);
void j_put_library_defs(FILE *stream);

/* wchmm.c */
WCHMM_INFO *wchmm_new();
void wchmm_free(WCHMM_INFO *w);
void print_wchmm_info(WCHMM_INFO *wchmm);
boolean build_wchmm(WCHMM_INFO *wchmm, Jconf *jconf);
boolean build_wchmm2(WCHMM_INFO *wchmm, Jconf *jconf);

/* wchmm_check.c */
void wchmm_check_interactive(WCHMM_INFO *wchmm);
void check_wchmm(WCHMM_INFO *wchmm);

/* realtime.c --- callback for adin_cut() */
boolean RealTimeInit(Recog *recog);
boolean RealTimePipeLinePrepare(Recog **recoglist, int recognum);
boolean RealTimeMFCC(VECT *tmpmfcc, SP16 *window, int windowlen, Value *para, Recog *re);
int RealTimePipeLine(SP16 *Speech, int len, Recog **recoglist, int recognum);
int RealTimeResume(Recog **recoglist, int recognum);
boolean RealTimeParam(Recog **recoglist, int recognum);
void RealTimeCMNUpdate(HTK_Param *param, Recog *recog);
void RealTimeTerminate(Recog **recoglist, int recognum);
void realbeam_free(Recog *recog);

/* word_align.c */
void word_align(WORD_ID *words, short wnum, HTK_Param *param, Sentence *s, Recog *recog);
void phoneme_align(WORD_ID *words, short wnum, HTK_Param *param, Sentence *s, Recog *recog);
void state_align(WORD_ID *words, short wnum, HTK_Param *param, Sentence *s, Recog *recog);
void word_rev_align(WORD_ID *revwords, short wnum, HTK_Param *param, Sentence *s, Recog *recog);
void phoneme_rev_align(WORD_ID *revwords, short wnum, HTK_Param *param, Sentence *s, Recog *recog);
void state_rev_align(WORD_ID *revwords, short wnum, HTK_Param *param, Sentence *s, Recog *recog);

/* m_usage.c */
void opt_terminate();
void j_output_argument_help(FILE *fp);
/* m_options.c */
char *filepath(char *filename, char *dirname);
boolean opt_parse(int argc, char *argv[], char *cwd, Jconf *jconf);
void opt_release(Jconf *jconf);
/* m_jconf.c */
void get_dirname(char *path);
boolean config_file_parse(char *conffile, Jconf *jconf);
/* m_chkparam.c */
boolean checkpath(char *filename);
boolean j_jconf_finalize(Jconf *jconf);
int set_beam_width(WCHMM_INFO *wchmm, int specified);
void set_lm_weight(Jconf *jconf, Model *model);
void set_lm_weight2(Jconf *jconf, Model *model);
/* m_info.c */
void print_setting(Jconf *jconf);
void print_info(Recog *recog);
/* m_bootup.c */
void system_bootup(Recog *recog);
/* m_adin.c */
boolean adin_initialize(Recog *recog);
boolean adin_initialize_user(Recog *recog, void *arg);
/* m_fusion.c */
boolean j_model_load_all(Model *model, Jconf *jconf);
boolean j_final_fusion(Recog *recog);
/* result_tty.c */
void setup_result_tty();
/* result_msock.c */
void setup_result_msock(Recog *recog);
void decode_output_selection(char *str);

/* hmm_check.c */
void hmm_check(Jconf *jconf, WORD_INFO *winfo, HTK_HMM_INFO *hmminfo);

/* visual.c */
void visual_init(Recog *recog);
void visual_show(BACKTRELLIS *bt);
void visual2_init(int maxhypo);
void visual2_popped(NODE *n, int popctr);
void visual2_next_word(NODE *next, NODE *prev, int popctr);
void visual2_best(NODE *now, WORD_INFO *winfo);

/* gmm.c */
boolean gmm_init(Recog *recog);
void gmm_prepare(Recog *recog);
void gmm_proceed(Recog *recog, HTK_Param *param, int t);
void gmm_end(Recog *recog);
boolean gmm_valid_input(Recog *recog);
void gmm_free(Recog *recog);
#ifdef GMM_VAD
boolean gmm_is_valid_frame(Recog *recog, VECT *vec, short veclen);
#endif

/* graphout.c */
void wordgraph_init(WCHMM_INFO *wchmm);
void wordgraph_free(WordGraph *wg);
void put_wordgraph(FILE *fp, WordGraph *wg, WORD_INFO *winfo);
void wordgraph_dump(FILE *fp, WordGraph *root, WORD_INFO *winfo);
WordGraph *wordgraph_assign(WORD_ID wid, WORD_ID wid_left, WORD_ID wid_right, int leftframe, int rightframe, LOGPROB fscore_head, LOGPROB fscore_tail, LOGPROB gscore_head, LOGPROB gscore_tail, LOGPROB lscore, LOGPROB cmscore, Recog *recog);
boolean wordgraph_check_and_add_rightword(WordGraph *wg, WordGraph *right, LOGPROB lscore);
boolean wordgraph_check_and_add_leftword(WordGraph *wg, WordGraph *left, LOGPROB lscore);
void wordgraph_save(WordGraph *wg, WordGraph *right, WordGraph **root);
WordGraph *wordgraph_check_merge(WordGraph *now, WordGraph **root, WORD_ID next_wid, boolean *merged_p, Jconf *jconf);
WordGraph *wordgraph_dup(WordGraph *wg, WordGraph **root);
void wordgraph_purge_leaf_nodes(WordGraph **rootp, Recog *recog);
void wordgraph_depth_cut(WordGraph **rootp, Recog *recog);
void wordgraph_adjust_boundary(WordGraph **rootp, Recog *recog);
void wordgraph_clean(WordGraph **rootp);
void wordgraph_compaction_thesame(WordGraph **rootp);
void wordgraph_compaction_exacttime(WordGraph **rootp, Recog *recog);
void wordgraph_compaction_neighbor(WordGraph **rootp, Recog *recog);
int wordgraph_sort_and_annotate_id(WordGraph **rootp, Recog *recog);
void wordgraph_check_coherence(WordGraph *rootp, Recog *recog);

/* main.c */
void main_recognition_loop(Recog *recog);

/* default.c */
void jconf_set_default_values(Jconf *j);

/* multi-gram.c */
boolean multigram_delete(int gid, Recog *recog);
void multigram_add(DFA_INFO *dfa, WORD_INFO *winfo, char *name, Model *model);
void multigram_delete_all(Recog *recog);
boolean multigram_exec(Recog *recog);
int multigram_activate(int gid, Recog *recog);
int multigram_deactivate(int gid, Recog *recog);

void multigram_add_gramlist(char *dfafile, char *dictfile, Jconf *jconf, int lmvar);
void multigram_remove_gramlist(Jconf *jconf);
boolean multigram_read_all_gramlist(Jconf *jconf, Model *model);
boolean multigram_add_prefix_list(char *prefix_list, char *cwd, Jconf *jconf, int lmvar);
boolean multigram_add_prefix_filelist(char *listfile, Jconf *jconf, int lmvar);
int multigram_get_active_num(Recog *recog);
int multigram_get_gram_from_category(int category, Recog *recog);
int multigram_get_all_num(Recog *recog);
void multigram_free_all(MULTIGRAM *root);

/* adin-cut.c */
void adin_setup_param(ADIn *adin, Jconf *jconf);
boolean adin_thread_create(Recog **recoglist, int recognum);
int adin_go(int (*ad_process)(SP16 *, int, Recog **, int), int (*ad_check)(Recog **, int), Recog **recoglist, int recognum);
boolean adin_standby(ADIn *a, int freq, void *arg);
boolean adin_begin(ADIn *a);
boolean adin_end(ADIn *a);
void adin_free_param(Recog *recog);

/* confnet.c */
CN_CLUSTER *confnet_create(WordGraph *root, Recog *recog);
void graph_forward_backward(WordGraph *root, Recog *recog);
void graph_make_order(WordGraph *root, Recog *recog);
void graph_free_order();
void cn_free_all(CN_CLUSTER **croot);

/* callback.c */
void callback_init(Recog *recog);
int callback_add(Recog *recog, int code, void (*func)(Recog *recog, void *data), void *data);
int callback_add_adin(Recog *recog, int code, void (*func)(Recog *recog, SP16 *buf, int len, void *data), void *data);
void callback_exec(int code, Recog *recog);
void callback_exec_adin(int code, Recog *recog, SP16 *buf, int len);
boolean callback_exist(Recog *recog, int code);
boolean callback_delete(Recog *recog, int id);
void callback_multi_exec(int code, Recog **recoglist, int num);

