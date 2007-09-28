/**
 * @file   realtime-1stpass.c
 * @author Akinobu Lee
 * @date   Tue Aug 23 11:44:14 2005
 * 
 * <JA>
 * @brief  �»���ǧ���Τ������1�ѥ���ʿ�Խ���
 *
 * ��1�ѥ������ϳ��Ϥ�Ʊ���˥������Ȥ������Ϥ�ʿ�Ԥ���ǧ��������Ԥ������
 * �ؿ����������Ƥ��롥
 * 
 * �̾Julius �β���ǧ�������ϰʲ��μ��� main_recognition_loop() ���
 * �¹Ԥ���롥
 *
 *  -# �������� adin_go()  �� ���ϲ����� speech[] �˳�Ǽ�����
 *  -# ��ħ����� new_wav2mfcc() ��speech������ħ�ѥ�᡼���� param �˳�Ǽ
 *  -# ��1�ѥ��¹� get_back_trellis() ��param �ȥ�ǥ뤫��ñ��ȥ�ꥹ������
 *  -# ��2�ѥ��¹� wchmm_fbs()
 *  -# ǧ����̽���
 *
 * ��1�ѥ���ʿ�Խ��������硤�嵭�� 1 �� 3 ��ʿ�Ԥ��ƹԤ��롥
 * Julius �Ǥϡ������¹Խ����򡤲������Ϥ����Ҥ������뤿�Ӥ�
 * ǧ�������򤽤�ʬ��������Ū�˿ʤ�뤳�ȤǼ������Ƥ��롥
 * 
 *  - ��ħ����Ф���1�ѥ��¹Ԥ򡤰�ĤˤޤȤ�ƥ�����Хå��ؿ��Ȥ��������
 *  - �������ϴؿ� adin_go() �Υ�����Хå��Ȥ��ƾ嵭�δؿ���Ϳ����
 *
 * ����Ū�ˤϡ��������������Ƥ��� RealTimePipeLine() ��������Хå��Ȥ���
 * adin_go() ��Ϳ�����롥adin_go() �ϲ������Ϥ��ȥꥬ����Ȥ�������줿����
 * ���Ҥ��Ȥ� RealTimePipeLine() ��ƤӽФ���RealTimePipeLine() ������줿
 * ����ʬ�ˤĤ�����ħ����Ф���1�ѥ��η׻���ʤ�롥
 *
 * CMN �ˤĤ�����դ�ɬ�פǤ��롥CMN ���̾�ȯ��ñ�̤ǹԤ��뤬��
 * �ޥ������Ϥ�ͥåȥ�����ϤΤ褦�ˡ���1�ѥ���ʿ�Ԥ�ǧ����Ԥ�
 * ��������ȯ�����ΤΥ��ץ��ȥ��ʿ�Ѥ����뤳�Ȥ��Ǥ��ʤ����С������ 3.5
 * �����Ǥ�ľ����ȯ��5��ʬ(���Ѥ��줿���Ϥ����)�� CMN �����Τޤ޼�ȯ�ä�
 * ή�Ѥ���Ƥ�������3.5.1 ����ϡ��嵭��ľ��ȯ�� CMN �����ͤȤ���
 * ȯ���� CMN �� MAP-CMN ��������Ʒ׻�����褦�ˤʤä����ʤ���
 * �ǽ��ȯ���Ѥν��CMN�� "-cmnload" ��Ϳ���뤳�Ȥ�Ǥ����ޤ�
 * "-cmnnoupdate" �����Ϥ��Ȥ� CMN ������Ԥ�ʤ��褦�ˤǤ��롥
 * "-cmnnoupdate" �� "-cmnload" ���Ȥ߹�碌�뤳�Ȥ�, �ǽ�˥����Х��
 * ���ץ��ȥ��ʿ�Ѥ�Ϳ����������˽���ͤȤ��� MAP-CMN ���뤳�Ȥ��Ǥ��롥
 *
 * ���פʴؿ��ϰʲ����̤�Ǥ��롥
 *
 *  - RealTimeInit() - ��ư���ν����
 *  - RealTimePipeLinePrepare() - ���Ϥ��Ȥν����
 *  - RealTimePipeLine() - ��1�ѥ�ʿ�Խ����ѥ�����Хå��ʾ�ҡ�
 *  - RealTimeResume() - ���硼�ȥݡ����������ơ���������ǧ������
 *  - RealTimeParam() - ���Ϥ��Ȥ���1�ѥ���λ����
 *  - RealTimeCMNUpdate() - CMN �ι���
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

/* �׻���̤� MFCC ����¸���� param ��¤�Τ��������
   �����1���ǧ�����Ȥ˷����֤��ƤФ�� */
