/**
 * @file   callback.c
 * 
 * <EN>
 * @brief  Regist and execute callback functions.
 *
 * This file contains functions for handling callback functions.
 * User should use callback_add() (and callback_add_adin() for A/D-in
 * related callbacks) to regist user function to the callback repository.
 * Then, Julius will call the registered functions at apropriate timimg
 * while search.
 *
 * More than one function can be assigned to a callback,
 * in which case all functions will be called in turn.
 * </EN>
 * 
 * <JA>
 * @brief  ������Хå��ؿ�����Ͽ�ȼ¹�
 *
 * ���Υե�����ˤϥ�����Хå��򰷤��ؿ����ޤޤ�Ƥ��ޤ�. 
 * �桼���� callback_add() (A/D-in ��Ϣ�Υ�����Хå��Ǥ� allback_add_adin())
 * ��Ȥäơ��桼�������������ؿ��򡤻���Υ�����Хå���ݥ��ȥ����Ͽ���ޤ�. 
 * ǧ������Julius ����Ͽ���줿�ؿ���ǧ�������γƾ��̤ǸƤӽФ��ޤ�. 
 *
 * ���륳����Хå��ˤĤ���ʣ���δؿ�����Ͽ���뤳�Ȥ��Ǥ��ޤ�. ���ξ�
 * �硤������Хå��ƽФ��ϡ�Ʊ��������Хå�����Ͽ���줿ʣ���δؿ���
 * ���ƸƤФ�ޤ�. 
 * </JA>
 * 
 * @author Akinobu Lee
 * @date   Fri Oct 26 00:03:18 2007
 *
 * $Revision: 1.2 $
 * 
 */
#include <julius/julius.h>

/** 
 * <EN>
 * Initialize callback management area.
 * </EN>
 * <JA>
 * ������Хå��������ꥢ�ν����
 * </JA>
 * 
 * @param recog [i/o] engine instance
 *
 * @callergraph
 * @callgraph
 * 
 */
void
callback_init(Recog *recog)
{
  int i;
  for(i=0;i<SIZEOF_CALLBACK_ID;i++) recog->callback_function_num[i] = 0;
  recog->callback_num = 0;
}

/** 
 * <EN>
 * Core function to register a function to a callback registory.
 * </EN>
 * <JA>
 * �ؿ��򥳡���Хå��쥸���ȥ����Ͽ���륳���ؿ�
 * </JA>
 * 
 * @param recog [i/o] engine instance
 * @param code [in] code in which the function will be registered
 * @param func [in] function
 * @param data [in] user-specified argument to be passed when the function is called inside Julius
 * 
 * @return global callback ID unique for the whole process, or -1 on error.
 * 
 */
static int
callback_add_core(Recog *recog, int code, void (*func)(), void *data)
{
  int i;
  int num;
  int newid;

  num = recog->callback_function_num[code];

  if (num >= MAX_CALLBACK_HOOK) {
    jlog("ERROR: callback_add: failed to add callback for slot %d\n", code);
    jlog("ERROR: callback_add: maximum number of callback for a slot is limited to %d\n", MAX_CALLBACK_HOOK);
    jlog("ERROR: callback_add: please increase the value of MAX_CALLBACK_HOOK\n");
    return -1;
  }

  for(i=0;i<num;i++) {
    if (recog->callback_function[code][i] == func && recog->callback_user_data[code][i] == data) {
      jlog("WARNING: callback_add: the same callback already registered at slot %d\n", code);
      return -1;
    }
  }
  recog->callback_function[code][num] = func;
  recog->callback_user_data[code][num] = data;
  recog->callback_function_num[code]++;

  newid = recog->callback_num;
  if (newid >= MAX_CALLBACK_HOOK * SIZEOF_CALLBACK_ID) {
    jlog("ERROR: callback_add: callback registration count reached maximum limit (%d)!\n", MAX_CALLBACK_HOOK * SIZEOF_CALLBACK_ID);
    return -1;
  }
  recog->callback_list_code[newid] = code;
  recog->callback_list_loc[newid] = num;
  recog->callback_num++;

  return newid;
}

/** 
 * <EN>
 * Register a function to a callback registory.
 * </EN>
 * <JA>
 * �ؿ��򥳡���Хå��쥸���ȥ����Ͽ����. 
 * </JA>
 * 
 * @param recog [i/o] engine instance
 * @param code [in] code in which the function will be registered
 * @param func [in] function
 * @param data [in] user-specified argument to be passed when the function is called inside Julius
 * 
 * @return global callback ID unique for the whole process, or -1 on error.
 *
 * @ingroup callback
 * @callergraph
 * @callgraph
 * 
 */
int
callback_add(Recog *recog, int code, void (*func)(Recog *recog, void *data), void *data)
{
  return(callback_add_core(recog, code, func, data));
}

