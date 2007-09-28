/**
 * @file   calc_mix.c
 * @author Akinobu LEE
 * @date   Thu Feb 17 14:18:52 2005
 * 
 * <JA>
 * @brief  混合ガウス分布の重みつき和の計算：非 tied-mixture 用，キャッシュ無し
 * </JA>
 * 
 * <EN>
 * @brief Compute weighed sum of Gaussian mixture for non tied-mixture model (no cache)
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

/** 
 * @brief  Compute the output probability of current state OP_State.
 *
 * No codebook-level cache is done.  
 * 
 * @return the output probability of the state OP_State in log10
 */
LOGPROB
calc_mix(HMMWork *wrk)
{
  int i;
  LOGPROB logprob = LOG_ZERO;
  int n;
  LOGPROB *s;
  PROB *w;
  int *id;

  /* compute Gaussian set */
  (*(wrk->compute_gaussset))(wrk, wrk->OP_state->b, wrk->OP_state->mix_num, NULL);
  /* computed Gaussians will be set in:
     score ... OP_calced_score[0..OP_calced_num]
     id    ... OP_calced_id[0..OP_calced_num] */
  
  n = wrk->OP_calced_num;
  s = wrk->OP_calced_score;
  w = wrk->OP_state->bweight;
  id = wrk->OP_calced_id;

  /* sum */
  for(i=0;i<n;i++) {
    s[i] += w[id[i]];
  }
  logprob = addlog_array(s, n);
  if (logprob <= LOG_ZERO) return LOG_ZERO;
  return (logprob * INV_LOG_TEN);
}