/* prepare new parameter vector holder for RealTime*
   This will be called each time a recognition begins */
/** 
 * <JA>
 * ����Ū�˷׻��������ħ�٥��ȥ�����ݻ����뤿����ΰ��������롥
 * ���������/ǧ��1�󤴤Ȥ˷����֤��ƤФ�롥
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
  /* ���줫��׻������ѥ�᡼���η���إå������� */
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
  /* �ե졼�ऴ�ȤΥѥ�᡼���٥��ȥ���¸���ΰ����� */
  /* ���Ȥ�ɬ�פ˱����ƿ�Ĺ����� */
  if (param_alloc(r->param, 1, r->param->veclen) == FALSE) {
    j_internal_error("ERROR: segmented: failed to allocate memory for rest param\n");
  }

  /* ǧ��������/��λ��˥��åȤ�����ѿ�:
     param->parvec (�ѥ�᡼���٥��ȥ����)
     param->header.samplenum, param->samplenum (���ե졼���)
  */
  /* variables that will be set while/after computation has been done:
     param->parvec (parameter vector sequence)
     param->header.samplenum, param->samplenum (total number of frames)
  */
}

/** 
 * <JA>
 * ��1�ѥ�ʿ��ǧ�������ν�����ʵ�ư��1������ƤФ���
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

  /* -ssload �����, SS�ѤΥΥ������ڥ��ȥ��ե����뤫���ɤ߹��� */
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
  /* �п����ͥ륮���������Τ���ν���� */
  /* set initial value for log energy normalization */
  if (para->energy && para->enormal) energy_max_init();
  /* �ǥ륿�׻��Τ���Υ�������Хåե����Ѱ� */
  /* initialize cycle buffers for delta and accel coef. computation */
  if (para->delta) r->db = WMP_deltabuf_new(para->baselen, para->delWin);
  if (para->acc) r->ab = WMP_deltabuf_new(para->baselen * 2, para->accWin);
  /* �ǥ륿�׻��Τ���Υ�����ꥢ����� */
  /* allocate work area for the delta computation */
  r->tmpmfcc = (VECT *)mymalloc(sizeof(VECT) * para->vecbuflen);
  /* ����ե졼��Ĺ��������ϻ��ֿ�����׻� */
  /* set maximum allowed frame length */
  r->maxframelen = MAXSPEECHLEN / para->frameshift;
  /* ��Ĺ�򥻥å� */
  /* set window length */
  r->windowlen = para->framesize + 1;
  /* �뤫���ѥХåե������ */
  /* set window buffer */
  r->window = mymalloc(sizeof(SP16) * r->windowlen);
  /* MAP-CMN �Ѥν�����ץ��ȥ��ʿ�Ѥ��ɤ߹���ǽ�������� */
  /* Initialize the initial cepstral mean data from file for MAP-CMN */
  if (para->cmn) CMN_realtime_init(para->mfcc_dim, jconf->frontend.cmn_map_weight);
  /* -cmnload �����, CMN�ѤΥ��ץ��ȥ��ʿ�Ѥν���ͤ�ե����뤫���ɤ߹��� */
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

