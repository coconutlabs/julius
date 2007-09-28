/**
 * @file   common.h
 * @author Akinobu Lee
 * @date   Fri Feb 16 13:42:28 2007
 * 
 * <JA>
 * @brief  Julius/Julian 共有変数の定義
 *
 * Julius/Julian 全体で用いられる共有変数を定義します．ここでは以下の
 * 構造体を定義しています．
 *  - Jconf: ユーザ設定を管理する構造体．サブ構造は以下の通り:
 *     - input          音声入力
 *     - detect         音声区間検出
 *     - analysis       音響分析
 *     - frontend       フロントエンド処理 (CMN, SS, etc.)
 *     - am             音響モデル
 *     - lm             言語モデル (n-gram / dfa) および辞書
 *     - search         探索パラメータ
 *     - graph          単語グラフ
 *     - annotate       信頼度付与，アラインメント等
 *     - output         認識結果の出力
 *     - reject         入力棄却
 *     - server         モジュールモード
 *     - successive     逐次デコーディング設定
 *     - sw             その他動作スイッチ
 *  - Model: 読み込んだモデルの情報を格納する構造体
 *  - Recog: 認識処理のトップインスタンス．認識に使用するモデル，ユーザ設定，
 *           および内部情報やワークエリアを保管する構造体
 *
 *  デフォルトの値については jconf_set_default_values() を参照してください．
 * </JA>
 * 
 * <EN>
 * @brief  Global variables
 *
 * This file defines common parameter and work area structure of Julius/Julian.
 * The defined structures are:
 *  - Jconf: user-defined parameters and settings.  Sub structures are:
 *     - input          speech input
 *     - detect         voice activity detection
 *     - analysis       acoustic analysis
 *     - frontend       front-end processing (CMN, SS, etc.)
 *     - am             acoustic model
 *     - lm             language model (n-gram / dfa) and dictionary
 *     - search         search parameters
 *     - graph          word graph
 *     - annotate       confidence scoring, forced alignment, etc.
 *     - output         result output
 *     - reject         input rejection
 *     - server         module mode
 *     - successive     successive decoding
 *     - sw             misc. switches
 *  - Model: structure to hold informations for language / acoustic models
 *  - Recog: the top instance of recognition process that includes Jconf and
 *    Model used for the recognition, and work area for search and internal
 *    status
 *
 *  See jconf_set_default_values() for system default values.
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

#ifndef __J_COMMON_H__
#define __J_COMMON_H__

#include <sent/stddefs.h>
#include <sent/hmm.h>
#include <sent/vocabulary.h>
#include <sent/ngram2.h>
#include <sent/dfa.h>
#include <julius/wchmm.h>
#include <julius/search.h>
#include <julius/callback.h>

/**
 * Configuration parameters
 * 
 */
