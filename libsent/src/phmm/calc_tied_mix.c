/**
 * @file   calc_tied_mix.c
 * @author Akinobu LEE
 * @date   Thu Feb 17 14:22:44 2005
 * 
 * <JA>
 * @brief  混合ガウス分布の重みつき和の計算：tied-mixture用，キャッシュ有り
 *
 * Tied-mixture 用のガウス混合分布計算ではキャッシュが考慮されます．
 * 計算された混合分布の音響尤度はコードブック単位でフレームごとに
 * キャッシュされ，同じコードブックが同じ時間でアクセスされた場合は
 * そのキャッシュから値を返します．
 * </JA>
 * 
 * <EN>
 * @brief  Compute weighed sum of Gaussian mixture for tied-mixture model (cache enabled)
 *
 * In tied-mixture computation, the computed output probability of each
 * Gaussian component will be cache per codebook, for each input frame.
 * If the same codebook of the same time is accessed later, the cached
 * value will be returned.
 * </EN>
 * 
 * $Revision: 1.2 $
 * 
 */
/*
 * Copyright (c) 1991-2006 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2006 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#include <sent/stddefs.h>
#include <sent/speech.h>
#include <sent/htk_hmm.h>
#include <sent/htk_param.h>
#include <sent/hmm.h>
#include <sent/hmm_calc.h>


/** 
 * Initialize codebook cache area.
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
calc_tied_mix_init(HMMWork *wrk)
{
  wrk->mixture_cache = NULL;
  wrk->tmix_allocframenum = 0;
  wrk->mroot = NULL;
  wrk->tmix_last_id = (int *)mymalloc(sizeof(int) * wrk->OP_hmminfo->maxmixturenum);
  return TRUE;
}

/** 
 * Setup codebook cache for the next incoming input.
 * 
 * @param framenum [in] length of the next input.
 * 
 * @return TRUE on success, FALSE on failure.
 */
boolean
calc_tied_mix_prepare(HMMWork *wrk, int framenum)
{
  int bid, t;

  /* clear */
  for(t=0;t<wrk->tmix_allocframenum;t++) {
    for(bid=0;bid<wrk->OP_hmminfo->codebooknum;bid++) {
      wrk->mixture_cache[t][bid][0].score = LOG_ZERO;
    }
  }

  return TRUE;
}

/** 
 * Expand the cache to time axis if needed.
 * 
 * @param reqframe [in] required frame length
 */
static void
calc_tied_mix_extend(HMMWork *wrk, int reqframe)
{
  int newnum;
  int bid, t, size;
  
  /* if enough length are already allocated, return immediately */
  if (reqframe < wrk->tmix_allocframenum) return;

  /* allocate per certain period */
  newnum = reqframe + 1;
  if (newnum < wrk->tmix_allocframenum + OUTPROB_CACHE_PERIOD)
    newnum = wrk->tmix_allocframenum + OUTPROB_CACHE_PERIOD;

  if (wrk->mixture_cache == NULL) {
    wrk->mixture_cache = (MIXCACHE ***)mymalloc(sizeof(MIXCACHE **) * newnum);
  } else {
    wrk->mixture_cache = (MIXCACHE ***)myrealloc(wrk->mixture_cache, sizeof(MIXCACHE **) * newnum);
  }

  size = wrk->OP_gprune_num * wrk->OP_hmminfo->codebooknum;

  for(t = wrk->tmix_allocframenum; t < newnum; t++) {
    wrk->mixture_cache[t] = (MIXCACHE **)mybmalloc2(sizeof(MIXCACHE *) * wrk->OP_hmminfo->codebooknum, &(wrk->mroot));
    wrk->mixture_cache[t][0] = (MIXCACHE *)mybmalloc2(sizeof(MIXCACHE) * size, &(wrk->mroot));
    for(bid=1;bid<wrk->OP_hmminfo->codebooknum;bid++) {
      wrk->mixture_cache[t][bid] = &(wrk->mixture_cache[t][0][wrk->OP_gprune_num * bid]);
    }
    /* clear the new part */
    for(bid=0;bid<wrk->OP_hmminfo->codebooknum;bid++) {
      wrk->mixture_cache[t][bid][0].score = LOG_ZERO;
    }
  }

  wrk->tmix_allocframenum = newnum;
}

/** 
 * Free work area for tied-mixture calculation.
 * 
 */
void
calc_tied_mix_free(HMMWork *wrk)
{
  if (wrk->mroot != NULL) mybfree2(&(wrk->mroot));
  if (wrk->mixture_cache != NULL) free(wrk->mixture_cache);
  free(wrk->tmix_last_id);
}

/** 
 * @brief  Compute the output probability of current state OP_State on
 * tied-mixture model
 * 
 * This function assumes that the OP_state is assigned to a tied-mixture
 * codebook.  Here the output probability of Gaussian mixture component
 * referred by OP_state is consulted to the book level cache, and if not
 * computed yet on that input frame time, it will be computed here.
 *
 * @return the computed output probability in log10.
 */
LOGPROB
calc_tied_mix(HMMWork *wrk)
{
  GCODEBOOK *book = (GCODEBOOK *)(wrk->OP_state->b);
  LOGPROB logprob;
  int i, id;
  MIXCACHE *ttcache;
  MIXCACHE *last_ttcache;
  PROB *weight;

  weight = wrk->OP_state->bweight;

#if 0
  if (wrk->OP_last_time != wrk->OP_time) { /* different frame */
    if (wrk->OP_time >= 1) {
      last_tcache = wrk->mixture_cache[wrk->OP_time-1];
    } else {
      last_tcache = NULL;
    }
  }
#endif

  /* extend cache if needed */
  calc_tied_mix_extend(wrk, wrk->OP_time);
  ttcache = wrk->mixture_cache[wrk->OP_time][book->id];
  if (ttcache[0].score != LOG_ZERO) { /* already calced */
    /* calculate using cache and weight */
    for (i=0;i<wrk->OP_calced_num;i++) {
      wrk->OP_calced_score[i] = ttcache[i].score + weight[ttcache[i].id];
    }
  } else { /* not calced yet */
    /* compute Gaussian set */
    if (wrk->OP_time >= 1) {
      last_ttcache = wrk->mixture_cache[wrk->OP_time-1][book->id];
      if (last_ttcache[0].score != LOG_ZERO) {
	for(i=0;i<wrk->OP_gprune_num;i++) wrk->tmix_last_id[i] = last_ttcache[i].id;
	/* tell last calced best */
	(*(wrk->compute_gaussset))(wrk, book->d, book->num, wrk->tmix_last_id);
      } else {
	(*(wrk->compute_gaussset))(wrk, book->d, book->num, NULL);
      }
    } else {
      (*(wrk->compute_gaussset))(wrk, book->d, book->num, NULL);
    }
    /* computed Gaussians will be set in:
       score ... OP_calced_score[0..OP_calced_num]
       id    ... OP_calced_id[0..OP_calced_num] */
    /* OP_gprune_num = required, OP_calced_num = actually calced */
    /* store to cache */
    for (i=0;i<wrk->OP_calced_num;i++) {
      id = wrk->OP_calced_id[i];
      ttcache[i].id = id;
      ttcache[i].score = wrk->OP_calced_score[i];
      /* now OP_calced_{id|score} can be used for work area */
      wrk->OP_calced_score[i] += weight[id];
    }
  }
  logprob = addlog_array(wrk->OP_calced_score, wrk->OP_calced_num);
  if (logprob <= LOG_ZERO) return LOG_ZERO;
  return (logprob * INV_LOG_TEN);
}  