/* ON-THE-FLY �ǥ����ǥ��󥰴ؿ�: ���� (ǧ�����ϻ����Ȥ˸ƤФ��) */
/* ON-THE-FLY DECODING FUNCTION: prepare (on start of every input segment) */
/** 
 * <JA>
 * ��1�ѥ�ʿ��ǧ�������Υǡ���������ǧ�����Ϥ��Ȥ˸ƤФ���
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
  /* param ��¤�Τ� Recog �ȶ�ͭ���� */
  /* share allocate parameter data with Recog*/
  r->param = recog->param;
  init_param(r, para);
  /* �п����ͥ륮���������Τ���ν���ͤ򥻥å� */
  /* set initial value for log energy normalization */
  if (para->energy && para->enormal) energy_max_prepare(para);
  /* �ǥ륿�׻��ѥХåե������ */
  /* set the delta cycle buffer */
  if (para->delta) WMP_deltabuf_prepare(r->db);
  if (para->acc) WMP_deltabuf_prepare(r->ab);
  /* �������� param ��¤�ΤΥǡ����Υѥ�᡼�����򲻶���ǥ�ȥ����å����� */
  /* check type coherence between param and hmminfo here */
  if (!check_param_coherence(recog->model->hmminfo, r->param)) {
    jlog("ERROR: input parameter type does not match AM\n");
    return FALSE;
  }
  /* �׻��Ѥ��ѿ������� */
  /* initialize variables for computation */
  r->f = 0;
  r->windownum = 0;
  /* MAP-CMN �ν���� */
  /* Prepare for MAP-CMN */
  if (para->cmn) CMN_realtime_prepare();
  /* �������ٷ׻��ѥ���å�������
     ����Ĺ�򤳤��Ǥ��餫������ݤ��Ƥ��ޤ� */
  /* prepare cache area for acoustic computation of HMM states and mixtures
     pre-fetch for maximum length here */
  for(i=0;i<recognum;i++) {
    outprob_prepare(&(recoglist[i]->hmmwrk), r->maxframelen);
  }

  return TRUE;
}