typedef struct __Jconf__ {

  /**
   * Configurations for audio input.
   * (Sampling frequency and other acoustic parameters are
   * in analysis section)
   * 
   */
  struct {
    /**
     * Selected input source (-input)
     */
    int speech_input;
    /**
     * Use 48kHz input and perform down sampling to 16kHz (-48)
     */
    boolean use_ds48to16;
    /**
     * List of input files for rawfile / mfcfile input (-filelist) 
     */
    char *inputlist_filename;
    /**
     * Port number for adinnet input (-adport)
     */
    int adinnet_port;
#ifdef USE_NETAUDIO
    /**
     * Host/unit name for NetAudio/DatLink input (-NA)
     */
    char *netaudio_devname;
#endif
  } input;

  /**
   * Configurations for Voice activity detection
   * 
   */
  struct {
    /**
     * Input level threshold from 0 to 32767 (-lv)
     */
    int level_thres;
    /**
     * Head margin in msec (-headmargin)
     */
    int head_margin_msec;
    /**
     * Tail margin in msec (-tailmargin)
     */
    int tail_margin_msec;
    /**
     * Zero cross number threshold per a second (-zc)
     */
    int zero_cross_num;
    /**
     * Silence detection and cutting: 0=off, 1=on, 2=accept device default
     * (-cutsilence / -nocutsilence)
     */
    int silence_cut;
  } detect;

  /**
   * Acoustic Analysis Conditions.  Parameter setting priority is:
   * user-specified > specified HTK Config > model-embedded > Julius default.
   * 
   */
  struct {
    /**
     * All MFCC computation parameters, actually used for recognition.
     */
    Value para;         
    /**
     * default parameters of Julius
     */
    Value para_default;
    /**
     * parameters from binhmm header
     */
    Value para_hmm;             
    /**
     * parameters from HTK Config (-htkconf)
     */
    Value para_htk;     
    /**
     * Check input parameter type with header of the hmmdefs
     * (-notypecheck to unset)
     */
    boolean paramtype_check_flag;
  } analysis;

  /**
   * Parameters for frontend processing of speech data.
   * Currently implemented features are:
   *  - cepstral mean normalization (CMN)
   *  - spectral subtraction (SS).
   *  - zero frame stripping
   *  - DC offset removal
   * 
   */
  struct {
    /**
     * CMN: load initial cepstral mean from file at startup (-cmnload)
     */
    char *cmnload_filename;
    /**
     * CMN: update cepstral mean while recognition
     * (-cmnnoupdate to unset)
     */
    boolean cmn_update;
    /**
     * CMN: save cepstral mean to file at end of every recognition (-cmnsave)
     */
    char *cmnsave_filename;     
    /**
     * CMN: MAP weight for initial cepstral mean on (-cmnmapweight)
     */
    float cmn_map_weight;
    /**
     * SS: compute noise spectrum from head silence on file input (-sscalc)
     */
    boolean sscalc;
    /**
     * With "-sscalc", specify noise length at input head in msec (-sscalclen)
     */
    int sscalc_len;
    /**
     * Load noise spectrum data from file (-ssload), that was made by "mkss".
     */
    char *ssload_filename;
    /**
     * Strip off zero samples (-nostrip to unset)
     */
    boolean strip_zero_sample;
    /**
     * Remove DC offset by zero mean (-zmean / -nozmean)
     */
    boolean use_zmean;
  } frontend;

  /**
   * Configuration for acoustic model (HMM, HMMList) and acoustic computation
   * 
   */
  struct {
    /**
     * HMM definition file (-h)
     */
    char *hmmfilename;
    /**
     * HMMList file to map logical (tri)phones to physical models (-hlist)
     */
    char *mapfilename;
    /**
     * Gaussian pruning method (-gprune)
     * Default: use value from compile-time engine configuration default.
     */
    int gprune_method;
    /**
     * Number of Gaussian to compute per mixture on Gaussian pruning (-tmix)
     */
    int mixnum_thres;   
    /**
     * Logical HMM name of short pause model (-spmodel)
     * Default: "sp"
     */
    char *spmodel_name;
    /**
     * GMS: HMM definition file for GMS (-gshmm)
     */
    char *hmm_gs_filename;
    /**
     * GMS: number of mixture PDF to select (-gsnum)
     */
    int gs_statenum;    
    /**
     * Force triphone handling
     */
    boolean ccd_flag_force;
    /**
     * INTERNAL: Handle hmmdefs as context-dependent HMM if TRUE
     * (default determined from hmmdefs macro name)
     */
    boolean ccd_flag;
    /**
     * force multipath mode
     * 
     */
    boolean force_multipath;

  } am;

  /**
   * Language models (N-gram / DFA), dictionary, and related parameters.
   * 
   */
  struct {
    /**
     * Word dictionary file (-v)
     */
    char *dictfilename;

    /**
     * Silence word to be placed at beginning of speech (-silhead) for N-gram
     */
    char *head_silname;
    /**
     * Silence word to be placed at end of search (-siltail) for N-gram
     */
    char *tail_silname;

    /**
     * For isolated word recognition mode: name of head silence model
     */
    char wordrecog_head_silence_model_name[MAX_HMMNAME_LEN];
    /**
     * For isolated word recognition mode: name of tail silence model
     */
    char wordrecog_tail_silence_model_name[MAX_HMMNAME_LEN];
    /**
     * For isolated word recognition mode: name of silence as phone context
     */
    char wordrecog_silence_context_name[MAX_HMMNAME_LEN];

    /**
     * Skip error words in dictionary and continue (-forcedict)
     */
    boolean forcedict_flag;


    /**
     * N-gram in binary format (-d)
     */
    char *ngram_filename;
    /**
     * LR 2-gram in ARPA format (-nlr)
     */
    char *ngram_filename_lr_arpa;
    /**
     * RL 3-gram in ARPA format (-nrl)
     */
    char *ngram_filename_rl_arpa;

    /**
     * DFA grammar file (-dfa, for single use)
     */
    char *dfa_filename;

    /**
     * List of grammars to be read at startup (-gram) (-gramlist)
     */
    GRAMLIST *gramlist_root;

    /**
     * List of word lists to be read at startup (-w) (-wlist)
     */
    GRAMLIST *wordlist_root;

    /**
     * Enable inter-word short pause handling on multi-path version (-iwsp)
     * for multi-path mode
     */
    boolean enable_iwsp; 
    /**
     * Transition penalty of inter-word short pause (-iwsppenalty)
     * for multi-path mode
     */
    LOGPROB iwsp_penalty; 

    /**
     * Enable automatic addition of "short pause word" to the dictionary
     * (-iwspword) for N-gram
     */
    boolean enable_iwspword;
    /**
     * Dictionary entry to be added on "-iwspword" (-iwspentry) for N-gram
     */
    char *iwspentry;

    /**
     * N-gram Language model weight (-lmp)
     */
    LOGPROB lm_weight;  
    /**
     * N-gram Word insertion penalty (-lmp)
     */
    LOGPROB lm_penalty; 
    /**
     * N-gram Language model weight for 2nd pass (-lmp2)
     */
    LOGPROB lm_weight2; 
    /**
     * N-gram Word insertion penalty for 2nd pass (-lmp2)
     */
    LOGPROB lm_penalty2;        
    /**
     * N-gram Additional insertion penalty for transparent words (-transp)
     */
    LOGPROB lm_penalty_trans;

    /**
     * Word insertion penalty for DFA grammar on 1st pass (-penalty1)
     */
    LOGPROB penalty1;
    /**
     * Word insertion penalty for DFA grammar on 2nd pass (-penalty2)
     */
    LOGPROB penalty2;

    /**
     * INTERNAL: TRUE if -lmp2 specified
     */
    boolean lmp2_specified;
    
    /**
     * INTERNAL: TRUE if -lmp specified
     */
    boolean lmp_specified;

  } lm;

  /**
   * Search parameters to control recognition process
   * 
   */
  struct {
    /**
     * First pass parameters
     * 
     */
    struct {
      /**
       * Beam width of 1st pass. If value is -1 (not specified), system
       * will guess the value from dictionary size.  If 0, a possible
       * maximum value will be assigned to do full search.
       */
      int specified_trellis_beam_width;

      /**
       * INTERNAL: do on-the-fly decoding if TRUE (value depends on
       * device default and forced_realtime.
       */
      boolean realtime_flag;    

      /**
       * INTERNAL: TRUE if either of "-realtime" or "-norealtime" is
       * explicitly specified by user.  When TRUE, the user-specified value
       * in forced_realtime will be applied to realtime_flag.
       */
      boolean force_realtime_flag;

      /**
       * Force on-the-fly decoding on 1st pass with audio input and
       * MAP-CMN (-realtime / -norealtime)
       */
      boolean forced_realtime;

      /**
       * Calculation method for outprob score of a lcdset on cross-word
       * triphone (-iwcd1) 
       */
      short iwcdmethod;

      /**
       * N-best states to be calculated on IWCD_NBEST (-iwcd1 best N)
       */
      short iwcdmaxn;

#ifdef SEPARATE_BY_UNIGRAM
      /**
       * Number of best frequency words to be separated (linearized)
       * from lexicon tree (-sepnum)
       */
      int separate_wnum;
#endif

#if defined(WPAIR) && defined(WPAIR_KEEP_NLIMIT)
      /**
       * Keeps only N token on word-pair approximation (-nlimit)
       */
      int wpair_keep_nlimit;
#endif

#ifdef HASH_CACHE_IW
      /**
       * Inter-word LM cache size rate (-iwcache)
       */
      int iw_cache_rate;
#endif

      /**
       * (DEBUG) use old build_wchmm() instead of build_wchmm2() for lexicon
       * construction (-oldtree)
       */
      boolean old_tree_function_flag;

#ifdef DETERMINE
      /**
       * (EXPERIMENTAL) score threshold between maximum node score and
       * maximum word end score for early word determination
       * 
       */
      LOGPROB determine_score_thres;

      /**
       * (EXPERIMENTAL) frame duration threshold for early word determination
       * 
       */
      int determine_duration_thres;

#endif /* DETERMINE */


    } pass1;

    /**
     * Second pass parameters
     * 
     */
    struct {
      /**
       * Search until N-best sentences are found (-n). Also see "-output".
       */
      int nbest;                
      /**
       * Word beam width of 2nd pass. -1 means no beaming (-b2)
       */
      int enveloped_bestfirst_width;
#ifdef SCAN_BEAM
      /**
       * Score beam threshold of 2nd pass (-sb)
       */
      LOGPROB scan_beam_thres;
#endif
      /**
       * Hypothesis overflow threshold at 2nd pass (-m)
       */
      int hypo_overflow;
      /**
       * Hypothesis stack size of 2nd pass (-s)
       */
      int stack_size;
      /**
       * Get next words from word trellis with a range of this frames
       * on 2nd pass (-lookuprange)
       */
      int lookup_range;

      /**
       * Limit expansion words for trellis words on neighbor frames
       * at 2nd pass of DFA for speedup (-looktrellis)
       */
      boolean looktrellis_flag;

    } pass2;

    /**
     * TRUE if enable short-pause segmentation
     * 
     */
    boolean sp_segment;

  } search;

  /**
   * Word graph output
   * 
   */
  struct {

    /**
     * GraphOut: if enabled, graph search is enabled.
     * 
     */
    boolean enabled;

    /**
     * GraphOut: if enabled, output word graph
     * 
     */
    boolean lattice;

    /**
     * GraphOut: if enabled, generate confusion network
     * 
     */
    boolean confnet;

    /**
     * GraphOut: allowed margin for post-merging on word graph generation
     * (-graphrange) if set to -1, same word with different phone context
     * will be separated.
     */
    int graph_merge_neighbor_range;

#ifdef   GRAPHOUT_DEPTHCUT
    /**
     * GraphOut: density threshold to cut word graph at post-processing.
     * (-graphcut)  Setting larger value is safe for all condition.
     */
    int graphout_cut_depth;
#endif

#ifdef   GRAPHOUT_LIMIT_BOUNDARY_LOOP
    /**
     * GraphOut: limitation of iteration loop for word boundary adjustment
     * (-graphboundloop)
     */
    int graphout_limit_boundary_loop_num;
#endif

#ifdef   GRAPHOUT_SEARCH_DELAY_TERMINATION
    /**
     * GraphOut: delay the termination of search on graph merging until
     * at least one sentence candidate is found
     * (-graphsearchdelay / -nographsearchdelay)
     */
    boolean graphout_search_delay;
#endif

  } graph;

  /**
   * Parameters for output annotation (confidence, alignment, etc.)
   * 
   */
  struct {

#ifdef CONFIDENCE_MEASURE
    /**
     * Scaling factor for confidence scoring (-cmalpha)
     */
    LOGPROB cm_alpha;

#ifdef   CM_MULTIPLE_ALPHA
    /**
     * Begin value of alpha
     */
    LOGPROB cm_alpha_bgn;
    /**
     * End value of alpha
     */
    LOGPROB cm_alpha_end;
    /**
     * Number of test values (will be set from above values)
     */
    int cm_alpha_num;
    /**
     * Step value of alpha
     */
    LOGPROB cm_alpha_step;
#endif

#ifdef   CM_SEARCH_LIMIT
    /**
     * Cut-off threshold for generated hypo. for confidence decoding (-cmthres)
     */
    LOGPROB cm_cut_thres;
#endif

#ifdef   CM_SEARCH_LIMIT_POPO
    /**
     * Cut-off threshold for popped hypo. for confidence decoding (-cmthres2)
     */
    LOGPROB cm_cut_thres_pop;
#endif

#endif /* CONFIDENCE_MEASURE */


    /**
     * Forced alignment: per word (-walign)
     */
    boolean align_result_word_flag;
    /**
     * Forced alignment: per phoneme (-palign)
     */
    boolean align_result_phoneme_flag;
    /**
     * Forced alignment: per state (-salign)
     */
    boolean align_result_state_flag;

  } annotate;

  /**
   * Output configurations
   * 
   */
  struct {
    /**
     * Result: number of sentence to output (-output) , also see @a nbest (-n).
     */
    int output_hypo_maxnum;
    /**
     * Result: output partial recognition result on the 1st pass (-progout)
     */
    boolean progout_flag;
    /**
     * Result: Progressive output interval on 1st pass in msec (-proginterval)
     */
    int progout_interval;
    /**
     * Result: INTERNAL: interval in number of frames
     */
    int progout_interval_frame;

    /**
     * Result: Output AM and LM score independently (-separatescore) on N-gram
     */
    boolean separate_score_flag;

    /**
     * Get results for all grammars independently on 2nd pass on DFA
     * (-multigramout / -nomultigramout)
     */
    boolean multigramout_flag;

  } output;

  /**
   * Models and parameters for input rejection
   * 
   */
  struct {
    /**
     * GMM definition file (-gmm)
     */
    char *gmm_filename;
    /**
     * Number of Gaussians to be computed on GMM calculation (-gmmnum)
     */
    int gmm_gprune_num;
    /**
     * Comma-separated list of GMM model name to be rejected (-gmmreject)
     */
    char *gmm_reject_cmn_string;
    /**
     * Length threshold to reject input (-rejectshort)
     */
    int rejectshortlen;
  } reject;

  /**
   * Successive decoding (--enable-sp-segment)
   * 
   */
  struct {

#ifdef SP_BREAK_CURRENT_FRAME
    /**
     * Default length threshold to detect short-pause segment in frames
     */
    int sp_frame_duration;
#endif

  } successive;

  /**
   * Misc. switches
   * 
   */
  struct {
    /**
     * Compute only 1pass (-1pass)
     */
    boolean compute_only_1pass;
    /**
     * Enter trellis interactive check routine after boot (-check trellis)
     */
    boolean trellis_check_flag;
    /**
     * Enter triphone existence check routine after boot (-check triphone)
     */
    boolean triphone_check_flag;
    /**
     * Enter lexicon structure consulting mode after boot (-check wchmm)
     */
    boolean wchmm_check_flag;
  } sw;

  /**
   * Variation type of language model: one of LM_NGRAM, LM_DFA_GRAMMAR,
   * LM_DFA_WORD
   * 
   */
  int lmvar;

  /**
   * Language model type: one of LM_UNDEF, LM_NGRAM, LM_DFA
   * 
   */
  int lmtype;


} Jconf;

