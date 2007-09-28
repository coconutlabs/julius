/**
 * @file   realtime-1stpass.c
 * @author Akinobu Lee
 * @date   Tue Aug 23 11:44:14 2005
 * 
 * <JA>
 * @brief  実時間認識のための第1パスの平行処理
 *
 * 第1パスを入力開始と同時にスタートし，入力と平行して認識処理を行うための
 * 関数が定義されている．
 * 
 * 通常，Julius の音声認識処理は以下の手順で main_recognition_loop() 内で
 * 実行される．
 *
 *  -# 音声入力 adin_go()  → 入力音声が speech[] に格納される
 *  -# 特徴量抽出 new_wav2mfcc() →speechから特徴パラメータを param に格納
 *  -# 第1パス実行 get_back_trellis() →param とモデルから単語トレリスの生成
 *  -# 第2パス実行 wchmm_fbs()
 *  -# 認識結果出力
 *
 * 第1パスを平行処理する場合，上記の 1 〜 3 が平行して行われる．
 * Julius では，この並行処理を，音声入力の断片が得られるたびに
 * 認識処理をその分だけ漸次的に進めることで実装している．
 * 
 *  - 特徴量抽出と第1パス実行を，一つにまとめてコールバック関数として定義．
 *  - 音声入力関数 adin_go() のコールバックとして上記の関数を与える
 *
 * 具体的には，ここで定義されている RealTimePipeLine() がコールバックとして
 * adin_go() に与えられる．adin_go() は音声入力がトリガするとその得られた入力
 * 断片ごとに RealTimePipeLine() を呼び出す．RealTimePipeLine() は得られた
 * 断片分について特徴量抽出と第1パスの計算を進める．
 *
 * CMN について注意が必要である．CMN は通常発話単位で行われるが，
 * マイク入力やネットワーク入力のように，第1パスと平行に認識を行う
 * 処理時は発話全体のケプストラム平均を得ることができない．バージョン 3.5
 * 以前では直前の発話5秒分(棄却された入力を除く)の CMN がそのまま次発話に
 * 流用されていたが，3.5.1 からは，上記の直前発話 CMN を初期値として
 * 発話内 CMN を MAP-CMN を持ちいて計算するようになった．なお，
 * 最初の発話用の初期CMNを "-cmnload" で与えることもでき，また
 * "-cmnnoupdate" で入力ごとの CMN 更新を行わないようにできる．
 * "-cmnnoupdate" と "-cmnload" と組み合わせることで, 最初にグローバルな
 * ケプストラム平均を与え，それを常に初期値として MAP-CMN することができる．
 *
 * 主要な関数は以下の通りである．
 *
 *  - RealTimeInit() - 起動時の初期化
 *  - RealTimePipeLinePrepare() - 入力ごとの初期化
 *  - RealTimePipeLine() - 第1パス平行処理用コールバック（上述）
 *  - RealTimeResume() - ショートポーズセグメンテーション時の認識復帰
 *  - RealTimeParam() - 入力ごとの第1パス終了処理
 *  - RealTimeCMNUpdate() - CMN の更新
 *  
 * </JA>
 * 
 * <EN>
 * @brief  On-the-fly decoding of the 1st pass.
 *
 * These are functions to perform on-the-fly decoding of the 1st pass
 * (frame-synchronous beam search).  These function can be used
 * instead of new_wav2mfcc() and get_back_trellis().  These functions enable
 * recognition as soon as an input triggers.  The 1st pass processing
 * will be done concurrently with the input.
 *
 * The basic recognition procedure of Julius in main_recognition_loop()
 * is as follows:
 *
 *  -# speech input: (adin_go())  ... buffer `speech' holds the input
 *  -# feature extraction: (new_wav2mfcc()) ... compute feature vector
 *     from `speech' and store the vector sequence to `param'.
 *  -# recognition 1st pass: (get_back_trellis()) ... frame-wise beam decoding
 *     to generate word trellis index from `param' and models.
 *  -# recognition 2nd pass: (wchmm_fbs())
 *  -# Output result.
 *
 * At on-the-fly decoding, procedures from 1 to 3 above will be performed
 * in parallel.  It is implemented by a simple scheme, processing the captured
 * small speech fragments one by one progressively:
 *
 *  - Define a callback function that can do feature extraction and 1st pass
 *    processing progressively.
 *  - The callback will be given to A/D-in function adin_go().
 *
 * Actual procedure is as follows. The function RealTimePipeLine()
 * will be given to adin_go() as callback.  Then adin_go() will watch
 * the input, and if speech input starts, it calls RealTimePipeLine()
 * for every captured input fragments.  RealTimePipeLine() will
 * compute the feature vector of the given fragment and proceed the
 * 1st pass processing for them, and return to the capture function.
 * The current status will be hold to the next call, to perform
 * inter-frame processing (computing delta coef. etc.).
 *
 * Note about CMN: With acoustic models trained with CMN, Julius performs
 * CMN to the input.  On file input, the whole sentence mean will be computed
 * and subtracted.  At the on-the-fly decoding, the ceptral mean will be
 * performed using the cepstral mean of last 5 second input (excluding
 * rejected ones).  This was a behavier earlier than 3.5, and 3.5.1 now
 * applies MAP-CMN at on-the-fly decoding, using the last 5 second cepstrum
 * as initial mean.  Initial cepstral mean at start can be given by option
 * "-cmnload", and you can also prohibit the updates of initial cepstral
 * mean at each input by "-cmnnoupdate".  The last option is useful to always
 * use static global cepstral mean as initial mean for each input.
 *
 * The primary functions in this file are:
 *  - RealTimeInit() - initialization at application startup
 *  - RealTimePipeLinePrepare() - initialization before each input
 *  - RealTimePipeLine() - callback for on-the-fly 1st pass decoding
 *  - RealTimeResume() - recognition resume procedure for short-pause segmentation.
 *  - RealTimeParam() - finalize the on-the-fly 1st pass when input ends.
 *  - RealTimeCMNUpdate() - update CMN data for next input
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

#undef RDEBUG			///< Define if you want local debug message

/* 計算結果の MFCC を保存する param 構造体を準備する
   これは1回の認識ごとに繰り返し呼ばれる */
