/**
 * @file   dfa_util.c
 * @author Akinobu LEE
 * @date   Tue Feb 15 14:18:36 2005
 * 
 * <JA>
 * @brief  文法の情報をテキストで出力
 * </JA>
 * 
 * <EN>
 * @brief  Output text informations about the grammar
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
#include <sent/dfa.h>

/** 
 * Output overall grammar information to stdout.
 * 
 * @param dinfo [in] DFA grammar
 */
void
print_dfa_info(FILE *fp, DFA_INFO *dinfo)
{
  if (fp == NULL) return;
  fprintf(fp, "DFA grammar info:\n");
  fprintf(fp, "      %d nodes, %d arcs, %d terminal(category) symbols\n",
	 dinfo->state_num, dinfo->arc_num, dinfo->term_num);
  fprintf(fp, "      category-pair matrix size is %d bytes\n",
	 sizeof(unsigned char) * dinfo->term_num * dinfo->term_num / 8);
}

/** 
 * Output the category-pair matrix in text format to stdout
 * 
 * @param dinfo [in] DFA grammar that holds category pair matrix
 */
void
print_dfa_cp(FILE *fp, DFA_INFO *dinfo)
{
  int i,j;
  int t;

  if (fp == NULL) return;
  fprintf(fp, "---------- terminal(category)-pair matrix ----------\n");
  /* horizontal ruler */
  fprintf(fp, "    ");
  for (j=0;j<dinfo->term_num;j++) {
    if (j > 0 && (j % 10) == 0) {
      t = j / 10;
      fprintf(fp, "%1d", t);
    } else {
      fprintf(fp, " ");
    }
  }
  fprintf(fp, "\n    ");
  for (j=0;j<dinfo->term_num;j++) {
    fprintf(fp, "%1d", j % 10);
  }
  fprintf(fp, "\n");
  
  fprintf(fp, "bgn ");
  for (j=0;j<dinfo->term_num;j++) {
    fprintf(fp, (dfa_cp_begin(dinfo, j) == TRUE) ? "o" : " ");
  }
  fprintf(fp, "\n");
  fprintf(fp, "end ");
  for (j=0;j<dinfo->term_num;j++) {
    fprintf(fp, (dfa_cp_end(dinfo, j) == TRUE) ? "o" : " ");
  }
  fprintf(fp, "\n");
  for (i=0;i<dinfo->term_num;i++) {
    fprintf(fp, "%3d ",i);
    for (j=0;j<dinfo->term_num;j++) {
      fprintf(fp, (dfa_cp(dinfo, i, j) == TRUE) ? "o" : " ");
    }
    fprintf(fp, "\n");
  }
}