/**
 * Common memory area to hold model parameters for recognition
 * 
 */
typedef struct __Model__ {

  /**
   * Main phoneme HMM 
   */
  HTK_HMM_INFO *hmminfo;

  /**
   * HMM for Gaussian Selection
   */
  HTK_HMM_INFO *hmm_gs;

  /**
   * GMM for utterance verification
   */
  HTK_HMM_INFO *gmm;

  /**
   * Main Word dictionary for all LM types
   */
  WORD_INFO *winfo;

  /**
   * Main N-gram language model (do not use with grammars)
   */
  NGRAM_INFO *ngram;

  /**
   * List of all loaded grammars (do not use with ngram)
   */
  MULTIGRAM *grammars;

  /**
   * Current maximum value of assigned grammar ID.
   * A new grammar ID will be assigned to each new grammar.
   * 
   */
  int gram_maxid;
  /**
   * Global DFA for recognition.  This will be generated from @a grammars,
   * concatinating each DFA into one.
   */
  DFA_INFO *dfa;

  /**
   * the LM type of this Model holder: will be set from Jconf used for loading
   * 
   */
  int lmtype;

  /**
   * the LM variation type of this Model holder: will be set from
   * Jconf used for loading
   * 
   */
  int lmvar;

} Model;

/**
 * Work area for the first pass
 * 
 */
