/**
 * @file   wav2mfcc-pipe.c
 * 
 * <JA>
 * @brief  音声波形から MFCC 特徴量へ変換する (フレーム単位)
 *
 * ここでは wav2mfcc.c の関数をフレーム同期に処理するために変換した
 * 関数が納められています．認識処理を音声入力と平行して行う場合，こちらの
 * 関数が用いられます．
 * </JA>
 * 
 * <EN>
 * @brief  Convert speech inputs into MFCC parameter vectors (per input frame)
 *
 * There are functions are derived from wav2mfcc.c, to compute
 * MFCC vectors in per-frame basis.  When performing on-line recognition,
 * these functions will be used instead of ones in wav2mfcc.c
 * </EN>
 * 
 * @author Akinobu LEE
 * @date   Thu Feb 17 18:12:30 2005
 *
 * $Revision: 1.2 $
 * 
 */
/*
 * Copyright (c) 1991-2007 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2007 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

/* wav2mfcc-pipe.c --- split Wav2MFCC to perform per-frame-basis,
   and also realtime CMN for 1st-pass pipe-lining */

/************************************************************************/
/*    wav2mfcc.c   Convert Speech file to MFCC_E_D_(Z) file             */
/*----------------------------------------------------------------------*/
/*    Author    : Yuichiro Nakano                                       */
/*                                                                      */
/*    Copyright(C) Yuichiro Nakano 1996-1998                            */
/*----------------------------------------------------------------------*/
/************************************************************************/


#include <sent/stddefs.h>
#include <sent/mfcc.h>

/***********************************************************************/
/** 
 * Allocate a new delta cycle buffer.
 * 
 * @param veclen [in] length of a vector
 * @param windowlen [in] window width for computing delta
 * 
 * @return pointer to newly allocated delta cycle buffer structure.
 */
DeltaBuf *
WMP_deltabuf_new(int veclen, int windowlen)
{
  int i;
  DeltaBuf *db;

  db = (DeltaBuf *)mymalloc(sizeof(DeltaBuf));
  db->veclen = veclen;
  db->win = windowlen;
  db->len = windowlen * 2 + 1;
  db->mfcc = (float **)mymalloc(sizeof(float *) * db->len);
  db->is_on = (boolean *) mymalloc(sizeof(boolean) * db->len);
  for (i=0;i<db->len;i++) {
    db->mfcc[i] = (float *)mymalloc(sizeof(float) * veclen * 2);
  }
  db->B = 0;
  for(i = 1; i <= windowlen; i++) db->B += i * i;
  db->B *= 2;

  return (db);
}

/** 
 * Destroy the delta cycle buffer.
 * 
 * @param db [i/o] delta cycle buffer
 */
void
WMP_deltabuf_free(DeltaBuf *db)
{
  int i;

  for (i=0;i<db->len;i++) {
    free(db->mfcc[i]);
  }
  free(db->is_on);
  free(db->mfcc);
  free(db);
}

/** 
 * Reset and clear the delta cycle buffer.
 * 
 * @param db [i/o] delta cycle buffer
 */
void
WMP_deltabuf_prepare(DeltaBuf *db)
{
  int i;
  db->store = 0;
  for (i=0;i<db->len;i++) {
    db->is_on[i] = FALSE;
  }
}

/** 
 * Calculate delta coefficients of the specified point in the cycle buffer.
 *
 * @param db [i/o] delta cycle buffer
 * @param cur [in] target point to calculate the delta coefficients
 */
static void
WMP_deltabuf_calc(DeltaBuf *db, int cur)
{
  int n, theta, p;
  float A1, A2, sum;
  int last_valid_left, last_valid_right;
  
  for (n = 0; n < db->veclen; n++) {
    sum = 0.0;
    last_valid_left = last_valid_right = cur;
    for (theta = 1; theta <= db->win; theta++) {
      p = cur - theta;
      if (p < 0) p += db->len;
      if (db->is_on[p]) {
	A1 = db->mfcc[p][n];
	last_valid_left = p;
      } else {
	A1 = db->mfcc[last_valid_left][n];
      }
      p = cur + theta;
      if (p >= db->len) p -= db->len;
      if (db->is_on[p]) {
	A2 = db->mfcc[p][n];
	last_valid_right = p;
      } else {
	A2 = db->mfcc[last_valid_right][n];
      }
      sum += theta * (A2 - A1);
    }
    db->mfcc[cur][db->veclen + n] = sum / db->B;
  }
}