/* prepare new parameter vector holder for RealTime*
   This will be called each time a recognition begins */
/** 
 * <JA>
 * 漸次的に計算される特徴ベクトル列を保持するための領域を準備する．
 * これは入力/認識1回ごとに繰り返し呼ばれる．
 * </JA>
 * <EN>
 * Prepare parameter vector holder to incrementally store the
 * calculated MFCC vectors.  This function will be called each time
 * after a recognition ends and new input begins.
 * </EN>
 */
static void
init_param(RealBeam *r, Value *para)
{
  /* これから計算されるパラメータの型をヘッダに設定 */
  /* set header types */
  r->param->header.samptype = F_MFCC;
  if (para->delta) r->param->header.samptype |= F_DELTA;
  if (para->acc) r->param->header.samptype |= F_ACCL;
  if (para->energy) r->param->header.samptype |= F_ENERGY;
  if (para->c0) r->param->header.samptype |= F_ZEROTH;
  if (para->absesup) r->param->header.samptype |= F_ENERGY_SUP;
  if (para->cmn) r->param->header.samptype |= F_CEPNORM;

  r->param->header.wshift = para->smp_period * para->frameshift;
  r->param->header.sampsize = para->veclen * sizeof(VECT); /* not compressed */
  r->param->veclen = para->veclen;
  /* フレームごとのパラメータベクトル保存の領域を確保 */
  /* あとで必要に応じて伸長される */
  if (param_alloc(r->param, 1, r->param->veclen) == FALSE) {
    j_internal_error("ERROR: segmented: failed to allocate memory for rest param\n");
  }

  /* 認識処理中/終了後にセットされる変数:
     param->parvec (パラメータベクトル系列)
     param->header.samplenum, param->samplenum (全フレーム数)
  */
  /* variables that will be set while/after computation has been done:
     param->parvec (parameter vector sequence)
     param->header.samplenum, param->samplenum (total number of frames)
  */
}

/** 
 * <JA>
 * 第1パス平行認識処理の初期化（起動後1回だけ呼ばれる）
 * </JA>
 * <EN>
 * Initializations for on-the-fly 1st pass decoding (will be called once
 * on startup)
 * </EN>
 */
boolean
RealTimeInit(Recog *recog)
{
  Value *para;
  Jconf *jconf;
  RealBeam *r;

  jconf = recog->jconf;
  para = &(jconf->analysis.para);
  r = &(recog->real);

  /* -ssload 指定時, SS用のノイズスペクトルをファイルから読み込む */
  /* if "-ssload", load noise spectrum for spectral subtraction from file */
  if (jconf->frontend.ssload_filename && recog->ssbuf == NULL) {
    if ((recog->ssbuf = new_SS_load_from_file(jconf->frontend.ssload_filename, &recog->sslen)) == NULL) {
      jlog("ERROR: failed to read \"%s\"\n", jconf->frontend.ssload_filename);
      return FALSE;
    }
    /* check ssbuf length */
    if (recog->sslen != recog->mfccwrk->bflen) {
      jlog("ERROR: noise spectrum length not match\n");
      return FALSE;
    }
    recog->mfccwrk->ssbuf = recog->ssbuf;
    recog->mfccwrk->ssbuflen = recog->sslen;
  }
  /* 対数エネルギー正規化のための初期値 */
  /* set initial value for log energy normalization */
  if (para->energy && para->enormal) energy_max_init();
  /* デルタ計算のためのサイクルバッファを用意 */
  /* initialize cycle buffers for delta and accel coef. computation */
  if (para->delta) r->db = WMP_deltabuf_new(para->baselen, para->delWin);
  if (para->acc) r->ab = WMP_deltabuf_new(para->baselen * 2, para->accWin);
  /* デルタ計算のためのワークエリアを確保 */
  /* allocate work area for the delta computation */
  r->tmpmfcc = (VECT *)mymalloc(sizeof(VECT) * para->vecbuflen);
  /* 最大フレーム長を最大入力時間数から計算 */
  /* set maximum allowed frame length */
  r->maxframelen = MAXSPEECHLEN / para->frameshift;
  /* 窓長をセット */
  /* set window length */
  r->windowlen = para->framesize + 1;
  /* 窓かけ用バッファを確保 */
  /* set window buffer */
  r->window = mymalloc(sizeof(SP16) * r->windowlen);
  /* MAP-CMN 用の初期ケプストラム平均を読み込んで初期化する */
  /* Initialize the initial cepstral mean data from file for MAP-CMN */
  if (para->cmn) CMN_realtime_init(para->mfcc_dim, jconf->frontend.cmn_map_weight);
  /* -cmnload 指定時, CMN用のケプストラム平均の初期値をファイルから読み込む */
  /* if "-cmnload", load initial cepstral mean data from file for CMN */
  if (jconf->frontend.cmnload_filename) {
    if (para->cmn) {
      if ((recog->cmn_loaded = CMN_load_from_file(jconf->frontend.cmnload_filename, para->mfcc_dim))== FALSE) {
	jlog("WARNING: failed to read initial cepstral mean from \"%s\", do flat start\n", jconf->frontend.cmnload_filename);
      }
    } else {
      jlog("WARNING: CMN not required on AM, file \"%s\" ignored\n", jconf->frontend.cmnload_filename);
    }
  }

  return TRUE;
}