/** 
 * <EN>
 * Register a function to the A/D-in type callback registory.
 * </EN>
 * <JA>
 * �ؿ���A/D-in�����פΥ�����Хå��쥸���ȥ����Ͽ����. 
 * </JA>
 * 
 * @param recog [i/o] engine instance
 * @param code [in] code in which the function will be registered
 * @param func [in] function
 * @param data [in] user-specified argument to be passed when the function is called inside Julius
 * 
 * @return global callback ID unique for the whole process, or -1 on error.
 *
 * @ingroup callback
 * @callergraph
 * @callgraph
 * 
 */
int
callback_add_adin(Recog *recog, int code, void (*func)(Recog *recog, SP16 *buf, int len, void *data), void *data)
{
  return(callback_add_core(recog, code, func, data));
}


/** 
 * <EN>
 * Execute all functions assigned to a callback registory.
 * </EN>
 * <JA>
 * ������Хå��쥸���ȥ����Ͽ����Ƥ���ؿ������Ƽ¹Ԥ���. 
 * </JA>
 * 
 * @param code [in] callback code
 * @param recog [in] engine instance.
 *
 * @callergraph
 * @callgraph
 * 
 */
void
callback_exec(int code, Recog *recog)
{
  int i;

  if (code < 0 || code >= SIZEOF_CALLBACK_ID) {
    jlog("ERROR: callback_exec: failed to exec callback: invalid code number: %d\n", code);
    return;
  }
  for(i=0;i<recog->callback_function_num[code];i++) {
    (*(recog->callback_function[code][i]))(recog, recog->callback_user_data[code][i]);
  }
}

/** 
 * <EN>
 * Execute all functions assigned to a A/D-in callback.
 * </EN>
 * <JA>
 * A/D-in �����פΥ�����Хå�����Ͽ���줿�ؿ������Ƽ¹Ԥ���. 
 * </JA>
 * 
 * @param code [in] callbcak code
 * @param recog [in] engine instance
 * @param buf [in] buffer that holds the current input speech which will be passed to the functions
 * @param len [in] length of @a buf
 *
 * @callergraph
 * @callgraph
 * 
 */
void
callback_exec_adin(int code, Recog *recog, SP16 *buf, int len) 
{
  int i;

  if (code < 0 || code >= SIZEOF_CALLBACK_ID) {
    jlog("ERROR: callback_exec_adin: failed to exec callback: invalid code number: %d\n", code);
    return;
  }
  for(i=0;i<recog->callback_function_num[code];i++) {
    (*(recog->callback_function[code][i]))(recog, buf, len, recog->callback_user_data[code][i]);
  }
}

/** 
 * <EN>
 * Check if at least one function has been registered to a callback repository.
 * </EN>
 * <JA>
 * ������Хå��쥸���ȥ�ˣ��ĤǤ�ؿ�����Ͽ���줿���ɤ������֤�. 
 * </JA>
 * 
 * @param recog [in] engine instance
 * @param code [in] callback code
 * 
 * @return TRUE when at least one is registered, or FALSE if none.
 *
 * @ingroup callback
 * @callergraph
 * @callgraph
 * 
 */
boolean
callback_exist(Recog *recog, int code)
{
  if (recog->callback_function_num[code] == 0) return FALSE;
  return TRUE;
}

/** 
 * <EN>
 * Delete an already registered function from callback.
 * </EN>
 * <JA>
 * ������Хå�����ؿ���������. 
 * </JA>
 * 
 * @param recog [i/o] engine instance
 * @param id [in] global callback ID to delete
 * 
 * @return TRUE on success, or FALSE on failure.
 * 
 * @ingroup callback
 * @callergraph
 * @callgraph
 * 
 */
boolean
callback_delete(Recog *recog, int id)
{
  int code;
  int loc;
  int i;

  if (id >= recog->callback_num || id < 0) {
    jlog("ERROR: callback_delete: callback id #%d not exist!\n", id);
    return FALSE;
  }

  code = recog->callback_list_code[id];
  loc  = recog->callback_list_loc[id];

  if (code == -1) {
    jlog("WARNING: callback_delete: callback #%d already deleted\n", id);
    return TRUE;
  }

  for(i=loc;i<recog->callback_function_num[code]-1;i++) {
    recog->callback_function[code][i] = recog->callback_function[code][i+1];
    recog->callback_function[code][i] = recog->callback_function[code][i+1];
    recog->callback_user_data[code][i] = recog->callback_user_data[code][i+1];
  }
  recog->callback_function_num[code]--;
  recog->callback_list_code[id] = -1;
  recog->callback_list_loc[id] = -1;

  jlog("STAT: callback #%d deleted\n", id);
  return TRUE;
}

/* end of file */