/** 
 * <JA>
 * ��ñ�̤Ǽ��Ф��줿�����ȷ�����ѥ�᡼���٥��ȥ��׻����롣
 * 
 * @param tmpmfcc [out] �׻����줿�ѥ�᡼���٥��ȥ뤬��¸�����Хåե�
 * @param window [in] ��ñ�̤Ǽ��Ф��줿�����ȷ��ǡ���
 * @param windowlen [in] @a window ��Ĺ��
 * @param para [in] ������ħ����о��ѥ�᡼��
 * @param re [in] ǧ�����󥹥��� (re->mfccwrk �������ǻ��Ѥ����)
 * 
 * @return ��������@a tmpmfcc �˷׻���̤Υѥ�᡼���٥��ȥ뤬��¸���졢TRUE
 * ���֤������Ի��ʥǥ륿�׻������ϥե졼�ब���ʤ��ʤɡˤ� FALSE ���֤���
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

  /* �����ȷ����� base MFCC ��׻� (recog->mfccwrk ������) */
  /* calculate base MFCC from waveform (use recog->mfccwrk) */
  for (i=0; i < windowlen; i++) {
    re->mfccwrk->bf[i+1] = (float) window[i];
  }
  WMP_calc(re->mfccwrk, tmpmfcc, re->jconf->analysis.para);

  if (para->energy && para->enormal) {
    /* �п����ͥ륮��������������� */
    /* normalize log energy */
    /* �ꥢ�륿�������ϤǤ�ȯ�ä��Ȥκ��票�ͥ륮���������ʤ��Τ�
       ľ����ȯ�äΥѥ�����Ѥ��� */
    /* Since the maximum power of the whole input utterance cannot be
       obtained at real-time input, the maximum of last input will be
       used to normalize.
    */
    tmpmfcc[para->baselen-1] = energy_max_normalize(tmpmfcc[para->baselen-1], para);
  }

  if (para->delta) {
    /* �ǥ륿��׻����� */
    /* calc delta coefficients */
    ret = WMP_deltabuf_proceed(r->db, tmpmfcc);
#ifdef RDEBUG
    printf("DeltaBuf: ret=%d, status=", ret);
    for(i=0;i<r->db->len;i++) {
      printf("%d", r->db->is_on[i]);
    }
    printf(", nextstore=%d\n", r->db->store);
#endif
    /* ret == FALSE �ΤȤ��Ϥޤ��ǥ��쥤��ʤΤ�ǧ���������������Ϥ� */
    /* if ret == FALSE, there is no available frame.  So just wait for
       next input */
    if (! ret) {
      return FALSE;
    }

    /* db->vec �˸��ߤθ��ǡ����ȥǥ륿���������äƤ���Τ� tmpmfcc �˥��ԡ� */
    /* now db->vec holds the current base and full delta, so copy them to tmpmfcc */
    memcpy(tmpmfcc, r->db->vec, sizeof(VECT) * para->baselen * 2);
  }

  if (para->acc) {
    /* Acceleration��׻����� */
    /* calc acceleration coefficients */
    /* base+delta �򤽤Τޤ������ */
    /* send the whole base+delta to the cycle buffer */
    ret = WMP_deltabuf_proceed(r->ab, tmpmfcc);
#ifdef RDEBUG
    printf("AccelBuf: ret=%d, status=", ret);
    for(i=0;i<r->ab->len;i++) {
      printf("%d", r->ab->is_on[i]);
    }
    printf(", nextstore=%d\n", r->ab->store);
#endif
    /* ret == FALSE �ΤȤ��Ϥޤ��ǥ��쥤��ʤΤ�ǧ���������������Ϥ� */
    /* if ret == FALSE, there is no available frame.  So just wait for
       next input */
    if (! ret) {
      return FALSE;
    }
    /* ab->vec �ˤϡ�(base+delta) �Ȥ��κ�ʬ���������äƤ��롥
       [base] [delta] [delta] [acc] �ν�����äƤ���Τ�,
       [base] [delta] [acc] �� tmpmfcc �˥��ԡ����롥*/
    /* now ab->vec holds the current (base+delta) and their delta coef. 
       it holds a vector in the order of [base] [delta] [delta] [acc], 
       so copy the [base], [delta] and [acc] to tmpmfcc.  */
    memcpy(tmpmfcc, r->ab->vec, sizeof(VECT) * para->baselen * 2);
    memcpy(&(tmpmfcc[para->baselen*2]), &(r->ab->vec[para->baselen*3]), sizeof(VECT) * para->baselen);
  }

  if (para->delta && (para->energy || para->c0) && para->absesup) {
    /* �����ͥѥ����� */
    /* suppress absolute power */
    memmove(&(tmpmfcc[para->baselen-1]), &(tmpmfcc[para->baselen]), sizeof(VECT) * (para->vecbuflen - para->baselen));
  }

  /* ���λ����� tmpmfcc �˸������Ǥκǿ�����ħ�٥��ȥ뤬��Ǽ����Ƥ��� */
  /* tmpmfcc[] now holds the latest parameter vector */

#ifdef RDEBUG
  printf("DeltaBuf: got frame %d\n", r->f);
#endif
  /* CMN ��׻� */
  /* perform CMN */
  if (para->cmn) CMN_realtime(tmpmfcc, para->mfcc_dim);
  
  return TRUE;
}