/* ON-THE-FLY デコーディング関数: 準備 (認識開始時ごとに呼ばれる) */
/* ON-THE-FLY DECODING FUNCTION: prepare (on start of every input segment) */
/** 
 * <JA>
 * 第1パス平行認識処理のデータ準備（認識開始ごとに呼ばれる）
 * </JA>
 * <EN>
 * Data preparation for on-the-fly 1st pass decoding (will be called on the
 * start of each sentence input)
 * </EN>
 */
boolean
RealTimePipeLinePrepare(Recog **recoglist, int recognum)
{
  Value *para;
  RealBeam *r;
  Recog *recog;
  int i;

  recog = recoglist[0];

  para = &(recog->jconf->analysis.para);
  r = &(recog->real);
  /* param 構造体は Recog と共有する */
  /* share allocate parameter data with Recog*/
  r->param = recog->param;
  init_param(r, para);
  /* 対数エネルギー正規化のための初期値をセット */
  /* set initial value for log energy normalization */
  if (para->energy && para->enormal) energy_max_prepare(para);
  /* デルタ計算用バッファを準備 */
  /* set the delta cycle buffer */
  if (para->delta) WMP_deltabuf_prepare(r->db);
  if (para->acc) WMP_deltabuf_prepare(r->ab);
  /* 準備した param 構造体のデータのパラメータ型を音響モデルとチェックする */
  /* check type coherence between param and hmminfo here */
  if (!check_param_coherence(recog->model->hmminfo, r->param)) {
    jlog("ERROR: input parameter type does not match AM\n");
    return FALSE;
  }
  /* 計算用の変数を初期化 */
  /* initialize variables for computation */
  r->f = 0;
  r->windownum = 0;
  /* MAP-CMN の初期化 */
  /* Prepare for MAP-CMN */
  if (para->cmn) CMN_realtime_prepare();
  /* 音響尤度計算用キャッシュを準備
     最大長をここであらかじめ確保してしまう */
  /* prepare cache area for acoustic computation of HMM states and mixtures
     pre-fetch for maximum length here */
  for(i=0;i<recognum;i++) {
    outprob_prepare(&(recoglist[i]->hmmwrk), r->maxframelen);
  }

  return TRUE;
}

/** 
 * <JA>
 * 窓単位で取り出された音声波形からパラメータベクトルを計算する。
 * 
 * @param tmpmfcc [out] 計算されたパラメータベクトルが保存されるバッファ
 * @param window [in] 窓単位で取り出された音声波形データ
 * @param windowlen [in] @a window の長さ
 * @param para [in] 音響特徴量抽出条件パラメータ
 * @param re [in] 認識インスタンス (re->mfccwrk が内部で使用される)
 * 
 * @return 成功時、@a tmpmfcc に計算結果のパラメータベクトルが保存され、TRUE
 * を返す。失敗時（デルタ計算で入力フレームが少ないなど）は FALSE を返す。
 * </JA>
 * <EN>
 * Compute a parameter vector from a speech window.
 * 
 * @param tmpmfcc [out] resulting parameter vector will be written to this.
 * @param window [in] windowed speech input segment
 * @param windowlen [in] length of @a window
 * @param para [in] acoustic analysis parameters
 * @param re [i/o] recognition instance (re->mfccwrk will be used inside)
 * 
 * @return TRUE on success with resulting parameter vector on @a tmpmfcc,
 * or FALSE if no parameter vector obtained.
 * </EN>
 */
