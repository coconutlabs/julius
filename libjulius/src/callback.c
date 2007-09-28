#include <julius/julius.h>

/* callback */

void
callback_init(Recog *recog)
{
  int i;
  for(i=0;i<SIZEOF_CALLBACK_ID;i++) recog->callback_function_num[i] = 0;
  recog->callback_num = 0;
}

static int
callback_add_core(Recog *recog, int code, void (*func)(), void *data)
{
  int i;
  int num;
  int newid;

  num = recog->callback_function_num[code];

  if (num >= MAX_CALLBACK_HOOK) {
    jlog("ERROR: failed to add callback for slot %d\n", code);
    jlog("ERROR: maximum number of callback for a slot is limited to %d\n", MAX_CALLBACK_HOOK);
    jlog("ERROR: please increase the value of MAX_CALLBACK_HOOK\n");
    return -1;
  }

  for(i=0;i<num;i++) {
    if (recog->callback_function[code][i] == func && recog->callback_user_data[code][i] == data) {
      jlog("WARNING: the same callback already registered at slot %d\n", code);
      return -1;
    }
  }
  recog->callback_function[code][num] = func;
  recog->callback_user_data[code][num] = data;
  recog->callback_function_num[code]++;

  newid = recog->callback_num;
  if (newid >= MAX_CALLBACK_HOOK * SIZEOF_CALLBACK_ID) {
    jlog("ERROR: callback registration count reached maximum limit (%d)!\n", MAX_CALLBACK_HOOK * SIZEOF_CALLBACK_ID);
    return -1;
  }
  recog->callback_list_code[newid] = code;
  recog->callback_list_loc[newid] = num;
  recog->callback_num++;

  return newid;
}

int
callback_add(Recog *recog, int code, void (*func)(Recog *recog, void *data), void *data)
{
  return(callback_add_core(recog, code, func, data));
}

int
callback_add_adin(Recog *recog, int code, void (*func)(Recog *recog, SP16 *buf, int len, void *data), void *data)
{
  return(callback_add_core(recog, code, func, data));
}

void
callback_exec(int code, Recog *recog)
{
  int i;

  if (code < 0 || code >= SIZEOF_CALLBACK_ID) {
    jlog("ERROR: failed to exec callback: invalid code number: %d\n", code);
    return;
  }
  for(i=0;i<recog->callback_function_num[code];i++) {
    (*(recog->callback_function[code][i]))(recog, recog->callback_user_data[code][i]);
  }
}

void
callback_exec_adin(int code, Recog *recog, SP16 *buf, int len) 
{
  int i;

  if (code < 0 || code >= SIZEOF_CALLBACK_ID) {
    jlog("ERROR: failed to exec callback: invalid code number: %d\n", code);
    return;
  }
  for(i=0;i<recog->callback_function_num[code];i++) {
    (*(recog->callback_function[code][i]))(recog, buf, len, recog->callback_user_data[code][i]);
  }
}
  
boolean
callback_exist(Recog *recog, int code)
{
  if (recog->callback_function_num[code] == 0) return FALSE;
  return TRUE;
}

boolean
callback_delete(Recog *recog, int id)
{
  int code;
  int loc;
  int i;

  if (id >= recog->callback_num || id < 0) {
    jlog("ERROR: callback id #%d not exist!\n", id);
    return FALSE;
  }

  code = recog->callback_list_code[id];
  loc  = recog->callback_list_loc[id];

  if (code == -1) {
    jlog("WARNING: callback #%d already deleted\n", id);
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

void
callback_multi_exec(int code, Recog **recoglist, int num)
{
  int i;
  for(i=0;i<num;i++) callback_exec(code, recoglist[i]);
}