/** 
 * <JA>
 * @brief  ��1�ѥ�ʿ�Բ���ǧ�������Υᥤ��
 *
 * ���δؿ���, �������ϥ롼����Υ�����Хå��Ȥ��ơ��������
 * �����ǡ��������Ҥ�����Ȥ��ƸƤӽФ���ޤ����������Ϥ����Ϥ����ȡ�
 * ������������ǡ����Ͽ��饵��ץ뤴�Ȥˡ��������٤��δؿ����ƤӽФ���ޤ���
 * �ƤӽФ��ϡ�������ֽ�λ�����ϥ��ȥ꡼��ν�λ�ޤ�³���ޤ���
 * 
 * ���δؿ���Ǥϡ�����Ū����ħ����Ф������1�ѥ���ǧ�����Ԥ��ޤ���
 * 
 * @param Speech [in] �����ǡ����ؤΥХåե��ؤΥݥ���
 * @param nowlen [in] �����ǡ�����Ĺ��
 * 
 * @return ���顼�� -1 ������� 0 ���֤����ޤ��������������ҤޤǤǡ�
 * ʸ�Ϥζ��ڤ�Ȥ�����1�ѥ���λ�������Ȥ��ˤ� 1 ���֤���
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

  /* window[0..windownum-1] ������θƤӽФ��ǻĤä������ǡ�������Ǽ����Ƥ��� */
  /* window[0..windownum-1] are speech data left from previous call */

  /* �����ѥݥ��󥿤����� */
  /* initialize pointer for local processing */
  now = 0;
  
  /* ǧ�����������������׵�ǽ���ä��Τ��ɤ����Υե饰��ꥻ�å� */
  /* reset flag which indicates whether the input has ended with segmentation request */
  r->last_is_segmented = FALSE;

#ifdef RDEBUG
  printf("got %d samples\n", nowlen);
