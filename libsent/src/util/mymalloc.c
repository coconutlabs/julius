/**
 * @file   mymalloc.c
 * 
 * <JA>
 * @brief  ưŪ������ݤ�Ԥ��ؿ�
 *
 * ���顼�������ä���硤¨�¤˥��顼��λ���ޤ���
 * </JA>
 * 
 * <EN>
 * @brief  Dynamic memory allocation funtions
 *
 * When allocation error occured within these functions, the program will
 * exit immediately.
 * </EN>
 * 
 * @author Akinobu LEE
 * @date   Thu Feb 17 16:27:03 2005
 *
 * $Revision: 1.4 $
 * 
 */
/*
 * Copyright (c) 1991-2007 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2007 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

#include <sent/stddefs.h>


/** 
 * Allocate a memory, as the same as malloc.
 * 
 * @param size [in] required size in bytes.
 * 
 * @return pointer to the the newly allocated area.
 */
void *
mymalloc(size_t size)
{
  void *p;
  if ( (p = malloc(size)) == NULL) {
    jlog("Error: mymalloc: failed to allocate %zu bytes\n", size);
    exit(1);
  }
  return p;
}

/** 
 * Allocate a memory for huge block, check for limit
 * 
 * @param size [in] required size in bytes.
 * 
 * @return pointer to the the newly allocated area.
 */
void *
mymalloc_big(size_t elsize, size_t nelem)
{
  void *p;
  double limit;

  if (sizeof(size_t) == 4) {	/* 32bit environment */
    limit = (double)4294967296.0 / elsize; /* 2^32 */
    if (nelem >= limit) {
      jlog("Error: mymalloc_big: %zu bytes x %zu unit exceeds 4GB limit\n", elsize, nelem);
      exit(1);
    }
  }
  if ( (p = malloc(nelem * elsize)) == NULL) {
    jlog("Error: mymalloc: failed to allocate %zu bytes\n", nelem * elsize);
    exit(1);
  }
  return p;
}

/** 
 * Re-allocate memory area, keeping the existing data, as the same as realloc.
 *
 * @param ptr [in] memory pointer to be re-allocated
 * @param size [in] required new size in bytes
 * 
 * @return pointer to the the newly allocated area with existing data.
 */
void *
myrealloc(void *ptr, size_t size)
{
  void *p;
  if ( (p = realloc(ptr,size)) == NULL) {
    jlog("Error: mymalloc: failed to reallocate %zu bytes\n", size);
    exit(1);
  }
  return p;
}

/** 
 * Allocate memory area and set it to zero, as the same as calloc.
 * 
 * @param nelem [in] size of element in bytes
 * @param elsize [in] number of elements to allocate
 * 
 * @return pointer to the newly allocated area.
 */
void *
mycalloc(size_t nelem, size_t elsize)
{
  void *p;
  if ( (p = calloc(nelem,elsize)) == NULL) {
    jlog("Error: mymalloc: failed to clear-allocate %zu bytes\n", nelem*elsize);
    exit(1);
  }
  return p;
}