/*
  How tokens are managed:
   o  tlist[][] is a token stocker.  It holds all tokens in sequencial
      buffer.  They are malloced first on startup, and refered by ID while
      Viterbi procedure.  In word-pair mode, each token also has a link to
      another token to allow a node to have more than 1 token.
      
   o  token[n] holds the current ID number of a token associated to a
      lexicon tree node 'n'.

  */
typedef struct __FSBeam__ {
  /* token stocker */
  TOKEN2 *tlist[2];     ///< Token space to hold all token entities.
  TOKENID *tindex[2];   ///< Token index corresponding to @a tlist for sort
  int maxtnum;          ///< Allocated number of tokens (will grow)
  int expand_step;      ///< Number of tokens to be increased per expansion
  boolean expanded;     ///< TRUE if the tlist[] and tindex[] has been expanded at last create_token();
  int tnum[2];          ///< Current number of tokens used in @a tlist
  int n_start;          ///< Start index of in-beam nodes on @a tindex
  int n_end;            ///< end index of in-beam nodes on @a tindex
  int tl;               ///< Current work area id (0 or 1, swapped for each frame)
  int tn;               ///< Next work area id (0 or 1, swapped for each frame)
    
  /* Active token list */
  TOKENID *token;       ///< Active token list that holds currently assigned tokens for each tree node
#ifdef UNIGRAM_FACTORING
  /* for wordend processing with 1-gram factoring */
  LOGPROB wordend_best_score; ///< Best score of word-end nodes
  int wordend_best_node;	///< Node id of the best wordend nodes
  TRELLIS_ATOM *wordend_best_tre; ///< Trellis word corresponds to above
  WORD_ID wordend_best_last_cword;	///< Last context-aware word of above
#endif

  int totalnodenum;     ///< Allocated number of nodes in @a token
  TRELLIS_ATOM bos;     ///< Special token for beginning-of-sentence
  boolean nodes_malloced; ///< Flag to check if tokens already allocated
  LOGPROB lm_weight;           ///< Language score weight (local copy)
  LOGPROB lm_penalty;          ///< Word insertion penalty (local copy)
  LOGPROB lm_penalty_trans; ///< Additional insertion penalty for transparent words (local copy)
  LOGPROB penalty1; ///< Word insertion penalty for DFA (local copy)
#if defined(WPAIR) && defined(WPAIR_KEEP_NLIMIT)
  boolean wpair_keep_nlimit; ///< Keeps only N token on word-pair approx. (local copy from jconf)
#endif
#ifdef SP_BREAK_CURRENT_FRAME
  boolean in_sparea;         ///< TRUE when we are in a pause area now
  int sparea_start; ///< Determined beginning frame of current processing segment
  int tmp_sparea_start;         ///< Memorize where the current pause area begins
#ifdef SP_BREAK_RESUME_WORD_BEGIN
  WORD_ID tmp_sp_break_last_word; ///< Keep the max word hypothesis at beginning of this segment as the starting word of next segment
#else
  WORD_ID last_tre_word;        ///< Keep ths max word hypothesis at the end of this segment for as the starting word of the next segment
#endif
  boolean first_sparea;  ///< TRUE when we are in the first pause area
  int sp_duration;   ///< Number of current successive sp frame
#endif
  int current_frame_num; ///< num of computed frames
} FSBeam;