#endif

  while (now < nowlen) {	/* till whole input is processed */
    /* ����Ĺ�� maxframelen ��ã�����餳���Ƕ�����λ */
    /* if input length reaches maximum buffer size, terminate 1st pass here */
    if (r->f >= r->maxframelen) return(1);
    /* ��Хåե���������������� */
    /* fill window buffer as many as possible */
    for(i = min(r->windowlen - r->windownum, nowlen - now); i > 0 ; i--)
      r->window[r->windownum++] = (float) Speech[now++];
    /* �⤷��Хåե�����ޤ�ʤ����, ���Υ������Ȥν����Ϥ����ǽ���롥
       ��������ʤ��ä�����ץ� (window[0..windownum-1]) �ϼ���˻����ۤ���*/
    /* if window buffer was not filled, end processing here, keeping the
       rest samples (window[0..windownum-1]) in the window buffer. */
    if (r->windownum < r->windowlen) break;
#ifdef RDEBUG
    /*    printf("%d used, %d rest\n", now, nowlen - now);

	  printf("[f = %d]\n", f);*/
#endif

    /* ����β����ȷ�������ħ�̤�׻����� r->tmpmfcc �˳�Ǽ  */
    /* calculate a parameter vector from current waveform windows
       and store to r->tmpmfcc */
    if ((*(re->calc_vector))(r->tmpmfcc, r->window, r->windowlen, &(re->jconf->analysis.para), re) == FALSE) {
      /* ��ħ�̤������ʤ��ä��Τǡ��ʹߤν����򥹥��åפ��Ƽ�����ؿʤ� */
      /* skip processing this window and go to next input window */
      goto next_input;
    }

    /* MFCC��������Ͽ */
    /* now get the MFCC vector of current frame, now store it to param */
    if (param_alloc(param, r->f + 1, param->veclen) == FALSE) {
      jlog("ERROR: failed to allocate memory for incoming MFCC vectors\n");
      return -1;
    }
    memcpy(param->parvec[r->f], r->tmpmfcc, sizeof(VECT) * param->veclen);
    
    /* �����ǥե졼�� "f" �˺ǿ���MFCC����¸���줿���Ȥˤʤ� */
    /* now we got the most recent MFCC parameter for frame 'f' */
    /* ���� "f" �Υե졼��ˤĤ���ǧ������(�ե졼��Ʊ���ӡ���õ��)��ʤ�� */
    /* proceed beam search for this frame [f] */
    if (r->f == 0) {
      /* �ǽ�Υե졼��: õ������������ */
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
	  /* GMM �׻��ν���� */
	  gmm_prepare(recoglist[ir]);
	}
      }
      
    }
    for(ir=0;ir<recognum;ir++) {
      if (recoglist[ir]->model->hmminfo->multipath || r->f != 0) {
	/* 1�ե졼��õ����ʤ�� */
	/* proceed search for 1 frame */
	if (get_back_trellis_proceed(r->f, param, recoglist[ir], FALSE) == FALSE) {
	  
	  /* õ�������ν�λ��ȯ�������ΤǤ�����ǧ���򽪤��롥
	     �ǽ�Υե졼�फ�� [f-1] ���ܤޤǤ�ǧ�����줿���Ȥˤʤ�
	  */
	  /* the recognition process tells us to stop recognition, so
	     recognition should be terminated here.
	     the recognized data are [0..f-1] */
	  
	  /* ǧ�������Υ��������׵�ǽ���ä����Ȥ�ե饰�˥��å� */
	  /* set flag which indicates that the input has ended with segmentation request */
	  r->last_is_segmented = TRUE;
	  /* �ǽ��ե졼��� last_time �˥��å� */
	  /* set the last frame to last_time */
	  r->last_time = r->f - 1;
#ifdef SP_BREAK_CURRENT_FRAME
	  /* ���硼�ȥݡ����������ơ������: �Хåե��˻ĤäƤ���ǡ�����
	     �̤��ݻ����ơ�����κǽ�˽������� */
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
	  /* param �˳�Ǽ���줿�ե졼��Ĺ�򥻥å� */
	  /* set number of frames to param */
	  param->header.samplenum = r->f;
	  param->samplenum = r->f;
#endif
	  /* tell the caller to be segmented by this function */
	  /* �ƤӽФ����ˡ����������Ϥ��ڤ�褦������ */
	  return(1);
	}
      
      }
      if (recoglist[ir]->model->gmm != NULL) {
	/* GMM �׻���Ԥ� */
	gmm_proceed(recoglist[ir], param, r->f);
      }
    }
    /* 1�ե졼��������ʤ���Τǥݥ��󥿤�ʤ�� */
    /* proceed frame pointer */
    r->f++;

  next_input:

    /* ��Хåե������������ä�ʬ���ե� */
    /* shift window */
    memmove(r->window, &(r->window[para->frameshift]), sizeof(SP16) * (r->windowlen - para->frameshift));
    r->windownum -= para->frameshift;
  }

  /* Ϳ����줿�����������Ȥ��Ф���ǧ�����������ƽ�λ
     �ƤӽФ�����, ���Ϥ�³����褦������ */
  /* input segment is fully processed
     tell the caller to continue input */
  return(0);			
}