boolean
RealTimeMFCC(VECT *tmpmfcc, SP16 *window, int windowlen, Value *para, Recog *re)
{
  RealBeam *r;
  int i;
  boolean ret;

  r = &(re->real);

  /* 音声波形から base MFCC を計算 (recog->mfccwrk を利用) */
  /* calculate base MFCC from waveform (use recog->mfccwrk) */
  for (i=0; i < windowlen; i++) {
    re->mfccwrk->bf[i+1] = (float) window[i];
  }
  WMP_calc(re->mfccwrk, tmpmfcc, re->jconf->analysis.para);

  if (para->energy && para->enormal) {
    /* 対数エネルギー項を正規化する */
    /* normalize log energy */
    /* リアルタイム入力では発話ごとの最大エネルギーが得られないので
       直前の発話のパワーで代用する */
    /* Since the maximum power of the whole input utterance cannot be
       obtained at real-time input, the maximum of last input will be
       used to normalize.
    */
    tmpmfcc[para->baselen-1] = energy_max_normalize(tmpmfcc[para->baselen-1], para);
  }

  if (para->delta) {
    /* デルタを計算する */
    /* calc delta coefficients */
    ret = WMP_deltabuf_proceed(r->db, tmpmfcc);
#ifdef RDEBUG
    printf("DeltaBuf: ret=%d, status=", ret);
    for(i=0;i<r->db->len;i++) {
      printf("%d", r->db->is_on[i]);
    }
    printf(", nextstore=%d\n", r->db->store);
#endif
    /* ret == FALSE のときはまだディレイ中なので認識処理せず次入力へ */
    /* if ret == FALSE, there is no available frame.  So just wait for
       next input */
    if (! ret) {
      return FALSE;
    }

    /* db->vec に現在の元データとデルタ係数が入っているので tmpmfcc にコピー */
    /* now db->vec holds the current base and full delta, so copy them to tmpmfcc */
    memcpy(tmpmfcc, r->db->vec, sizeof(VECT) * para->baselen * 2);
  }

  if (para->acc) {
    /* Accelerationを計算する */
    /* calc acceleration coefficients */
    /* base+delta をそのまま入れる */
    /* send the whole base+delta to the cycle buffer */
    ret = WMP_deltabuf_proceed(r->ab, tmpmfcc);
#ifdef RDEBUG
    printf("AccelBuf: ret=%d, status=", ret);
    for(i=0;i<r->ab->len;i++) {
      printf("%d", r->ab->is_on[i]);
    }
    printf(", nextstore=%d\n", r->ab->store);
#endif
    /* ret == FALSE のときはまだディレイ中なので認識処理せず次入力へ */
    /* if ret == FALSE, there is no available frame.  So just wait for
       next input */
    if (! ret) {
      return FALSE;
    }
    /* ab->vec には，(base+delta) とその差分係数が入っている．
       [base] [delta] [delta] [acc] の順で入っているので,
       [base] [delta] [acc] を tmpmfcc にコピーする．*/
    /* now ab->vec holds the current (base+delta) and their delta coef. 
       it holds a vector in the order of [base] [delta] [delta] [acc], 
       so copy the [base], [delta] and [acc] to tmpmfcc.  */
    memcpy(tmpmfcc, r->ab->vec, sizeof(VECT) * para->baselen * 2);
    memcpy(&(tmpmfcc[para->baselen*2]), &(r->ab->vec[para->baselen*3]), sizeof(VECT) * para->baselen);
  }

  if (para->delta && (para->energy || para->c0) && para->absesup) {
    /* 絶対値パワーを除去 */
    /* suppress absolute power */
    memmove(&(tmpmfcc[para->baselen-1]), &(tmpmfcc[para->baselen]), sizeof(VECT) * (para->vecbuflen - para->baselen));
  }

  /* この時点で tmpmfcc に現時点での最新の特徴ベクトルが格納されている */
  /* tmpmfcc[] now holds the latest parameter vector */

#ifdef RDEBUG
  printf("DeltaBuf: got frame %d\n", r->f);
#endif
  /* CMN を計算 */
  /* perform CMN */
  if (para->cmn) CMN_realtime(tmpmfcc, para->mfcc_dim);
  
  return TRUE;
}

/** 
 * <JA>
 * @brief  第1パス平行音声認識処理のメイン
 *
 * この関数は, 音声入力ルーチンのコールバックとして，取り込んだ
 * 音声データの断片を引数として呼び出されます．音声入力が開始されると，
 * 取り込んだ音声データは数千サンプルごとに，その都度この関数が呼び出されます．
 * 呼び出しは，音声区間終了か入力ストリームの終了まで続きます．
 * 
 * この関数内では，漸次的な特徴量抽出および第1パスの認識が行われます．
 * 
 * @param Speech [in] 音声データへのバッファへのポインタ
 * @param nowlen [in] 音声データの長さ
 * 
 * @return エラー時 -1 を，正常時 0 を返す．また，この入力断片までで，
 * 文章の区切りとして第1パスを終了したいときには 1 を返す．
 * </JA>
 * <EN>
 * @brief  Main function of on-the-fly 1st pass decoding
 *
 * This function will be called each time a new speech sample comes, as
 * as callback from A/D-in routine.  When a speech input begins, the captured
 * speech will be passed to this function for every sample segments.  This
 * process will continue until A/D-in routine detects an end of speech or
 * input stream reached to an end.
 *
 * This function will perform feture vector extraction and beam decording as
 * 1st pass recognition simultaneously, in frame-wise mannar.
 * 
 * @param Speech [in] pointer to the speech sample segments
 * @param nowlen [in] length of above
 * 
 * @return -1 on error (tell caller to terminate), 0 on success (allow caller
 * to call me for the next segment), or 1 when an input segmentation is
 * required at this point (in that case caller will stop input and go to
 * 2nd pass)
 * </EN>
 */