/**
 * Work area for realtime processing of 1st pass
 * 
 */
typedef struct __RealBeam__ {
  /* input parameter */
  HTK_Param *param;     ///< Computed MFCC parameter vectors 
  DeltaBuf *db;         ///< Work space for delta MFCC cycle buffer
  DeltaBuf *ab;         ///< Work space for accel MFCC cycle buffer
  VECT *tmpmfcc;                ///< Work space to hold temporal MFCC vector
  int maxframelen;              ///< Maximum allowed input frame length
  int last_time;                ///< Last processed frame

  boolean last_is_segmented; ///<  TRUE if last pass was a segmented input
#ifdef SP_BREAK_CURRENT_FRAME
  SP16 *rest_Speech; ///< Speech samples left unprocessed by segmentation at previous segment
  int rest_alloc_len;   ///< Allocated length of rest_Speech
  int rest_len;         ///< Current stored length of rest_Speech
#endif

  int f;                        ///< Frame pointer where all MFCC computation has been done
  SP16 *window;         ///< Window buffer for MFCC calculation
  int windowlen;                ///< Buffer length of @a window
  int windownum;                ///< Currently left samples in @a window
} RealBeam;

/**
 * Work area for the 2nd pass
 * 
 */
typedef struct __StackDecode__ {
  int hypo_len_count[MAXSEQNUM+1];      ///< Count of popped hypothesis per each length
  int maximum_filled_length; ///< Current least beam-filled depth
#ifdef SCAN_BEAM
  LOGPROB *framemaxscore; ///< Maximum score of each frame on 2nd pass for score enveloping
#endif
  NODE *stocker_root; ///< Node stocker for recycle
  int popctr;           ///< Num of popped hypotheses from stack
  int genectr;          ///< Num of generated hypotheses
  int pushctr;          ///< Num of hypotheses actually pushed to stack
  int finishnum;        ///< Num of found sentence hypothesis
  NODE *current;                ///< Current node for debug

} StackDecode;

/**
 * User LM function entry point
 * 
 */
typedef struct {
  LOGPROB (*uniprob)(WORD_INFO *, WORD_ID, LOGPROB); ///< Pointer to function returning word occurence probability
  LOGPROB (*biprob)(WORD_INFO *, WORD_ID, WORD_ID, LOGPROB); ///< Pointer to function returning a word probability given a word context (corresponds to bi-gram)
  LOGPROB (*lmprob)(WORD_INFO *, WORD_ID *, int, WORD_ID, LOGPROB); ///< Pointer to function returning LM probability
} LMFunc;

/**
 * Work area for GMM calculation
 * 
 */
typedef struct __gmm_calc__{
  LOGPROB *gmm_score;   ///< Current accumurated scores for each GMM
  int framecount;               ///< Current frame count
  LOGPROB *OP_calced_score; ///< Work area for Gaussian pruning on GMM: scores
  int *OP_calced_id; ///< Work area for Gaussian pruning on GMM: id
  int OP_calced_num; ///< Work area for Gaussian pruning on GMM: number of above
  int OP_calced_maxnum; ///< Work area for Gaussian pruning on GMM: size of allocated area
  int OP_gprune_num; ///< Number of Gaussians to be computed in Gaussian pruning
  VECT *OP_vec;         ///< Local workarea to hold the input vector of current frame
  short OP_veclen;              ///< Local workarea to hold the length of above
  HTK_HMM_Data *max_d;  ///< Hold model of the maximum score
#ifdef CONFIDENCE_MEASURE
  LOGPROB gmm_max_cm;   ///< Hold maximum score
#endif
} GMMCalc;

/**
 * Output information structure
 * 
 */
typedef struct __sentence__ {
  WORD_ID *word;                ///< Sequence of word ID 
  int word_num;                 ///< Number of words in the sentence
  LOGPROB score;                ///< Likelihood (LM+AM)
  LOGPROB *confidence;          ///< Word confidence scores
  LOGPROB score_lm;             ///< Language model likelihood (scaled) for N-gram
  LOGPROB score_am;             ///< Acoustic model likelihood for N-gram
  int gram_id;                  ///< The grammar ID this sentence belongs to for DFA

  /**
   * Alignment result, valid when forced alignment was done
   * 
   */
  struct {
    boolean filled;             ///< True if has data
    int num;                    ///< Number of units
    short unittype;             ///< Unit type (one of PER_*)

    WORD_ID *w;                 ///< word sequence by id (PER_WORD)
    HMM_Logical **ph;     ///< Phone sequence (PER_PHONEME, PER_STATE)
    short *loc; ///< sequence of state location in a phone (PER_STATE)
    boolean *is_iwsp;           ///< TRUE if PER_STATE and this is the inter-word pause state at multipath mode

    int *begin_frame;           ///< List of beginning frame
    int *end_frame;             ///< List of ending frame
    LOGPROB *avgscore;          ///< Score averaged by frames
   
    LOGPROB allscore;           ///< Re-computed acoustic score
  } align;

} Sentence;

