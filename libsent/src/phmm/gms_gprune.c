/**
 * @file   gms_gprune.c
 * @author Akinobu LEE
 * @date   Thu Feb 17 15:05:08 2005
 * 
 * <JA>
 * @brief  Gaussian Mixture Selection のための Gaussian pruning を用いたモノフォンHMMの計算
 * </JA>
 * 
 * <EN>
 * @brief  Calculate the GMS monophone %HMM for Gaussian Mixture Selection using Gaussian pruning
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

#include <sent/stddefs.h>
#include <sent/htk_hmm.h>
#include <sent/htk_param.h>
#include <sent/hmm.h>
#include <sent/hmm_calc.h>

/* activate experimental methods */
#define GS_MAX_PROB		///< Compute only max for GS states
#define LAST_BEST		///< Compute last best Gaussians first

/************************************************************************/
/** 
 * Initialization of GMS %HMM likelihood computation.
 * 
 * @param hmminfo [in] GMS %HMM definition
 * @param gsset_num [in] number of states in GMS %HMM
 */
void
gms_gprune_init(HMMWork *wrk)
{
  wrk->gms_last_max_id = (int *)mymalloc(sizeof(int) * wrk->gsset_num);
}

/** 
 * Prepare GMS %HMM computation for the next speech input.
 * 
 */
void
gms_gprune_prepare(HMMWork *wrk)
{
  int i;
  for(i=0;i<wrk->gsset_num;i++) {
    wrk->gms_last_max_id[i] = -1;
  }
}

/**
 * Free GMS related work area.
 * 
 */
void
gms_gprune_free(HMMWork *wrk)
{
  free(wrk->gms_last_max_id);
}

/**********************************************************************/
/* LAST_BEST ... compute the maximum component in last frame first */
/** 
 * Compute only max by safe pruning
 * 
 * @param binfo [in] Gaussian density
 * @param thres [in] constant pruning threshold
 * 
 * @return the computed likelihood.
 */
static LOGPROB
calc_contprob_with_safe_pruning(HMMWork *wrk, HTK_HMM_Dens *binfo, LOGPROB thres)
{
  LOGPROB tmp, x;
  VECT *mean;
  VECT *var;
  LOGPROB fthres = thres * (-2.0);
  VECT *vec = wrk->OP_vec;
  short veclen = wrk->OP_veclen;

  if (binfo == NULL) return(LOG_ZERO);
  mean = binfo->mean;
  var = binfo->var->vec;

  tmp = binfo->gconst;
  for (; veclen > 0; veclen--) {
    x = *(vec++) - *(mean++);
    tmp += x * x * *(var++);
    if ( tmp > fthres) {
      return LOG_ZERO;
    }
  }
  return(tmp * -0.5);
}

#ifdef LAST_BEST

/** 
 * Compute log output likelihood of a state.  Only maximum Gaussian will be
 * computed.
 * 
 * @param stateinfo [in] %HMM state to compute
 * @param last_maxi [in] the mixture id that got the maximum value at the previous frame, or -1 if not exist.
 * @param maxi_ret [out] tue mixture id that get the maximum value at this call.
 * 
 * @return the log likelihood.
 */
static LOGPROB
compute_g_max(HMMWork *wrk, HTK_HMM_State *stateinfo, int last_maxi, int *maxi_ret)
{
  int i, maxi;
  LOGPROB prob;
  LOGPROB maxprob = LOG_ZERO;

  if (last_maxi != -1) {
    maxi = last_maxi;
    maxprob = calc_contprob_with_safe_pruning(wrk, stateinfo->b[maxi], LOG_ZERO);
    for (i = stateinfo->mix_num - 1; i >= 0; i--) {
      if (i == last_maxi) continue;
      prob = calc_contprob_with_safe_pruning(wrk, stateinfo->b[i], maxprob);
      if (prob > maxprob) {
	maxprob = prob;
	maxi = i;
      }
    }
    *maxi_ret = maxi;
  } else {
    maxi = stateinfo->mix_num - 1;
    maxprob = calc_contprob_with_safe_pruning(wrk, stateinfo->b[maxi],  LOG_ZERO);
    i = maxi - 1;
    for (; i >= 0; i--) {
      prob = calc_contprob_with_safe_pruning(wrk, stateinfo->b[i], maxprob);
      if (prob > maxprob) {
	maxprob = prob;
	maxi = i;
      }
    }
    *maxi_ret = maxi;
  }

  return((maxprob + stateinfo->bweight[maxi]) * INV_LOG_TEN);
}
  
#else  /* ~LAST_BEST */
  
/** 
 * Compute log output likelihood of a state.  Only maximum Gaussian will be
 * computed.
 * 
 * @param stateinfo [in] %HMM state to compute
 * 
 * @return the log likelihood.
 */
static LOGPROB
compute_g_max(HMMWork *wrk, HTK_HMM_State *stateinfo)
{
  int i, maxi;
  LOGPROB prob;
  LOGPROB maxprob = LOG_ZERO;

  i = maxi = stateinfo->mix_num - 1;
  for (; i >= 0; i--) {
    prob = calc_contprob_with_safe_pruning(wrk, stateinfo->b[i], maxprob);
    if (prob > maxprob) {
      maxprob = prob;
      maxi = i;
    }
  }
  return((maxprob + stateinfo->bweight[maxi]) * INV_LOG_TEN);
}
#endif

/**********************************************************************/
/* main function: compute all gshmm scores */
/* *** assume to be called for sequencial frame (using last result) */

/** 
 * Main function to compute all the GMS %HMM states in a frame
 * with the input vectore specified by OP_vec.  This function assumes
 * that this will be called for sequencial frame, since it utilizes the
 * result of previous frame for faster pruning.
 * 
 * @param gsset [in] list of GMS %HMM state set.
 * @param gsset_num [in] length of above
 * @param scores_ret [out] array of scores for each GMS %HMM state
 */
void
compute_gs_scores(HMMWork *wrk)
{
  int i;
#ifdef LAST_BEST
  int max_id;
#endif

  for (i=0;i<wrk->gsset_num;i++) {
#ifdef GS_MAX_PROB
#ifdef LAST_BEST
    /* compute only the maximum with pruning (last best first) */
    wrk->t_fs[i] = compute_g_max(wrk, wrk->gsset[i].state, wrk->gms_last_max_id[i], &max_id);
    wrk->gms_last_max_id[i] = max_id;
#else
    wrk->t_fs[i] = compute_g_max(wrk, wrk->gsset[i].state);
#endif /* LAST_BEST */
#else
    /* compute all mixture */
    wrk->t_fs[i] = compute_g_base(wrk, wrk->gsset[i].state);
#endif
    /*printf("%d:%s:%f\n",i,gsset[i].book->name,t_fs[i]);*/
  }

}
