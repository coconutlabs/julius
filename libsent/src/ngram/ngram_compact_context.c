/**
 * @file   ngram_compact_context.c
 * 
 * <JA>
 * @brief  N-gram構造体のバックオフデータのコンパクト化
 * </JA>
 * 
 * <EN>
 * @brief  Compaction of back-off elements in N-gram data.
 * </EN>
 * 
 * @author Akinobu Lee
 * @date   Sat Aug 11 11:50:58 2007
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

#include <sent/stddefs.h>
#include <sent/ngram2.h>

/** 
 *
 * Compaction of back-off elements in N-gram data.
 * 
 * @param ndata [i/o] N-gram information
 * @param n [i] N of N-gram
 * 
 * @return TRUE on success, or FALSE on failure.
 * 
 */
boolean
ngram_compact_context(NGRAM_INFO *ndata, int n)
{
  NNID i;
  int c;
  int dst;
  NNID ntmp;
  NGRAM_TUPLE_INFO *this, *up;

  this = &(ndata->d[n-1]);
  up   = &(ndata->d[n]);

  /* count number of valid bigram context */
  c = 0;
  for(i=0;i<up->bgnlistlen;i++) {
    if ((up->is24bit == TRUE && up->bgn_upper[i] != NNID_INVALID_UPPER)
	|| (up->is24bit == FALSE && up->bgn[i] != NNID_INVALID)) {
      c++;
      if (c >= NNIDMAX) {
	jlog("Error: ngram_compact_context: num of %d-gram valid context exceeds 24bit limit\n", n+1);
	return FALSE;
      }
    } else {
      if (up->num[i] != 0) {
	jlog("Error: ngram_compact_context: internal error\n");
	return FALSE;
      }
      if (this->bo_wt[i] != 0.0) {
	jlog("Error: ngram_compact_context: 2-gram has no upper 3-gram, but not 0.0 back-off weight\n");
	return FALSE;
      }
    }
  }
  
  this->context_num = c;
  jlog("Stat: ngram_compact_context: bigram bowt compaction: %d -> %d\n", this->totalnum, this->context_num);

  
  /* allocate index buffer */
  this->nnid2ctid_upper = (NNID_UPPER *)mymalloc(sizeof(NNID_UPPER) * this->totalnum);
  this->nnid2ctid_lower = (NNID_LOWER *)mymalloc(sizeof(NNID_LOWER) * this->totalnum);
  /* make index and do compaction of context informations */
  dst = 0;
  for(i=0;i<up->bgnlistlen;i++) {
    if ((up->is24bit == TRUE && up->bgn_upper[i] != NNID_INVALID_UPPER)
	|| (up->is24bit == FALSE && up->bgn[i] != NNID_INVALID)) {
      this->bo_wt[dst] = this->bo_wt[i];
      up->bgn_upper[dst] = up->bgn_upper[i];
      up->bgn_lower[dst] = up->bgn_lower[i];
      up->num[dst] = up->num[i];
      ntmp = dst & 0xffff;
      this->nnid2ctid_lower[i] = ntmp;
      ntmp = dst >> 16;
      this->nnid2ctid_upper[i] = ntmp;
      dst++;
    } else {
      this->nnid2ctid_upper[i] = NNID_INVALID_UPPER;
      this->nnid2ctid_lower[i] = 0;
    }
  }
  up->bgnlistlen = this->context_num;

  /* shrink the memory area */
  this->bo_wt = (LOGPROB *)myrealloc(this->bo_wt, sizeof(LOGPROB) * this->context_num);
  up->bgn_upper = (NNID_UPPER *)myrealloc(up->bgn_upper, sizeof(NNID_UPPER) * up->bgnlistlen);
  up->bgn_lower = (NNID_LOWER *)myrealloc(up->bgn_lower, sizeof(NNID_LOWER) * up->bgnlistlen);
  up->num = (WORD_ID *)myrealloc(up->num, sizeof(WORD_ID) * up->bgnlistlen);

  return TRUE;

}
