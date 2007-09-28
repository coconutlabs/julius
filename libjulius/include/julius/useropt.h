/**
 * @file   useropt.h
 * @author Akinobu Lee
 * @date   Sun Sep 02 03:09:12 2007
 * 
 * <JA>
 * @brief  ユーザ指定の jconf オプション拡張
 * </JA>
 * 
 * <EN>
 * @brief  User-defined jconf options
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

#ifndef __J_USEROPT_H__
#define __J_USEROPT_H__

typedef struct __j_useropt__ {
  char *optstr;
  char *desc;
  int argnum;
  boolean (*func)(Jconf *jconf, char *arg[], int argnum);
  struct __j_useropt__ *next;
} USEROPT;

boolean j_add_option(char *fmt, int argnum, char *desc, boolean (*func)(Jconf *jconf, char *arg[], int argnum));
void useropt_free_all();
int useropt_exec(Jconf *jconf, char *argv[], int argc, int *n);
void useropt_show_desc(FILE *fp);


#endif /* __J_USEROPT_H__ */