int
RealTimePipeLine(SP16 *Speech, int nowlen, Recog **recoglist, int recognum) /* Speech[0...nowlen] = input */
{
  int i, now;
  boolean ret;
  Value *para;
  RealBeam *r;
  HTK_Param *param;
  int f;
  Recog *re;
  int ir;

  re = recoglist[0];

  para = &(re->jconf->analysis.para);
  r = &(re->real);
  param = r->param;

  /* window[0..windownum-1] は前回の呼び出しで残った音声データが格納されている */
  /* window[0..windownum-1] are speech data left from previous call */

  /* 処理用ポインタを初期化 */
  /* initialize pointer for local processing */
  now = 0;
  
  /* 認識処理がセグメント要求で終わったのかどうかのフラグをリセット */
  /* reset flag which indicates whether the input has ended with segmentation request */
  r->last_is_segmented = FALSE;

#ifdef RDEBUG
  printf("got %d samples\n", nowlen);
#endif

  while (now < nowlen) {	/* till whole input is processed */
    /* 入力長が maxframelen に達したらここで強制終了 */
    /* if input length reaches maximum buffer size, terminate 1st pass here */
    if (r->f >= r->maxframelen) return(1);
    /* 窓バッファを埋められるだけ埋める */
    /* fill window buffer as many as possible */
    for(i = min(r->windowlen - r->windownum, nowlen - now); i > 0 ; i--)
      r->window[r->windownum++] = (float) Speech[now++];
    /* もし窓バッファが埋まらなければ, このセグメントの処理はここで終わる．
       処理されなかったサンプル (window[0..windownum-1]) は次回に持ち越し．*/
    /* if window buffer was not filled, end processing here, keeping the
       rest samples (window[0..windownum-1]) in the window buffer. */
    if (r->windownum < r->windowlen) break;
#ifdef RDEBUG
    /*    printf("%d used, %d rest\n", now, nowlen - now);

	  printf("[f = %d]\n", f);*/
#endif

    /* 窓内の音声波形から特徴量を計算して r->tmpmfcc に格納  */
    /* calculate a parameter vector from current waveform windows
       and store to r->tmpmfcc */
    if ((*(re->calc_vector))(r->tmpmfcc, r->window, r->windowlen, &(re->jconf->analysis.para), re) == FALSE) {
      /* 特徴量が得られなかったので、以降の処理をスキップして次の窓へ進む */
      /* skip processing this window and go to next input window */
      goto next_input;
    }

    /* MFCC完成，登録 */
    /* now get the MFCC vector of current frame, now store it to param */
    if (param_alloc(param, r->f + 1, param->veclen) == FALSE) {
      jlog("ERROR: failed to allocate memory for incoming MFCC vectors\n");
      return -1;
    }
    memcpy(param->parvec[r->f], r->tmpmfcc, sizeof(VECT) * param->veclen);
    
    /* ここでフレーム "f" に最新のMFCCが保存されたことになる */
    /* now we got the most recent MFCC parameter for frame 'f' */
    /* この "f" のフレームについて認識処理(フレーム同期ビーム探索)を進める */
    /* proceed beam search for this frame [f] */
    if (r->f == 0) {
      /* 最初のフレーム: 探索処理を初期化 */
      /* initial frame: initialize search process */
#ifdef SP_BREAK_CURRENT_FRAME
      if (!re->process_segment) {
	callback_multi_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recoglist, recognum);
      }
      callback_multi_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recoglist, recognum);
#else
      callback_multi_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recoglist, recognum);
#endif
      for(ir=0;ir<recognum;ir++) {
	if (get_back_trellis_init(param, recoglist[ir]) == FALSE) {
	  jlog("ERROR: fatal error occured, program terminates now\n");
	  return -1;
	}
	if (recoglist[ir]->model->gmm != NULL) {
	  /* GMM 計算の初期化 */
	  gmm_prepare(recoglist[ir]);
	}
      }
      
    }
    for(ir=0;ir<recognum;ir++) {
      if (recoglist[ir]->model->hmminfo->multipath || r->f != 0) {
	/* 1フレーム探索を進める */
	/* proceed search for 1 frame */
	if (get_back_trellis_proceed(r->f, param, recoglist[ir], FALSE) == FALSE) {
	  
	  /* 探索処理の終了が発生したのでここで認識を終える．
	     最初のフレームから [f-1] 番目までが認識されたことになる
	  */
	  /* the recognition process tells us to stop recognition, so
	     recognition should be terminated here.
	     the recognized data are [0..f-1] */
	  
	  /* 認識処理のセグメント要求で終わったことをフラグにセット */
	  /* set flag which indicates that the input has ended with segmentation request */
	  r->last_is_segmented = TRUE;
	  /* 最終フレームを last_time にセット */
	  /* set the last frame to last_time */
	  r->last_time = r->f - 1;
#ifdef SP_BREAK_CURRENT_FRAME
	  /* ショートポーズセグメンテーション: バッファに残っているデータを
	     別に保持して，次回の最初に処理する */
	  /* short pause segmentation: there is some data left in buffer, so
	     we should keep them for next processing */
	  param->header.samplenum = r->f + 1;/* len = lastid + 1 */
	  param->samplenum = r->f + 1;
	  r->rest_len = nowlen - now;
	  if (r->rest_len > 0) {
	    /* copy rest samples to rest_Speech */
	    if (r->rest_Speech == NULL) {
	      r->rest_alloc_len = r->rest_len;
	      r->rest_Speech = (SP16 *)mymalloc(sizeof(SP16)*r->rest_alloc_len);
	    } else if (r->rest_alloc_len < r->rest_len) {
	      r->rest_alloc_len = r->rest_len;
	      r->rest_Speech = (SP16 *)myrealloc(r->rest_Speech, sizeof(SP16)*r->rest_alloc_len);
	    }
	    memcpy(r->rest_Speech, &(Speech[now]), sizeof(SP16) * r->rest_len);
	  }
#else
	  /* param に格納されたフレーム長をセット */
	  /* set number of frames to param */
	  param->header.samplenum = r->f;
	  param->samplenum = r->f;
#endif
	  /* tell the caller to be segmented by this function */
	  /* 呼び出し元に，ここで入力を切るよう伝える */
	  return(1);
	}
      
      }
      if (recoglist[ir]->model->gmm != NULL) {
	/* GMM 計算を行う */
	gmm_proceed(recoglist[ir], param, r->f);
      }
    }
    /* 1フレーム処理が進んだのでポインタを進める */
    /* proceed frame pointer */
    r->f++;

  next_input:

    /* 窓バッファを処理が終わった分シフト */
    /* shift window */
    memmove(r->window, &(r->window[para->frameshift]), sizeof(SP16) * (r->windowlen - para->frameshift));
    r->windownum -= para->frameshift;
  }

  /* 与えられた音声セグメントに対する認識処理が全て終了
     呼び出し元に, 入力を続けるよう伝える */
  /* input segment is fully processed
     tell the caller to continue input */
  return(0);			
}