/** 
 * Store the given MFCC vector into the delta cycle buffer, and compute the
 * latest delta coefficients.
 * 
 * @param db [i/o] delta cycle buffer
 * @param new_mfcc [in] MFCC vector
 * 
 * @return TRUE if next delta coeff. computed, in that case it is saved
 * in db->delta[], or FALSE if delta is not yet computed by short of data.
 */
boolean
WMP_deltabuf_proceed(DeltaBuf *db, float *new_mfcc) 
{
  int cur;
  boolean ret;

  /* copy data to store point */
  memcpy(db->mfcc[db->store], new_mfcc, sizeof(float) * db->veclen);
  db->is_on[db->store] = TRUE;

  /* get current calculation point */
  cur = db->store - db->win;
  if (cur < 0) cur += db->len;

  /* if the current point is fulfilled, compute delta  */
  if (db->is_on[cur]) {
    WMP_deltabuf_calc(db, cur);
    db->vec = db->mfcc[cur];
    ret = TRUE;
  } else {
    ret = FALSE;
  }

  /* move store pointer to next */
  db->store++;
  if (db->store >= db->len) db->store -= db->len;

  /* return TRUE if delta computed for current, or -1 if not calculated yet */
  return (ret);
}

/** 
 * Flush the delta cycle buffer the delta coefficients
 * left in the cycle buffer.
 * 
 * @param db [i/o] delta cycle buffer
 * 
 * @return TRUE if next delta coeff. computed, in that case it is saved
 * in db->delta[], or FALSE if all delta computation has been flushed and
 * no data is available.
 *
 */
boolean
WMP_deltabuf_flush(DeltaBuf *db) 
{
  int cur;
  boolean ret;

  /* clear store point */
  db->is_on[db->store] = FALSE;

  /* get current calculation point */
  cur = db->store - db->win;
  if (cur < 0) cur += db->len;

  /* if the current point if fulfilled, compute delta  */
  if (db->is_on[cur]) {
    WMP_deltabuf_calc(db, cur);
    db->vec = db->mfcc[cur];
    ret = TRUE;
  } else {
    ret = FALSE;
  }

  /* move store pointer to next */
  db->store++;
  if (db->store >= db->len) db->store -= db->len;

  /* return TRUE if delta computed for current, or -1 if not calculated yet */
  return (ret);
}

/***********************************************************************/
/* MAP-CMN */
/***********************************************************************/

/**
 * Initialize MAP-CMN at startup.
 * 
 * @param dimension [in] vector dimension
 * @param weight [in] initial cepstral mean weight
 * 
 */
CMNWork *
CMN_realtime_new(int dimension, float weight)
{
  int i;

  CMNWork *c;

  c = (CMNWork *)mymalloc(sizeof(CMNWork));

  c->dim = dimension;
  c->cweight = weight;

  c->clist_max = CPSTEP;
  c->clist_num = 0;
  c->clist = (CMEAN *)mymalloc(sizeof(CMEAN) * c->clist_max);
  for(i=0;i<c->clist_max;i++) {
    c->clist[i].mfcc_sum = (float *)mymalloc(sizeof(float)*c->dim);
    c->clist[i].framenum = 0;
  }
  c->now.mfcc_sum = (float *)mymalloc(sizeof(float) * c->dim);

  c->cmean_init = (float *)mymalloc(sizeof(float) * c->dim);
  c->cmean_init_set = FALSE;

  return c;
}

/** 
 * Free work area for real-time CMN.
 * 
 * @param c [i/o] CMN calculation work area
 * 
 */
void
CMN_realtime_free(CMNWork *c)
{
  int i;

  free(c->cmean_init);
  free(c->now.mfcc_sum);
  for(i=0;i<c->clist_max;i++) {
    free(c->clist[i].mfcc_sum);
  }
  free(c->clist);
  free(c);
}

/**
 * Prepare for MAP-CMN at start of each input
 * 
 * @param c [i/o] CMN calculation work area
 */
void
CMN_realtime_prepare(CMNWork *c)
{
  int d;
  
  for(d=0;d<c->dim;d++) c->now.mfcc_sum[d] = 0.0;
  c->now.framenum = 0;
}