typedef struct __adin__ {
  /* functions */
  /// Pointer to function for device initialization (call once on startup)
  boolean (*ad_standby)(int, void *);
  /// Pointer to function to open audio stream for capturing
  boolean (*ad_begin)();
  /// Pointer to function to close audio stream capturing
  boolean (*ad_end)();
  ///< Pointer to function to begin / restart recording
  boolean (*ad_resume)();
  ///< Pointer to function to pause recording
  boolean (*ad_pause)();
  ///< Pointer to function to read samples
  int (*ad_read)(SP16 *, int);

  /* configuration parameters */
  int thres;            ///< Input Level threshold (0-32767)
  int noise_zerocross;  ///< Computed threshold of zerocross num in the cycle buffer
  int nc_max;           ///< Computed number of fragments for tail margin
  boolean adin_cut_on;  ///< TRUE if do input segmentation by silence
  boolean silence_cut_default; ///< Device-dependent default value of adin_cut_on()
  boolean strip_flag;   ///< TRUE if skip invalid zero samples
  boolean enable_thread;        ///< TRUE if input device needs threading
  boolean ignore_speech_while_recog; ///< TRUE if ignore speech input between call, while waiting recognition process
  boolean need_zmean;   ///< TRUE if perform zmeansource

  /* work area */
  int c_length; ///< Computed length of cycle buffer for zero-cross, actually equals to head margin length
  int c_offset; ///< Static data DC offset (obsolute, should be 0)
  SP16 *swapbuf;                ///< Buffer for re-triggering in tail margin
  int sbsize, sblen;    ///< Size and current length of @a swapbuf
  int rest_tail;                ///< Samples not processed yet in swap buffer

  /* work area for zero-cross computation */
  ZEROCROSS zc;

#ifdef HAVE_PTHREAD
  /* Variables related to POSIX threading */
  pthread_mutex_t mutex;        ///< Lock primitive
  SP16 *speech;         ///< Unprocessed samples recorded by A/D-in thread
  int speechlen;                ///< Current length of @a speech
/*
 * @brief  Semaphore to start/stop recognition.
 * 
 * If TRUE, A/D-in thread will store incoming samples to @a speech and
 * main thread will detect and process them.
 * If FALSE, A/D-in thread will still get input and check trigger as the same
 * as TRUE case, but does not store them to @a speech.
 * 
 */
  boolean transfer_online;      
  boolean adinthread_buffer_overflowed;
#endif

  /* Input data buffer */
  SP16 *buffer; ///< Temporary buffer to hold input samples
  int bpmax;            ///< Maximum length of @a buffer
  int bp;                       ///< Current point to store the next data
  int current_len;              ///< Current length of stored samples
  SP16 *cbuf;           ///< Buffer for flushing cycle buffer just after detecting trigger 
  boolean down_sample; ///< TRUE if perform down sampling from 48kHz to 16kHz
  SP16 *buffer48; ///< Another temporary buffer to hold 48kHz inputs
  int io_rate; ///< frequency rate (should be 3 always for 48/16 conversion

  boolean is_valid_data;        ///< TRUE if we are now triggered
  int nc;               /* count of current tail silence segments */
  boolean end_of_stream;        
  boolean need_init;    /* if TRUE, initialize buffer on startup */

  /* filter buffer for 48-to-16 conversion */
  DS_BUFFER *ds;

} ADIn;

/**
 * Recognition result output structure.  You may want to use with model data
 * to get fully detailed results.
 * 
 */
typedef struct __Output__ {
  /**
   * 1: recognition in progress
   * 0: recognition succeeded (at least one candidate has been found)
   * -1: search failed, no candidate has been found
   * -2: input rejected by short input
   * -3: input rejected by GMM
   * 
   */
  int status;

  int num_frame;                ///< Number of frames of the recognized part
  int length_msec;              ///< Length of the recognized part

  Sentence *sent;               ///< List of (N-best) recognition result sentences
  int sentnum;                  ///< Number of sentences

  WordGraph *wg1;               ///< List of word graph generated on 1st pass
  int wg1_num;                  ///< Num of words in the wg1

  WordGraph *wg;                ///< List of word graph

  CN_CLUSTER *confnet;		///< List of confusion network clusters

  Sentence pass1;               ///< Recognition result on the 1st pass

} Output;  
  
/**
 * Work area for recognition process
 * 
 */