#ifdef SP_BREAK_CURRENT_FRAME
/** 
 * <JA>
 * ショートポーズセグメンテーションの再開処理:
 * 入力の認識開始の前に,前回のオーバーラップ分と残りを処理する．
 *
 * ショートポーズセグメンテーションでは, 前区間での末尾の sp に対応した
 * 区間分を遡って認識を再開する．この前区間の末尾の sp 区間のMFCCパラメー
 * タが rest_param に入っているので, まずはそこから認識処理を開始する．
 * 次に，前回の認識終了時に残った未処理の音声データが rest_Speech に
 * あるので，続けてそれらの認識処理を行う．
 * この処理のあと,通常の RealTimePipeLine() が呼び出される．
 * 
 * @return エラー時 -1，正常時 0 を返す．また，この入力断片の処理中に
 * 文章の区切りが見つかったときは第1パスをここで中断するために 1 を返す．
 * </JA>
 * </JA>
 * <EN>
 * Resuming recognition for short pause segmentation:
 * process the overlapped data and remaining speech prior to the next input.
 *
 * When short-pause segmentation is enabled, we restart(resume)
 * the recognition process from the beginning of last short-pause segment
 * at the last input.  The corresponding parameters are in "rest_param",
 * so we should first process the parameters.  Further, we have remaining
 * speech data that has not been converted to MFCC at the last input in
 * "rest_Speech[]", so next we should process the rest speech.
 * After this funtion, the usual RealTimePileLine() can be called for the
 * next incoming new speeches.
 * 
 * @return -1 on error (tell caller to terminate), 0 on success (allow caller
 * to call me for the next segment), or 1 when an end-of-sentence detected
 * at this point (in that case caller will stop input and go to 2nd pass)
 * </EN>
 */
int
RealTimeResume(Recog **recoglist, int recognum)
{
  int t;
  Value *para;
  RealBeam *r;
  Recog *recog;
  int ir;
  boolean ret;

  recog = recoglist[0];

  para = &(recog->jconf->analysis.para);
  r = &(recog->real);

  /* 最後のデータはすでに recog->param に格納されている */
  /* rest_param already exist at recog->param */
  r->param = recog->param;

  /* 対数エネルギー正規化のための初期値をセット */
  /* set initial value for log energy normalization */
  if (para->energy && para->enormal) energy_max_prepare(para);
  /* デルタ計算用バッファを準備 */
  /* set the delta cycle buffer */
  if (para->delta) WMP_deltabuf_prepare(r->db);
  if (para->acc) WMP_deltabuf_prepare(r->ab);

  /* paramを準備 */
  /* prepare param by expanding the last input param */
  for(ir=0;ir<recognum;ir++) {
    outprob_prepare(&(recoglist[ir]->hmmwrk), r->maxframelen);
  }
  //r->param->parvec = (VECT **)myrealloc(r->param->parvec, sizeof(VECT *) * r->maxframelen);
  /* param にある全パラメータを処理する: f_raw をあらかじめセット */
  /* process all data in param: pre-set the resulting f_raw */
  r->f = r->param->samplenum - 1;

  /* param 内の全フレームについて認識処理を進める */
  /* proceed recognition for all frames in param */
  if (r->f >= 0) {
#ifdef RDEBUG
    printf("Resume: f=%d\n", r->f);
#endif

    for(ir=0;ir<recognum;ir++) {
      if (get_back_trellis_init(r->param, recoglist[ir]) == FALSE) {
	jlog("ERROR: fatal error occured, program terminates now\n");
	return -1;
      }
      if (recoglist[ir]->model->gmm != NULL) {
	/* GMM 計算の初期化 */
	gmm_prepare(recoglist[ir]);
	if (!recoglist[ir]->model->hmminfo->multipath) {
	  /* 0フレーム目の GMM 計算を行う */
	  gmm_proceed(recoglist[ir], r->param, 0);
	}
      }
    }
     
    ret = TRUE;
    for(ir=0;ir<recognum;ir++) {
      for (t = recoglist[ir]->model->hmminfo->multipath ? 0 : 1; t <= r->f; t++) {
	if (get_back_trellis_proceed(t, r->param, recoglist[ir], FALSE) == FALSE) {
	  /* segmented, end procs ([0..f])*/
	  ret = FALSE;
	  break;
	}
	if (recoglist[ir]->model->gmm != NULL) {
	  /* GMM 計算を行う */
	  gmm_proceed(recoglist[ir], r->param, t);
	}
      }
    }
    if (ret == FALSE) {
      /* segmented, end procs ([0..f])*/
      r->last_is_segmented = TRUE;
      r->last_time = t-1;
      return(1);		/* segmented by this function */
    }
  }

  r->f++;

  /* シフトしておく */
  /* do last shift */
  memmove(r->window, &(r->window[para->frameshift]), sizeof(SP16) * (r->windowlen - para->frameshift));
  r->windownum -= para->frameshift;

  /* これで再開の準備が整ったので,まずは前回の処理で残っていた音声データから
     処理する */
  /* now that the search status has been prepared for the next input, we
     first process the rest unprocessed samples at the last session */
  if (r->rest_len > 0) {
#ifdef RDEBUG
    printf("Resume: rest %d samples\n", r->rest_len);
#endif
    return(RealTimePipeLine(r->rest_Speech, r->rest_len, recoglist, recognum));
  }

  /* 新規の入力に対して認識処理は続く… */
  /* the recognition process will continue for the newly incoming samples... */
  return 0;
}
#endif /* SP_BREAK_CURRENT_FRAME */


/* ON-THE-FLY デコーディング関数: 終了処理 */
/* ON-THE-FLY DECODING FUNCTION: end processing */
/** 
 * <JA>
 * 第1パス平行認識処理の終了処理を行う．
 * 
 * @return この入力の特徴パラメータを格納した構造体を返す．
 * </JA>
 * <EN>
 * Finalize the 1st pass on-the-fly decoding.
 * 
 * @return newly allocated input parameter data for this input.
 * </EN>
 */