/**
 * Perform MAP-CMN for incoming MFCC vectors
 * 
 * @param c [i/o] CMN calculation work area
 * @param mfcc [in] MFCC vector
 * 
 */
void
CMN_realtime(CMNWork *c, float *mfcc)
{
  int d;
  double x, y;

  c->now.framenum++;
  if (c->cmean_init_set) {
    for(d=0;d<c->dim;d++) {
      /* accumulate value of given MFCC to sum */
      c->now.mfcc_sum[d] += mfcc[d];
      /* calculate map-cmn and perform subtraction to the given vector */
      x = c->now.mfcc_sum[d] + c->cweight * c->cmean_init[d];
      y = (double)c->now.framenum + c->cweight;
      mfcc[d] -= x / y;
    }
  } else {
    for(d=0;d<c->dim;d++) {
      c->now.mfcc_sum[d] += mfcc[d];
      mfcc[d] -= c->now.mfcc_sum[d] / c->now.framenum;
    }
  }
}

/**
 * Update initial cepstral mean from previous utterances for next input.
 * 
 * @param c [i/o] CMN calculation work area
 */
void
CMN_realtime_update(CMNWork *c)
{
  float *tmp;
  int i, d;
  int frames;

  /* if CMN_realtime was never called before this, return immediately */
  /* this may occur by pausing just after startup */
  if (c->now.framenum == 0) return;

  /* compute cepstral mean from now and previous sums up to CPMAX frames */
  for(d=0;d<c->dim;d++) c->cmean_init[d] = c->now.mfcc_sum[d];
  frames = c->now.framenum;
  for(i=0;i<c->clist_num;i++) {
    for(d=0;d<c->dim;d++) c->cmean_init[d] += c->clist[i].mfcc_sum[d];
    frames += c->clist[i].framenum;
    if (frames >= CPMAX) break;
  }
  for(d=0;d<c->dim;d++) c->cmean_init[d] /= (float) frames;
  c->cmean_init_set = TRUE;

  /* expand clist if neccessary */
  if (c->clist_num == c->clist_max && frames < CPMAX) {
    c->clist_max += CPSTEP;
    c->clist = (CMEAN *)myrealloc(c->clist, sizeof(CMEAN) * c->clist_max);
    for(i=c->clist_num;i<c->clist_max;i++) {
      c->clist[i].mfcc_sum = (float *)mymalloc(sizeof(float)*c->dim);
      c->clist[i].framenum = 0;
    }
  }
  
  /* shift clist */
  tmp = c->clist[c->clist_max-1].mfcc_sum;
  memmove(&(c->clist[1]), &(c->clist[0]), sizeof(CMEAN) * (c->clist_max - 1));
  c->clist[0].mfcc_sum = tmp;
  /* copy now to clist[0] */
  memcpy(c->clist[0].mfcc_sum, c->now.mfcc_sum, sizeof(float) * c->dim);
  c->clist[0].framenum = c->now.framenum;

  if (c->clist_num < c->clist_max) c->clist_num++;

}

/** 
 * Read binary with byte swap (assume file is Big Endian)
 * 
 * @param buf [out] data buffer
 * @param unitbyte [in] size of unit in bytes
 * @param unitnum [in] number of units to be read
 * @param fp [in] file pointer
 * 
 * @return TRUE if required number of units are fully read, FALSE if failed.
 */
static boolean
myread(void *buf, size_t unitbyte, int unitnum, FILE *fp)
{
  if (myfread(buf, unitbyte, unitnum, fp) < (size_t)unitnum) {
    return(FALSE);
  }
#ifndef WORDS_BIGENDIAN
  swap_bytes(buf, unitbyte, unitnum);
#endif
  return(TRUE);
}

/** 
 * Write binary with byte swap (assume data is Big Endian)
 * 
 * @param buf [in] data buffer
 * @param unitbyte [in] size of unit in bytes
 * @param unitnum [in] number of units to write
 * @param fd [in] file descriptor
 * 
 * @return TRUE if required number of units are fully written, FALSE if failed.
 */