typedef struct __Recog__ {

  /*******************************************/
  /**
   * Models to be used at a recognition
   * 
   */
  Model *model;

  /*******************************************/
  /**
   * User-specified configuration parameters
   * 
   */
  Jconf *jconf;

  /*******************************************/
  /**
   * A/D-in buffers
   * 
   */
  ADIn *adin;

  /**
   * Parameter extraction work area
   * 
   */
  MFCCWork *mfccwrk;

  /**
   * Parameter extraction work area for spectral subtraction
   * 
   */
  MFCCWork *mfccwrk_ss;

  /**
   * TRUE if CMN parameter loaded from file at boot up
   */
  boolean cmn_loaded;

  /*******************************************/
  /* work area for search and result */

  /**
   * Word-conjunction HMM as tree lexicon
   */
  WCHMM_INFO *wchmm;

  /**
   * Actual beam width of 1st pass (will be set on startup)
   */
  int trellis_beam_width;

  /**
   * Word trellis index generated at the 1st pass
   */
  BACKTRELLIS *backtrellis;

  /**
   * Work area and outprob cache for HMM output probability computation
   */
  HMMWork hmmwrk;

  /**
   * Work area for the first pass
   */
  FSBeam pass1;

  /**
   * Work area for the realtime processing of first pass
   */
  RealBeam real;

  /**
   * Work area for second pass
   * 
   */
  StackDecode pass2;

  /**
   * Maximum score of best hypothesis at 1st pass
   */
  LOGPROB backmax;

  /**
   * Word sequence of best hypothesis on 1st pass
   */
  WORD_ID pass1_wseq[MAXSEQNUM];

  /**
   * Number of words in @a pass1_wseq
   */
  int pass1_wnum;

  /**
   * Score of @a pass1_wseq
   */
  LOGPROB pass1_score;

  /**
   * Whether handle phone context dependency (local copy from jconf)
   */
  boolean ccd_flag;             

#ifdef SP_BREAK_CURRENT_FRAME

  /**
   * TRUE when engine is processing a segment
   * 
   */
  boolean process_segment;

  /**
   * Rest parameter for next segment
   */
  HTK_Param *rest_param;
  /**
   * Last maximum word hypothesis on the begin point
   */
  WORD_ID sp_break_last_word;
  /**
   * Last (not transparent) context word for LM
   */
  WORD_ID sp_break_last_nword;
  /**
   * Allow override of last context word from result of 2nd pass
   */
  boolean sp_break_last_nword_allow_override;
  /**
   * Search start word on 2nd pass
   */
  WORD_ID sp_break_2_begin_word;
  /**
   * Search end word on 2nd pass
   */
  WORD_ID sp_break_2_end_word;
#endif

  /*******************************************/
  /* inputs */

  /**
   * Input speech data
   */
  SP16 speech[MAXSPEECHLEN];
  /**
   * Input length in samples
   */
  int speechlen;                

  /**
   * Input length in frames
   */
  int peseqlen;         


  /*******************************************/
  /* GMM */
  /**
   * Work area for GMM calculation
   * 
   */
  GMMCalc *gc;

  /*******************************************/
  /* misc. */

  /**
   * Estimated noise spectrum
   */
  float *ssbuf;

  /**
   * Length of @a ssbuf
   */
  int sslen;

  /**
   * GraphOut: total number of words in the generated graph
   */
  int graph_totalwordnum;

  /**
   * Status flag indicating whether the recognition is alive or not.  If
   * TRUE, the process is currently activated, either monitoring an
   * audio input or recognizing the current input.  If FALSE, the recognition
   * is now disabled until some activation command has been arrived from
   * client.  While disabled, all the inputs are ignored.
   *
   * If set to FALSE in the program, Julius/Julian will stop after
   * the current recognition ends, and enter the disabled status.
   * 
   */
  boolean process_active;

  /**
   * If set to TRUE, Julius/Julian stops recognition immediately, terminating
   * the currenct recognition process, and enter into disabled status.
   * 
   */
  boolean process_want_terminate;

  /**
   * If set to TRUE, Julius/Julian stops recognition softly.  If it is
   * performing recognition of the 1st pass, it immediately segments the
   * current input, process the 2nd pass, and output the result.  Then it
   * enters the disabled status.
   * 
   */
  boolean process_want_reload;

  /**
   * When to refresh the global lexicon if received while recognition for
   * DFA
   * 
   */
  short gram_switch_input_method;

  /**
   * TRUE if audio stream is now open and engine is either listening
   * audio stream or recognizing a speech.  FALSE on startup or when
   * in pause specified by a module command.
   * 
   */
  boolean process_online;

  /**
   * Recognition results
   * 
   */
  Output result;

  /**
   * Parameter vector sequence to be recognized
   * 
   */
  HTK_Param *param;

  /**
   * LM type: will be set from value from model->lmtype
   * 
   */
  int lmtype;

  /**
   * LM variation type: will be set from value from model->lmvar
   * 
   */
  int lmvar;

  /**
   * LM User function entry point
   * 
   */
  LMFunc lmfunc;

  /**
   * graphout: will be set from value from jconf->graph.enabled
   * 
   */
  boolean graphout;

  /**
   * multi-path mode
   * 
   */
  boolean multipath;

  /**
   * Function pointer to parameter vector computation for realtime 1st pass.
   * default: RealTimeMFCC() in realtime-1stpass.c
   * 
   */
  boolean (*calc_vector)(VECT *, SP16 *, int, Value *, struct __Recog__ *);

  /**
   * Callback entry point
   * 
   */
  void (*callback_function[SIZEOF_CALLBACK_ID][MAX_CALLBACK_HOOK])();
  /**
   * Callback user data
   * 
   */
  void *callback_user_data[SIZEOF_CALLBACK_ID][MAX_CALLBACK_HOOK];
  /**
   * Numbers of callbacks registered
   * 
   */
  int callback_function_num[SIZEOF_CALLBACK_ID];
  /**
   * Callback function code list
   * 
   */
  int callback_list_code[MAX_CALLBACK_HOOK*SIZEOF_CALLBACK_ID];
  /**
   * Callback function location list
   * 
   */
  int callback_list_loc[MAX_CALLBACK_HOOK*SIZEOF_CALLBACK_ID];
  /**
   * Number of callbacks
   * 
   */
  int callback_num;

  /*******************************************/

  /**
   * User-defined data hook.  JuliusLib does not concern about its content.
   * 
   */
  void *hook;

} Recog;

#endif /* __J_COMMON_H__ */

