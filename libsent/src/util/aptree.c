/**
 * @file   aptree.c
 * 
 * <JA>
 * @brief  パトリシア検索木を用いた名前検索：データ型がポインタの場合
 * </JA>
 * 
 * <EN>
 * @brief  Patricia index tree for name lookup: data type = pointer
 * </EN>
 * 
 * @author Akinobu LEE
 * @date   Thu Feb 17 15:21:53 2005
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
#include <sent/ptree.h>

/** 
 * Allocate a new node.
 * 
 * 
 * @return pointer to the new node.
 */
static APATNODE *
new_node()
{
  APATNODE *tmp;

  tmp = (APATNODE *)mymalloc(sizeof(APATNODE));
  tmp->left0 = NULL;
  tmp->right1 = NULL;

  return(tmp);
}

/** 
 * Recursive function to search the data in the tree
 * 
 * @param node [in] current node.
 * 
 * @return pointer to the found data
 */
static void *
aptree_search_data_r(APATNODE *node, char *str, int maxbitplace)
{
  if (node->left0 == NULL && node->right1 == NULL) {
    return(node->value.data);
  } else {
    if (testbit_max(str, node->value.thres_bit, maxbitplace) != 0) {
      return(aptree_search_data_r(node->right1, str, maxbitplace));
    } else {
      return(aptree_search_data_r(node->left0, str, maxbitplace));
    }
  }
}

/** 
 * Search for the data whose key string matches the given string.
 * 
 * @param str [in] search key string
 * @param node [in] root node of index tree
 * 
 * @return the exactly found data pointer, or the nearest one.
 */
void *
aptree_search_data(char *str, APATNODE *node)
{
  if (node == NULL) {
    //("Error: aptree_search_data: no node, search for \"%s\" failed\n", str);
    return NULL;
  }
  return(aptree_search_data_r(node, str, strlen(str) * 8 + 8));
}


/*******************************************************************/
/* add 1 node to given ptree */

/** 
 * Make a root node of a index tree.
 * 
 * @param data [in] the first data
 * 
 * @return the newly allocated root node.
 */
APATNODE *
aptree_make_root_node(void *data)
{
  APATNODE *nnew;
  /* make new leaf node for newstr */
  nnew = new_node();
  nnew->value.data = data;
  return(nnew);
}

/** 
 * Insert a new node to the existing index tree.
 * 
 * @param str [in] new key string
 * @param bitloc [in] bit branch to which this node will be added
 * @param data [in] new data pointer
 * @param parentlink [i/o] the parent node to which this node will be added
 */
static void
aptree_add_entry_at(char *str, int bitloc, void *data, APATNODE **parentlink)
{
  APATNODE *node;
  node = *parentlink;
  if (node->value.thres_bit > bitloc ||
      (node->left0 == NULL && node->right1 == NULL)) {
    APATNODE *newleaf, *newbranch;
    /* insert between [parent] and [node] */
    newleaf = new_node();
    newleaf->value.data = data;
    newbranch = new_node();
    newbranch->value.thres_bit = bitloc;
    *parentlink = newbranch;
    if (testbit(str, bitloc) ==0) {
      newbranch->left0  = newleaf;
      newbranch->right1 = node;
    } else {
      newbranch->left0  = node;
      newbranch->right1 = newleaf;
    }
    return;
  } else {
    if (testbit(str, node->value.thres_bit) != 0) {
      aptree_add_entry_at(str, bitloc, data, &(node->right1));
    } else {
      aptree_add_entry_at(str, bitloc, data, &(node->left0));
    }
  }
}

/** 
 * Insert a new node to the index tree.
 * 
 * @param str [in] new key string
 * @param data [in] new data pointer
 * @param matchstr [in] the most matching data already exist in the index tree,
 * as obtained by aptree_search_data()
 * @param rootnode [i/o] pointer to root index node
 */
void
aptree_add_entry(char *str, void *data, char *matchstr, APATNODE **rootnode)
{
  int bitloc;

  bitloc = where_the_bit_differ(str, matchstr);
  if (*rootnode == NULL) {
    *rootnode = aptree_make_root_node(data);
  } else {
    aptree_add_entry_at(str, bitloc, data, rootnode);
  }

}

/*******************************************************************/

/** 
 * Recursive sunction to find and remove an entry.
 * 
 * @param now [in] current node
 * @param up [in] parent node
 * @param up2 [in] parent parent node
 */
static void
aptree_remove_entry_r(APATNODE *now, APATNODE *up, APATNODE *up2, char *str, int maxbitplace, APATNODE **root)
{
  APATNODE *b;

  if (now->left0 == NULL && now->right1 == NULL) {
    /* assume this is exactly the node of data that has specified key string */
    /* make sure the data of your removal request already exists before call this */
    /* execute removal */
    if (up == NULL) {
      free(now);
      *root = NULL;
      return;
    }
    b = (up->right1 == now) ? up->left0 : up->right1;
    if (up2 == NULL) {
      free(now);
      free(up);
      *root = b;
      return;
    }
    if (up2->left0 == up) {
      up2->left0 = b;
    } else {
      up2->right1 = b;
    }
    free(now);
    free(up);
    return;
  } else {
    /* traverse */
    if (testbit_max(str, now->value.thres_bit, maxbitplace) != 0) {
      aptree_remove_entry_r(now->right1, now, up, str, maxbitplace, root);
    } else {
      aptree_remove_entry_r(now->left0, now, up, str, maxbitplace, root);
    }
  }
}
    
/** 
 * Remove a node from the index tree.
 * 
 * @param str [in] existing key string (must exist in the index tree)
 * @param rootnode [i/o] pointer to root index node
 *
 */
void
aptree_remove_entry(char *str, APATNODE **rootnode)
{
  if (*rootnode == NULL) {
    jlog("Warning: aptree: no node, deletion for \"%s\" failed\n", str);
    return;
  }
  aptree_remove_entry_r(*rootnode, NULL, NULL, str, strlen(str)*8+8, rootnode);
}

/*******************************************************************/

/** 
 * Recursive function to traverse index tree and execute
 * the callback for all the existing data.
 * 
 * @param node [in] current node
 * @param callback [in] callback function
 */
void
aptree_traverse_and_do(APATNODE *node, void (*callback)(void *))
{
  if (node->left0 == NULL && node->right1 == NULL) {
    (*callback)(node->value.data);
  } else {
    if (node->left0 != NULL) {
      aptree_traverse_and_do(node->left0, callback);
    }
    if (node->right1 != NULL) {
      aptree_traverse_and_do(node->right1, callback);
    }
  }
}

/** 
 * Free all the sub nodes from specified node.
 * 
 * @param node [in] current node.
 */
void
free_aptree(APATNODE *node)
{
  if (node == NULL) return;
  if (node->left0 != NULL) free_aptree(node->left0);
  if (node->right1 != NULL) free_aptree(node->right1);
  free(node);
}
