/**
 * @file   global.h
 * @author Akinobu Lee
 * @date   Sun Sep 18 23:53:17 2005
 * 
 * <JA>
 * @brief  大域変数の定義
 *
 * 値はデフォルト値として用いられます．
 *
 * </JA>
 * 
 * <EN>
 * @brief  Global variables
 *
 * Dfeault values are specified in this file.
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

#ifndef __J_GLOBAL_H__
#define __J_GLOBAL_H__

#include <sent/stddefs.h>
#include <sent/vocabulary.h>
#include <julius/wchmm.h>
#include <julius/search.h>

/**
 * If GLOBAL_VARIABLE_DEFINE is defined, global variables are actually made.
 * Else, these are external definition.
 * 
 */
#ifdef GLOBAL_VARIABLE_DEFINE
#define GLOBAL /*  */
#define GLOBAL_VAL(v) = (v)
#else
#define GLOBAL extern
#define GLOBAL_VAL(v) /*  */
#endif

/* global variables */
GLOBAL boolean verbose_flag GLOBAL_VAL(TRUE);
GLOBAL boolean debug2_flag GLOBAL_VAL(FALSE);

#endif /* __J_GLOBAL_H__ */