#ifdef SP_BREAK_CURRENT_FRAME
/** 
 * <JA>
 * ���硼�ȥݡ����������ơ������κƳ�����:
 * ���Ϥ�ǧ�����Ϥ�����,����Υ����С���å�ʬ�ȻĤ��������롥
 *
 * ���硼�ȥݡ����������ơ������Ǥ�, ����֤Ǥ������� sp ���б�����
 * ���ʬ���̤ä�ǧ����Ƴ����롥��������֤������� sp ��֤�MFCC�ѥ�᡼
 * ���� rest_param �����äƤ���Τ�, �ޤ��Ϥ�������ǧ�������򳫻Ϥ��롥
 * ���ˡ������ǧ����λ���˻Ĥä�̤�����β����ǡ����� rest_Speech ��
 * ����Τǡ�³���Ƥ�����ǧ��������Ԥ���
 * ���ν����Τ���,�̾�� RealTimePipeLine() ���ƤӽФ���롥
 * 
 * @return ���顼�� -1������� 0 ���֤����ޤ��������������Ҥν������
 * ʸ�Ϥζ��ڤ꤬���Ĥ��ä��Ȥ�����1�ѥ��򤳤������Ǥ��뤿��� 1 ���֤���
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

  /* �Ǹ�Υǡ����Ϥ��Ǥ� recog->param �˳�Ǽ����Ƥ��� */
  /* rest_param already exist at recog->param */
  r->param = recog->param;

  /* �п����ͥ륮���������Τ���ν���ͤ򥻥å� */
  /* set initial value for log energy normalization */
  if (para->energy && para->enormal) energy_max_prepare(para);
  /* �ǥ륿�׻��ѥХåե������ */
  /* set the delta cycle buffer */
  if (para->delta) WMP_deltabuf_prepare(r->db);
  if (para->acc) WMP_deltabuf_prepare(r->ab);

  /* param����� */
  /* prepare param by expanding the last input param */
  for(ir=0;ir<recognum;ir++) {
    outprob_prepare(&(recoglist[ir]->hmmwrk), r->maxframelen);
  }
  //r->param->parvec = (VECT **)myrealloc(r->param->parvec, sizeof(VECT *) * r->maxframelen);
  /* param �ˤ������ѥ�᡼�����������: f_raw �򤢤餫���᥻�å� */
  /* process all data in param: pre-set the resulting f_raw */
  r->f = r->param->samplenum - 1;

  /* param ������ե졼��ˤĤ���ǧ��������ʤ�� */
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
	/* GMM �׻��ν���� */
	gmm_prepare(recoglist[ir]);
	if (!recoglist[ir]->model->hmminfo->multipath) {
	  /* 0�ե졼���ܤ� GMM �׻���Ԥ� */
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
	  /* GMM �׻���Ԥ� */
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

  /* ���եȤ��Ƥ��� */
  /* do last shift */
  memmove(r->window, &(r->window[para->frameshift]), sizeof(SP16) * (r->windowlen - para->frameshift));
  r->windownum -= para->frameshift;

  /* ����ǺƳ��ν��������ä��Τ�,�ޤ�������ν����ǻĤäƤ��������ǡ�������
     �������� */
  /* now that the search status has been prepared for the next input, we
     first process the rest unprocessed samples at the last session */
  if (r->rest_len > 0) {
#ifdef RDEBUG
    printf("Resume: rest %d samples\n", r->rest_len);
#endif
    return(RealTimePipeLine(r->rest_Speech, r->rest_len, recoglist, recognum));
  }

  /* ���������Ϥ��Ф���ǧ��������³���� */
  /* the recognition process will continue for the newly incoming samples... */
  return 0;
}
#endif /* SP_BREAK_CURRENT_FRAME */