boolean
RealTimeParam(Recog **recoglist, int recognum)
{
  boolean ret1, ret2;
  Value *para;
  RealBeam *r;
  Recog *recog;
  int ir;

  recog = recoglist[0];

  para = &(recog->jconf->analysis.para);
  r = &(recog->real);

  if (r->last_is_segmented) {
    /* RealTimePipeLine で認識処理側の理由により認識が中断した場合,
       現状態のMFCC計算データをそのまま次回へ保持する必要があるので,
       MFCC計算終了処理を行わずに第１パスの結果のみ出力して終わる．*/
    /* When input segmented by recognition process in RealTimePipeLine(),
       we have to keep the whole current status of MFCC computation to the
       next call.  So here we only output the 1st pass result. */
    for(ir=0;ir<recognum;ir++) {
      recoglist[ir]->backmax = finalize_1st_pass(recoglist[ir], r->last_time);
#ifdef SP_BREAK_CURRENT_FRAME
      finalize_segment(recoglist[ir], r->param, r->last_time);
#endif
      if (recoglist[ir]->model->gmm != NULL) {
	/* GMM 計算の終了 */
	gmm_end(recoglist[ir]);
      }
    }
    /* この区間の param データを第２パスのために返す */
    /* return obtained parameter for 2nd pass */
    return(TRUE);
  }

  /* MFCC計算の終了処理を行う: 最後の遅延フレーム分を処理 */
  /* finish MFCC computation for the last delayed frames */

  if (para->delta || para->acc) {

    /* look until all data has been flushed */
    while(r->f < r->maxframelen) {

      /* check if there is data in cycle buffer of delta */
      ret1 = WMP_deltabuf_flush(r->db);
#ifdef RDEBUG
      {
	int i;
	printf("DeltaBufLast: ret=%d, status=", ret1);
	for(i=0;i<r->db->len;i++) {
	  printf("%d", r->db->is_on[i]);
	}
	printf(", nextstore=%d\n", r->db->store);
      }
#endif
      if (ret1) {
	/* uncomputed delta has flushed, compute it with tmpmfcc */
	if (para->energy && para->absesup) {
	  memcpy(r->tmpmfcc, r->db->vec, sizeof(VECT) * (para->baselen - 1));
	  memcpy(&(r->tmpmfcc[para->baselen-1]), &(r->db->vec[para->baselen]), sizeof(VECT) * para->baselen);
	} else {
	  memcpy(r->tmpmfcc, r->db->vec, sizeof(VECT) * para->baselen * 2);
	}
	if (para->acc) {
	  /* this new delta should be given to the accel cycle buffer */
	  ret2 = WMP_deltabuf_proceed(r->ab, r->tmpmfcc);
#ifdef RDEBUG
	  printf("AccelBuf: ret=%d, status=", ret2);
	  for(i=0;i<r->ab->len;i++) {
	    printf("%d", r->ab->is_on[i]);
	  }
	  printf(", nextstore=%d\n", r->ab->store);
#endif
	  if (ret2) {
	    /* uncomputed accel was given, compute it with tmpmfcc */
	    memcpy(r->tmpmfcc, r->ab->vec, sizeof(VECT) * (para->veclen - para->baselen));
	    memcpy(&(r->tmpmfcc[para->veclen - para->baselen]), &(r->ab->vec[para->veclen - para->baselen]), sizeof(VECT) * para->baselen);
	  } else {
	    /* no input is still available: */
	    /* in case of very short input: go on to the next input */
	    continue;
	  }
	}
      } else {
	/* no data left in the delta buffer */
	if (para->acc) {
	  /* no new data, just flush the accel buffer */
	  ret2 = WMP_deltabuf_flush(r->ab);
#ifdef RDEBUG
	  printf("AccelBuf: ret=%d, status=", ret2);
	  for(i=0;i<r->ab->len;i++) {
	    printf("%d", r->ab->is_on[i]);
	  }
	  printf(", nextstore=%d\n", r->ab->store);
#endif
	  if (ret2) {
	    /* uncomputed data has flushed, compute it with tmpmfcc */
	    memcpy(r->tmpmfcc, r->ab->vec, sizeof(VECT) * (para->veclen - para->baselen));
	    memcpy(&(r->tmpmfcc[para->veclen - para->baselen]), &(r->ab->vec[para->veclen - para->baselen]), sizeof(VECT) * para->baselen);
	  } else {
	    /* actually no data exists in both delta and accel */
	    break;		/* end this loop */
	  }
	} else {
	  /* only delta: input fully flushed, end this loop */
	  break;
	}
      }
      if(para->cmn) CMN_realtime(r->tmpmfcc, para->mfcc_dim);
      if (param_alloc(r->param, r->f + 1, r->param->veclen) == FALSE) {
	jlog("ERROR: failed to allocate memory for incoming MFCC vectors\n");
	return FALSE;
      }
      memcpy(r->param->parvec[r->f], r->tmpmfcc, sizeof(VECT) * r->param->veclen);
      if (r->f == 0) {
	for(ir=0;ir<recognum;ir++) {
	  if (get_back_trellis_init(r->param, recoglist[ir]) == FALSE) {
	    jlog("ERROR: failed to initialize the 1st pass\n");
	    return FALSE;
	  }
	  if (recoglist[ir]->model->gmm != NULL) {
	    /* GMM 計算の初期化 */
	    gmm_prepare(recoglist[ir]);
	  }
	}
      }
      for(ir=0;ir<recognum;ir++) {
	if (recoglist[ir]->model->hmminfo->multipath || r->f != 0) {
	  get_back_trellis_proceed(r->f, r->param, recoglist[ir], FALSE);
	}
	if (recog->model->gmm != NULL) {
	  /* GMM 計算を行う */
	  gmm_proceed(recoglist[ir], r->param, r->f);
	}
      }
      r->f++;
    }
  }

  /* フレーム長をセット */
  /* set frame length */
  r->param->header.samplenum = r->f;
  r->param->samplenum = r->f;

  /* 入力長がデルタの計算に十分でない場合,
     MFCC が CMN まで正しく計算できないため，エラー終了とする．*/
  /* if input is short for compute all the delta coeff., terminate here */
  if (r->f == 0) {
    jlog("ERROR: too short input to compute delta coef! (%d frames)\n", r->f);
    for(ir=0;ir<recognum;ir++) {
      recoglist[ir]->backmax = finalize_1st_pass(recoglist[ir], r->param->samplenum);
    }
  } else {
    /* 第１パスの終了処理を行う */
    /* finalize 1st pass */
    for(ir=0;ir<recognum;ir++) {
      get_back_trellis_end(r->param, recoglist[ir]);
      recog->backmax = finalize_1st_pass(recoglist[ir], r->param->samplenum);
#ifdef SP_BREAK_CURRENT_FRAME
      finalize_segment(recoglist[ir], r->param, r->param->samplenum);
#endif
      if (recoglist[ir]->model->gmm != NULL) {
	/* GMM 計算の終了 */
	gmm_end(recoglist[ir]);
      }
    }
  }

  return(TRUE);
}

