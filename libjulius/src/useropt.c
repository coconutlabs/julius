/**
 * @file   useropt.c
 * @author Akinobu Lee
 * @date   Sun Sep 02 19:44:37 2007
 * 
 * <JA>
 * @brief  Julius ユーザオプション
 * </JA>
 * 
 * <EN>
 * @brief  Julius user option handling
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

#include <julius/juliuslib.h>

/**
 * List of user option data
 * 
 */
static USEROPT *useropt_root = NULL;

static USEROPT *
useropt_new()
{
  USEROPT *new;

  new = (USEROPT *)mymalloc(sizeof(USEROPT));
  new->optstr = NULL;
  new->desc = NULL;
  new->argnum = 0;
  new->next = NULL;

  return new;
}
static void
useropt_free(USEROPT *x)
{
  if (x->optstr) free(x->optstr);
  if (x->desc) free(x->desc);
  free(x);
}

void
useropt_free_all()
{
  USEROPT *x, *tmp;

  x = useropt_root;
  while(x) {
    tmp = x->next;
    useropt_free(x);
    x = tmp;
  }
  useropt_root = NULL;
}
  

boolean
j_add_option(char *fmt, int argnum, char *desc, boolean (*func)(Jconf *jconf, char *arg[], int argnum))
{
  USEROPT *new;
  int i;

  /* allocate new */
  new = useropt_new();

  /* set option string */
  if (fmt[0] != '-') {
    jlog("Warning: j_add_option: option string not start with \'-\': %s\n", fmt);
  }
  new->optstr = strcpy((char *)mymalloc(strlen(fmt)+1), fmt);
  /* set number of arguments */
  new->argnum = argnum;
  /* set description string */
  new->desc = strcpy((char*)mymalloc(strlen(desc)+1),desc);

  /* set user-given function to process this option */
  new->func = func;

  /* add to list */
  new->next = useropt_root;
  useropt_root = new;

  return TRUE;
}


/* 1: processed  0: nothing processed  -1: error */
int
useropt_exec(Jconf *jconf, char *argv[], int argc, int *n)
{
  USEROPT *x;
  int i;

  for(x=useropt_root;x;x=x->next) {
    if (strmatch(argv[*n], x->optstr)) {
      if (*n + x->argnum >= argc) {
	jlog("ERROR: useropt_exec: \"%s\" requires %d argument(s)\n", x->optstr, x->argnum);
	return -1;		/* error */
      }
      if ((*(x->func))(jconf, &(argv[(*n)+1]), x->argnum) == FALSE) {
	jlog("ERROR: useropt_exec: \"%s\" function returns FALSE\n", x->optstr);
	return -1;		/* error */
      }
      *n += x->argnum;
      return 1;			/* processed */
    }
  }

  return 0;			/* nothing processed */
}

void
useropt_show_desc(FILE *fp)
{
  USEROPT *x;
  int i;

  if (! useropt_root) return;
  fprintf(fp, "\n Additional options for application:\n");
  for(x=useropt_root;x;x=x->next) {
    fprintf(fp, "    [%s", x->optstr);
    for(i=0;i<x->argnum;i++) fprintf(fp, " arg");
    fprintf(fp, "]\t%s\n", x->desc);
  }
}

  