/* ON-THE-FLY �ǥ����ǥ��󥰴ؿ�: ��λ���� */
/* ON-THE-FLY DECODING FUNCTION: end processing */
/** 
 * <JA>
 * ��1�ѥ�ʿ��ǧ�������ν�λ������Ԥ���
 * 
 * @return �������Ϥ���ħ�ѥ�᡼�����Ǽ������¤�Τ��֤���
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
    /* RealTimePipeLine ��ǧ������¦����ͳ�ˤ��ǧ�������Ǥ������,
       �����֤�MFCC�׻��ǡ����򤽤Τޤ޼�����ݻ�����ɬ�פ�����Τ�,
       MFCC�׻���λ������Ԥ鷺���裱�ѥ��η�̤Τ߽��Ϥ��ƽ���롥*/
    /* When input segmented by recognition process in RealTimePipeLine(),
       we have to keep the whole current status of MFCC computation to the
       next call.  So here we only output the 1st pass result. */
    for(ir=0;ir<recognum;ir++) {
      recoglist[ir]->backmax = finalize_1st_pass(recoglist[ir], r->last_time);
#ifdef SP_BREAK_CURRENT_FRAME
      finalize_segment(recoglist[ir], r->param, r->last_time);
#endif
      if (recoglist[ir]->model->gmm != NULL) {
	/* GMM �׻��ν�λ */
	gmm_end(recoglist[ir]);
      }
    }
    /* ���ζ�֤� param �ǡ������裲�ѥ��Τ�����֤� */
    /* return obtained parameter for 2nd pass */
    return(TRUE);
  }

  /* MFCC�׻��ν�λ������Ԥ�: �Ǹ���ٱ�ե졼��ʬ����� */
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
	    /* GMM �׻��ν���� */
	    gmm_prepare(recoglist[ir]);
	  }
	}
      }
      for(ir=0;ir<recognum;ir++) {
	if (recoglist[ir]->model->hmminfo->multipath || r->f != 0) {
	  get_back_trellis_proceed(r->f, r->param, recoglist[ir], FALSE);
	}
	if (recog->model->gmm != NULL) {
	  /* GMM �׻���Ԥ� */
	  gmm_proceed(recoglist[ir], r->param, r->f);
	}
      }
      r->f++;
    }
  }

  /* �ե졼��Ĺ�򥻥å� */
  /* set frame length */
  r->param->header.samplenum = r->f;
  r->param->samplenum = r->f;

  /* ����Ĺ���ǥ륿�η׻��˽�ʬ�Ǥʤ����,
     MFCC �� CMN �ޤ��������׻��Ǥ��ʤ����ᡤ���顼��λ�Ȥ��롥*/
  /* if input is short for compute all the delta coeff., terminate here */
  if (r->f == 0) {
    jlog("ERROR: too short input to compute delta coef! (%d frames)\n", r->f);
    for(ir=0;ir<recognum;ir++) {
      recoglist[ir]->backmax = finalize_1st_pass(recoglist[ir], r->param->samplenum);
    }
  } else {
    /* �裱�ѥ��ν�λ������Ԥ� */
    /* finalize 1st pass */
    for(ir=0;ir<recognum;ir++) {
      get_back_trellis_end(r->param, recoglist[ir]);
      recog->backmax = finalize_1st_pass(recoglist[ir], r->param->samplenum);
#ifdef SP_BREAK_CURRENT_FRAME
      finalize_segment(recoglist[ir], r->param, r->param->samplenum);
#endif
      if (recoglist[ir]->model->gmm != NULL) {
	/* GMM �׻��ν�λ */
	gmm_end(recoglist[ir]);
      }
    }
  }

  return(TRUE);
}

/** 
 * <JA>
 * �����ǧ���������� CMN�Ѥ˥��ץ��ȥ��ʿ�Ѥ򹹿����롥
 * 
 * @param param [in] ���ߤ����ϥѥ�᡼��
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
 * ��1�ѥ�ʿ��ǧ�����������ǻ��ν�λ������Ԥ���
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

  /* ����Ĺ���ǥ륿�η׻��˽�ʬ�Ǥʤ����,
     MFCC �� CMN �ޤ��������׻��Ǥ��ʤ����ᡤ���顼��λ�Ȥ��롥*/
  /* if input is short for compute all the delta coeff., terminate here */
  if (r->f == 0) {
    for(ir=0;ir<recognum;ir++) {
      finalize_1st_pass(recoglist[ir], param->samplenum);
    }
  } else {
    /* �裱�ѥ��ν�λ������Ԥ� */
    /* finalize 1st pass */
    for(ir=0;ir<recognum;ir++) {
      get_back_trellis_end(param, recoglist[ir]);
      finalize_1st_pass(recoglist[ir], param->samplenum);
#ifdef SP_BREAK_CURRENT_FRAME
      finalize_segment(recoglist[ir], param, param->samplenum);
#endif
      if (recoglist[ir]->model->gmm != NULL) {
      /* GMM �׻��ν�λ */
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