/** 
 * <JA>
 * 次回の認識に備えて CMN用にケプストラム平均を更新する．
 * 
 * @param param [in] 現在の入力パラメータ
 * </JA>
 * <EN>
 * Update cepstral mean of CMN to prepare for the next input.
 * 
 * @param param [in] current input parameter
 * </EN>
 */
void
RealTimeCMNUpdate(HTK_Param *param, Recog *recog)
{
  float mseclen;
  boolean cmn_update_p;
  Value *para;
  Jconf *jconf;

  jconf = recog->jconf;

  para = &(jconf->analysis.para);
  
  /* update CMN vector for next speech */
  if(para->cmn) {
    if (jconf->frontend.cmn_update) {
      cmn_update_p = TRUE;
      if (jconf->reject.rejectshortlen > 0) {
	/* not update if rejected by short input */
	mseclen = (float)param->samplenum * (float)para->smp_period * (float)para->frameshift / 10000.0;
	if (mseclen < jconf->reject.rejectshortlen) {
	  cmn_update_p = FALSE;
	}
      }
      if (jconf->reject.gmm_reject_cmn_string != NULL) {
	/* if using gmm, try avoiding update of CMN for noise input */
	if(! gmm_valid_input(recog)) {
	  cmn_update_p = FALSE;
	}
      }
      if (cmn_update_p) {
	/* update last CMN parameter for next spech */
	CMN_realtime_update();
      } else {
	/* do not update, because the last input is bogus */
	jlog("STAT: skip CMN parameter update since last input was invalid\n");
      }
    }
    /* if needed, save the updated CMN parameter to a file */
    if (jconf->frontend.cmnsave_filename) {
      if (CMN_save_to_file(jconf->frontend.cmnsave_filename) == FALSE) {
	jlog("WARNING: failed to save CMN parameter to \"%s\"\n", jconf->frontend.cmnsave_filename);
      }
    }
  }
}

/** 
 * <JA>
 * 第1パス平行認識処理の中断時の終了処理を行う．
 * </JA>
 * <EN>
 * Finalize the 1st pass on-the-fly decoding when terminated.
 * </EN>
 */
void
RealTimeTerminate(Recog **recoglist, int recognum)
{
  HTK_Param *param;
  RealBeam *r;
  Recog *recog;
  int ir;

  recog = recoglist[0];

  r = &(recog->real);
  param = r->param;

  param->header.samplenum = r->f;
  param->samplenum = r->f;

  /* 入力長がデルタの計算に十分でない場合,
     MFCC が CMN まで正しく計算できないため，エラー終了とする．*/
  /* if input is short for compute all the delta coeff., terminate here */
  if (r->f == 0) {
    for(ir=0;ir<recognum;ir++) {
      finalize_1st_pass(recoglist[ir], param->samplenum);
    }
  } else {
    /* 第１パスの終了処理を行う */
    /* finalize 1st pass */
    for(ir=0;ir<recognum;ir++) {
      get_back_trellis_end(param, recoglist[ir]);
      finalize_1st_pass(recoglist[ir], param->samplenum);
#ifdef SP_BREAK_CURRENT_FRAME
      finalize_segment(recoglist[ir], param, param->samplenum);
#endif
      if (recoglist[ir]->model->gmm != NULL) {
      /* GMM 計算の終了 */
	gmm_end(recoglist[ir]);
      }
    }
  }

}

void
realbeam_free(Recog *recog)
{
  RealBeam *r;

  r = &(recog->real);

  if (recog->real.tmpmfcc) {
    free(recog->real.tmpmfcc);
    recog->real.tmpmfcc = NULL;
  }
  if (recog->real.window) {
    free(recog->real.window);
    recog->real.window = NULL;
  }
#ifdef SP_BREAK_CURRENT_FRAME
  if (recog->real.rest_Speech) {
    free(recog->real.rest_Speech);
    recog->real.rest_Speech = NULL;
  }
#endif
  if (recog->real.db) {
    WMP_deltabuf_free(recog->real.db);
    recog->real.db = NULL;
  }
  if (recog->real.ab) {
    WMP_deltabuf_free(recog->real.ab);
    recog->real.ab = NULL;
  }
  
}