/*

=======================================================
  Variable name mapping from old global.h to common.h
=======================================================

result_reorder_flag -> DELETED
adinnet_port ->jconf.input.adinnet_port
align_result_phoneme_flag ->jconf.annotate.align_result_phoneme_flag
align_result_state_flag ->jconf.annotate.align_result_state_flag
align_result_word_flag ->jconf.annotate.align_result_word_flag
backmax ->recog.backmax
backtrellis ->recog.backtrellis
ccd_flag ->jconf.am.ccd_flag
ccd_flag_force ->jconf.am.ccd_flag_force
cm_alpha ->jconf.annotate.cm_alpha
cm_alpha_bgn ->jconf.annotate.cm_alpha_bgn
cm_alpha_end ->jconf.annotate.cm_alpha_end
cm_alpha_num ->jconf.annotate.cm_alpha_num
cm_alpha_step ->jconf.annotate.cm_alpha_step
cm_cut_thres ->jconf.annotate.cm_cut_thres
cm_cut_thres_pop ->jconf.annotate.cm_cut_thres_pop
cmn_loaded ->recog.cmn_loaded
cmn_map_weight ->jconf.frontend.cmn_map_weight
cmn_update ->jconf.frontend.cmn_update
cmnload_filename ->jconf.frontend.cmnload_filename
cmnsave_filename ->jconf.frontend.cmnsave_filename
compute_only_1pass ->jconf.sw.compute_only_1pass
dfa ->model.dfa
dfa_filename ->jconf.lm.dfa_filename
dictfilename ->jconf.lm.dictfilename
enable_iwsp ->jconf.lm.enable_iwsp
enable_iwspword ->jconf.lm.enable_iwspword
enveloped_bestfirst_width ->jconf.search.pass2.enveloped_bestfirst_width
force_realtime_flag ->jconf.search.pass1.force_realtime_flag
forced_realtime ->jconf.search.pass1.forced_realtime
forcedict_flag ->jconf.lm.forcedict_flag
framemaxscore ->recog.framemaxscore
from_code ->jconf.output.from_code
gmm ->model.gmm
gmm_filename ->jconf.reject.gmm_filename
gmm_gprune_num ->jconf.reject.gmm_gprune_num
gmm_reject_cmn_string ->jconf.reject.gmm_reject_cmn_string
gprune_method ->jconf.am.gprune_method
gramlist ->model.grammars
gramlist_root ->jconf.lm.gramlist_root
graph_merge_neighbor_range ->jconf.graph.graph_merge_neighbor_range
graph_totalwordnum ->recog.graph_totalwordnum
graphout_cut_depth ->jconf.graph.graphout_cut_depth
graphout_limit_boundary_loop_num ->jconf.graph.graphout_limit_boundary_loop_num
graphout_search_delay ->jconf.graph.graphout_search_delay
gs_statenum ->jconf.am.gs_statenum
head_margin_msec ->jconf.detect.head_margin_msec
head_silname ->jconf.lm.head_silname
hmm_gs ->model.hmm_gs
hmm_gs_filename ->jconf.am.hmm_gs_filename
hmmfilename ->jconf.am.hmmfilename
hmminfo ->model.hmminfo
hypo_overflow ->jconf.search.pass2.hypo_overflow
inputlist_filename ->jconf.input.inputlist_filename
iw_cache_rate ->jconf.search.pass1.iw_cache_rate
iwcdmaxn ->jconf.search.pass1.iwcdmaxn
iwcdmethod ->jconf.search.pass1.iwcdmethod
iwsp_penalty ->jconf.lm.iwsp_penalty
iwspentry ->jconf.lm.iwspentry
level_thres ->jconf.detect.level_thres
lm_penalty ->jconf.lm.lm_penalty
lm_penalty2 ->jconf.lm.lm_penalty2
lm_penalty_trans ->jconf.lm.lm_penalty_trans
lm_weight ->jconf.lm.lm_weight
lm_weight2 ->jconf.lm.lm_weight2
lmp_specified ->jconf.lm.lmp_specified
lmp2_specified ->jconf.lm.lmp2_specified
looktrellis_flag ->jconf.search.pass2.looktrellis_flag
lookup_range ->jconf.search.pass2.lookup_range
mapfilename ->jconf.am.mapfilename
mixnum_thres ->jconf.am.mixnum_thres
module_mode -> (app)
module_port -> (app)
module_sd -> (app)
multigramout_flag ->jconf.output.multigramout_flag
nbest ->jconf.search.pass2.nbest
netaudio_devname ->jconf.input.netaudio_devname
ngram ->model.ngram
ngram_filename ->jconf.lm.ngram_filename
ngram_filename_lr_arpa ->jconf.lm.ngram_filename_lr_arpa
ngram_filename_rl_arpa ->jconf.lm.ngram_filename_rl_arpa
old_iwcd_flag -> USE_OLD_IWCD (define.h)
old_tree_function_flag ->jconf.search.pass1.old_tree_function_flag
output_hypo_maxnum ->jconf.output.output_hypo_maxnum
para ->jconf.analysis.para
para_default ->jconf.analysis.para_default
para_hmm ->jconf.analysis.para_hmm
para_htk ->jconf.analysis.para_htk
paramtype_check_flag ->jconf.analysis.paramtype_check_flag
pass1_score ->recog.pass1_score
pass1_wnum ->recog.pass1_wnum
pass1_wseq ->recog.pass1_wseq
penalty1 ->jconf.lm.penalty1
penalty2 ->jconf.lm.penalty2
peseqlen ->recog.peseqlen
progout_flag ->jconf.output.progout_flag
progout_interval ->jconf.output.progout_interval
progout_interval_frame (beam.c) ->jconf.output.progout_interval
realtime_flag ->jconf.search.pass1.realtime_flag
record_dirname ->jconf.output.record_dirname
rejectshortlen ->jconf.reject.rejectshortlen
rest_param ->recog.rest_param
result_output -> (app)
scan_beam_thres ->jconf.search.pass2.scan_beam_thres
separate_score_flag ->jconf.output.separate_score_flag
separate_wnum ->jconf.search.pass1.separate_wnum
silence_cut ->jconf.detect.silence_cut
sp_break_2_begin_word ->recog.sp_break_2_begin_word
sp_break_2_end_word ->recog.sp_break_2_end_word
sp_break_last_nword ->recog.sp_break_last_nword
sp_break_last_nword_allow_override ->recog.sp_break_last_nword_allow_override
sp_break_last_word ->recog.sp_break_last_word
sp_frame_duration ->jconf.successive.sp_frame_duration
specified_trellis_beam_width ->jconf.search.pass1.specified_trellis_beam_width
speech ->recog.speech
speech_input ->jconf.input.speech_input
speechlen ->recog.speechlen
spmodel_name ->jconf.am.spmodel_name
ssbuf ->recog.ssbuf
sscalc ->jconf.frontend.sscalc
sscalc_len ->jconf.frontend.sscalc_len
sslen ->recog.sslen
ssload_filename ->jconf.frontend.ssload_filename
stack_size ->jconf.search.pass2.stack_size
strip_zero_sample ->jconf.frontend.strip_zero_sample
tail_margin_msec ->jconf.detect.tail_margin_msec
tail_silname ->jconf.lm.tail_silname
to_code ->jconf.output.to_code
trellis_beam_width ->recog.trellis_beam_width
trellis_check_flag ->jconf.sw.trellis_check_flag
triphone_check_flag ->jconf.sw.triphone_check_flag
use_ds48to16 ->jconf.input.use_ds48to16
use_zmean ->jconf.frontend.use_zmean
wchmm ->recog.wchmm
wchmm_check_flag ->jconf.sw.wchmm_check_flag
winfo ->model.winfo
wpair_keep_nlimit ->jconf.search.pass1.wpair_keep_nlimit
zero_cross_num ->jconf.detect.zero_cross_num

verbose_flag -> (remain in global.h)
debug2_flag -> (remain in global.h)

*/
 