static boolean
mywrite(void *buf, size_t unitbyte, size_t unitnum, int fd)
{
#ifndef WORDS_BIGENDIAN
  swap_bytes(buf, unitbyte, unitnum);
#endif
  if (write(fd, buf, unitbyte * unitnum) < unitbyte * unitnum) {
    return(FALSE);
  }
#ifndef WORDS_BIGENDIAN
  swap_bytes(buf, unitbyte, unitnum);
#endif
  return(TRUE);
}

/** 
 * Load CMN parameter from file.  If the number of MFCC dimension in the
 * file does not match the specified one, an error will occur.
 * 
 * @param c [i/o] CMN calculation work area
 * @param filename [in] file name
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
CMN_load_from_file(CMNWork *c, char *filename)
{
  FILE *fp;
  int veclen;

  jlog("Stat: wav2mfcc-pipe: reading initial CMN from file \"%s\"\n", filename);
  if ((fp = fopen_readfile(filename)) == NULL) {
    jlog("Error: wav2mfcc-pipe: failed to open\n");
    return(FALSE);
  }
  /* read header */
  if (myread(&veclen, sizeof(int), 1, fp) == FALSE) {
    jlog("Error: wav2mfcc-pipe: failed to read header\n");
    fclose_readfile(fp);
    return(FALSE);
  }
  /* check length */
  if (veclen != c->dim) {
    jlog("Error: wav2mfcc-pipe: vector dimension mismatch\n");
    fclose_readfile(fp);
    return(FALSE);
  }
  /* read body */
  if (myread(c->cmean_init, sizeof(float), c->dim, fp) == FALSE) {
    jlog("Error: wav2mfcc-pipe: failed to read\n");
    fclose_readfile(fp);
    return(FALSE);
  }
  if (fclose_readfile(fp) == -1) {
    jlog("Error: wav2mfcc-pipe: failed to close\n");
    return(FALSE);
  }

  c->cmean_init_set = TRUE;
  jlog("Stat: wav2mfcc-pipe: read CMN parameter\n");

  return(TRUE);
}

/** 
 * Save the current CMN vector to a file.
 * 
 * @param c [i/o] CMN calculation work area
 * @param filename [in] filename to save the data.
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
CMN_save_to_file(CMNWork *c, char *filename)
{
  int fd;

  jlog("Stat: wav2mfcc-pipe: writing current CM to file \"%s\"\n", filename);

  if ((fd = creat(filename, 0644)) == -1) {
    jlog("Error: wav2mfcc-pipe: failed to open\n");
    return(FALSE);
  }
  /* write header */
  if (mywrite(&(c->dim), sizeof(int), 1, fd) == FALSE) {
    jlog("Error: wav2mfcc-pipe: failed to write header\n");
    close(fd);
    return(FALSE);
  }
  /* write body */
  if (mywrite(c->cmean_init, sizeof(float), c->dim, fd) == FALSE) {
    jlog("Error: wav2mfcc-pipe: failed to write header\n");
    close(fd);
    return(FALSE);
  }
  close(fd);

  jlog("Stat: wav2mfcc-pipe: wrote current CM\n");
  
  return(TRUE);
}


/***********************************************************************/
/* energy normalization and scaling on live input */
/***********************************************************************/

/** 
 * Initialize work area for energy normalization on live input.
 * This should be called once on startup.
 * 
 * @param energy [in] energy normalization work area
 * 
 */
void
energy_max_init(ENERGYWork *energy)
{
  energy->max = 5.0;
}

/**
 * Prepare values for energy normalization on live input.
 * This should be called before each input segment.
 * 
 * @param energy [in] energy normalization work area
 * @param para [in] MFCC computation configuration parameter
 */
void
energy_max_prepare(ENERGYWork *energy, Value *para)
{
  energy->max_last = energy->max;
  energy->min_last = energy->max - (para->silFloor * LOG_TEN) / 10.0;
  energy->max = 0.0;
}

/** 
 * Peform energy normalization using maximum of last input.
 * 
 * @param energy [in] energy normalization work area
 * @param f [in] raw energy
 * @param para [in] MFCC computation configuration parameter
 * 
 * @return value of the normalized log energy.
 */
LOGPROB
energy_max_normalize(ENERGYWork *energy, LOGPROB f, Value *para)
{
  if (energy->max < f) energy->max = f;
  if (f < energy->min_last) f = energy->min_last;
  return(1.0 - (energy->max_last - f) * para->escale);
}
