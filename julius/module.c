#include "app.h"

#include <stdarg.h>

#define DEFAULT_MODULEPORT 10500

static int module_mode = FALSE;
int module_sd = -1;

#define MAXBUFLEN 4096 ///< Maximum line length of a message sent from a client
static char mbuf[MAXBUFLEN];	///< Work buffer for message output
static char buf[MAXBUFLEN];	///< Work buffer for exec
static char inbuf[MAXBUFLEN];
#ifdef CHARACTER_CONVERSION
static char outbuf[MAXBUFLEN];
#endif

/** 
 * Generic function to send a formatted message to client module.
 *
 * @param sd [in] socket descriptor
 * @param fmt [in] format string, like printf.
 * @param ... [in] variable length argument like printf.
 * 
 * @return the same as printf, i.e. number of characters printed.
 */
int
module_send(int sd, char *fmt, ...)
{
  va_list ap;
  int ret;
  char *buf;
  
  va_start(ap,fmt);
  ret = vsnprintf(inbuf, MAXBUFLEN, fmt, ap);
  va_end(ap);
  if (ret > 0) {		/* success */
    
#ifdef CHARACTER_CONVERSION
    buf = charconv(inbuf, outbuf, MAXBUFLEN);
#else
    buf = inbuf;
#endif
    if (
#ifdef WINSOCK
	send(sd, buf, strlen(buf), 0)
#else
	write(sd, buf, strlen(buf))
#endif
	< 0) {
      perror("Error: module_send:");
    }
  }
  return(ret);
}

/** 
 * <JA>
 * @brief  モジュールコマンドを処理する．
 *
 * クライアントより与えられたコマンドを処理する．この関数はクライアントから
 * コマンドが送られてくるたびに音声認識処理に割り込んで呼ばれる．
 * ステータス等についてはここですぐに応答メッセージを送るが，
 * 文法の追加や削除などは，ここでは受信のみ行い，実際の変更処理
 * （各文法からのグローバル文法の再構築など）は認識処理の合間に実行される．
 * この文法再構築処理を実際に行うのは multigram_exec() である．
 * 
 * @param command [in] コマンド文字列
 * </JA>
 * <EN>
 * @brief  Process a module command.
 *
 * This function processes command string received from module client.
 * This will be called whenever a command arrives from a client, interrupting
 * the main recognition process.  The status responses will be performed
 * at this function immediately.  On the whole, grammar modification
 * (add/delete/(de)activation) will not be performed here.  The received
 * data are just stored in this function, and they will be processed later
 * by calling multigram_exec() between the recognition process.
 * 
 * @param command [in] command string
 * </EN>
 */
static void
msock_exec_command(char *command, Recog *recog)
{
  DFA_INFO *new_dfa;
  WORD_INFO *new_winfo;
  static char *p, *q;
  int gid;
  char namebuf[MAXGRAMNAMELEN];
  int ret;

  /* prompt the received command string */
  printf("[[%s]]\n",command);

  if (strmatch(command, "STATUS")) {
    /* return status */
    if (recog->process_active) {
      module_send(module_sd, "<SYSINFO PROCESS=\"ACTIVE\"/>\n.\n");
    } else {
      module_send(module_sd, "<SYSINFO PROCESS=\"SLEEP\"/>\n.\n");
    }
  } else if (strmatch(command, "DIE")) {
    /* disconnect */
    close_socket(module_sd);
    module_sd = -1;
#if defined(_WIN32) && !defined(__CYGWIN32__)
    /* this is single process and has not forked, so
       we just disconnect the connection here.  */
#else
    /* this is a forked process, so exit here. */

#endif
  } else if (strmatch(command, "VERSION")) {
    /* return version */
    module_send(module_sd, "<ENGINEINFO TYPE=\"%s\" VERSION=\"%s\" CONF=\"%s\"/>\n.\n",
		JULIUS_PRODUCTNAME, JULIUS_VERSION, JULIUS_SETUP);
  } else if (strmatch(command, "PAUSE")) {
    /* pause recognition: will stop when the current input ends */
    j_request_pause(recog);
  } else if (strmatch(command, "TERMINATE")) {
    j_request_terminate(recog);
  } else if (strmatch(command, "RESUME")) {
    j_request_resume(recog);
  } else if (strmatch(command, "INPUTONCHANGE")) {
    /* change grammar switching timing policy */
    if (
#ifdef WINSOCK
	getl_sd(buf, MAXBUFLEN, module_sd)
#else	
	getl_fd(buf, MAXBUFLEN, module_sd)
#endif
	== NULL) {
      fprintf(stderr, "Error: msock(INPUTONCHANGE): no argument\n"); exit(-1);
    }
    if (recog->lmtype == LM_DFA) {
      if (strmatch(buf, "TERMINATE")) {
	recog->gram_switch_input_method = SM_TERMINATE;
      } else if (strmatch(buf, "PAUSE")) {
	recog->gram_switch_input_method = SM_PAUSE;
      } else if (strmatch(buf, "WAIT")) {
	recog->gram_switch_input_method = SM_WAIT;
      } else {
	fprintf(stderr, "Error: msock(INPUTONCHANGE): unknown method [%s]\n", buf); exit(-1);
      }
    } else {
      module_send(module_sd, "<GRAMMAR STATUS=\"ERROR\" REASON=\"INVALID COMMAND\"/>\n.\n");
    }
  } else if (strnmatch(command, "CHANGEGRAM", strlen("CHANGEGRAM"))) {
    /* receive grammar (DFA + DICT) from the socket, and swap the whole grammar  */
    /* read grammar name if any */
    p = &(command[strlen("CHANGEGRAM")]);
    while (*p == ' ' && *p != '\r' && *p != '\n' && *p != '\0') p++;
    if (*p != '\r' && *p != '\n' && *p != '\0') {
      q = buf;
      while (*p != ' ' && *p != '\r' && *p != '\n' && *p != '\0') *q++ = *p++;
      *q = '\0';
      p = buf;
    } else {
      p = NULL;
    }
    /* read a new grammar via socket */
    if (read_grammar_from_socket(module_sd, &new_dfa, &new_winfo, recog->model->hmminfo) == FALSE) {
      module_send(module_sd, "<GRAMMAR STATUS=\"ERROR\" REASON=\"WRONG DATA\"/>\n.\n");
    } else {
      if (recog->lmtype == LM_DFA) {
	/* delete all existing grammars */
	multigram_delete_all(recog);
	/* register the new grammar to multi-gram tree */
	multigram_add(new_dfa, new_winfo, p, recog->model);
	/* need to rebuild the global lexicon */
	/* tell engine to update at requested timing */
	schedule_grammar_update(recog);
	/* tell module client  */
	module_send(module_sd, "<GRAMMAR STATUS=\"RECEIVED\"/>\n.\n");
	send_gram_info(recog);
      } else {
	module_send(module_sd, "<GRAMMAR STATUS=\"ERROR\" REASON=\"INVALID COMMAND\"/>\n.\n");
      }
    }
  } else if (strnmatch(command, "ADDGRAM", strlen("ADDGRAM"))) {
    /* receive grammar and add it to the current grammars */
    /* read grammar name if any */
    p = &(command[strlen("ADDGRAM")]);
    while (*p == ' ' && *p != '\r' && *p != '\n' && *p != '\0') p++;
    if (*p != '\r' && *p != '\n' && *p != '\0') {
      q = buf;
      while (*p != ' ' && *p != '\r' && *p != '\n' && *p != '\0') *q++ = *p++;
      *q = '\0';
      p = buf;
    } else {
      p = NULL;
    }
    /* read a new grammar via socket */
    if (read_grammar_from_socket(module_sd, &new_dfa, &new_winfo, recog->model->hmminfo) == FALSE) {
      module_send(module_sd, "<GRAMMAR STATUS=\"ERROR\" REASON=\"WRONG DATA\"/>\n.\n");
    } else {
      if (recog->lmtype == LM_DFA) {
	/* add it to multi-gram tree */
	multigram_add(new_dfa, new_winfo, p, recog->model);
	/* need to rebuild the global lexicon */
	/* tell engine to update at requested timing */
	schedule_grammar_update(recog);
	/* tell module client  */
	module_send(module_sd, "<GRAMMAR STATUS=\"RECEIVED\"/>\n.\n");
	send_gram_info(recog);
      } else {
	module_send(module_sd, "<GRAMMAR STATUS=\"ERROR\" REASON=\"INVALID COMMAND\"/>\n.\n");
      }
    }
  } else if (strmatch(command, "DELGRAM")) {
    /* remove the grammar specified by ID */
    /* read a list of grammar IDs to be deleted */
    if (
#ifdef WINSOCK
	getl_sd(buf, MAXBUFLEN, module_sd)
#else
	getl_fd(buf, MAXBUFLEN, module_sd)
#endif
	== NULL) {
      fprintf(stderr, "Error: msock(DELGRAM): no argument\n"); exit(-1);
    }
    /* extract IDs and mark them as delete
       (actual deletion will be performed on the next 
    */
    if (recog->lmtype == LM_DFA) {
      for(p=strtok(buf," ");p;p=strtok(NULL," ")) {
	gid = atoi(p);
	if (multigram_delete(gid, recog) == FALSE) { /* deletion marking failed */
	  fprintf(stderr, "Warning: msock(DELGRAM): gram #%d failed to delete, ignored\n", gid);
	  /* tell module */
	  module_send(module_sd, "<ERROR MESSAGE=\"Gram #%d not found\"/>\n.\n", gid);
	}
      }
      /* need to rebuild the global lexicon */
      /* tell engine to update at requested timing */
      schedule_grammar_update(recog);
    } else {
      module_send(module_sd, "<GRAMMAR STATUS=\"ERROR\" REASON=\"INVALID COMMAND\"/>\n.\n");
    }
  } else if (strmatch(command, "ACTIVATEGRAM")) {
    /* activate grammar in this engine */
    /* read a list of grammar IDs to be activated */
    if (
#ifdef WINSOCK
	getl_sd(buf, MAXBUFLEN, module_sd)
#else
	getl_fd(buf, MAXBUFLEN, module_sd)
#endif
	== NULL) {
      fprintf(stderr, "Error: msock(ACTIVATEGRAM): no argument\n"); exit(-1);
    }
    /* mark them as active */
    if (recog->lmtype == LM_DFA) {
      for(p=strtok(buf," ");p;p=strtok(NULL," ")) {
	gid = atoi(p);
	ret = multigram_activate(gid, recog);
	if (ret == 1) {
	  /* already active */
	  module_send(module_sd, "<WARN MESSAGE=\"Gram #%d already active\"/>\n.\n", gid);
	} else if (ret == -1) {
	  /* not found */
	  module_send(module_sd, "<WARN MESSAGE=\"Gram #%d not found\"/>\n.\n", gid);
	}	/* else success */
      }
      /* tell engine to update at requested timing */
      schedule_grammar_update(recog);
    } else {
      module_send(module_sd, "<GRAMMAR STATUS=\"ERROR\" REASON=\"INVALID COMMAND\"/>\n.\n");
    }
  } else if (strmatch(command, "DEACTIVATEGRAM")) {
    /* deactivate grammar in this engine */
    /* read a list of grammar IDs to be de-activated */
    if (
#ifdef WINSOCK
	getl_sd(buf, MAXBUFLEN, module_sd)
#else
	getl_fd(buf, MAXBUFLEN, module_sd)
#endif
	== NULL) {
      fprintf(stderr, "Error: msock(DEACTIVATEGRAM): no argument\n"); exit(-1);
    }
    if (recog->lmtype == LM_DFA) {
      /* mark them as not active */
      for(p=strtok(buf," ");p;p=strtok(NULL," ")) {
	gid = atoi(p);
	ret = multigram_deactivate(gid, recog);
	if (ret == 1) {
	  /* already inactive */
	  module_send(module_sd, "<WARN MESSAGE=\"Gram #%d already inactive\"/>\n.\n", gid);
	} else if (ret == -1) {
	  /* not found */
	  module_send(module_sd, "<WARN MESSAGE=\"Gram #%d not found\"/>\n.\n", gid);
	}	/* else success */
      }
      schedule_grammar_update(recog);
    } else {
      module_send(module_sd, "<GRAMMAR STATUS=\"ERROR\" REASON=\"INVALID COMMAND\"/>\n.\n");
    }
  } else if (strmatch(command, "SYNCGRAM")) {
    /* update grammar if necessary */
    if (recog->lmtype == LM_DFA) {
      multigram_exec(recog);  /* some modification occured if return TRUE */
      module_send(module_sd, "<GRAMMAR STATUS=\"READY\"/>\n.\n");
    } else {
      module_send(module_sd, "<GRAMMAR STATUS=\"ERROR\" REASON=\"INVALID COMMAND\"/>\n.\n");
    }
  }
}

/** 
 * <JA>
 * 現在クライアントモジュールからの命令がバッファにあるか調べ，
 * もしあれば処理する．なければそのまま終了する．
 * 
 * </JA>
 * <EN>
 * Process one commands from client module.  If no command is in the buffer,
 * it will return without blocking.
 * 
 * </EN>
 */
static void
msock_check_and_process_command(Recog *recog, void *dummy)
{
  fd_set rfds;
  int ret;
  struct timeval tv;

  /* check if some commands are waiting in queue */
  FD_ZERO(&rfds);
  FD_SET(module_sd, &rfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;	      /* 0 msec timeout: return immediately */
  ret = select(module_sd+1, &rfds, NULL, NULL, &tv);
  if (ret < 0) {
    perror("msock_check_and_process_command: cannot poll\n");
  }
  if (ret > 0) {
    /* there is data to read */
    /* process command and change status if necessaty */
    while(select(module_sd+1, &rfds, NULL, NULL, &tv) > 0 &&
#ifdef WINSOCK
	  getl_sd(mbuf, MAXBUFLEN, module_sd)
#else
	  getl_fd(mbuf, MAXBUFLEN, module_sd)
#endif
	  != NULL) {
      msock_exec_command(mbuf, recog);
    }
  }
}

/** 
 * <JA>
 * クライアントモジュールからの命令を読み込んで処理する．
 * 命令が無い場合，次のコマンドが来るまで待つ．
 * msock_exec_command() 内で j_request_resume() が呼ばれて
 * recog->process_active が TRUE になるまで繰り返す．
 * この関数が終わったときプロセスは resume |!する．|
 * 
 * </JA>
 * <EN>
 * Process one commands from client module.  If no command is in the buffer,
 * it will block until next command comes.
 * 
 * </EN>
 */
static void
msock_process_command(Recog *recog, void *dummy)
{

  while(!recog->process_active) {
    if (
#ifdef WINSOCK
	getl_sd(mbuf, MAXBUFLEN, module_sd)
#else
	getl_fd(mbuf, MAXBUFLEN, module_sd)
#endif
	!= NULL) {
      msock_exec_command(mbuf, recog);
    }
  }
}

static void
module_regist_callback(Recog *recog, void *data)
{
  callback_add(recog, CALLBACK_POLL, msock_check_and_process_command, data);
  callback_add(recog, CALLBACK_PAUSE_FUNCTION, msock_process_command, data);
}

/************************************************************************/
static boolean
opt_module(Jconf *jconf, char *arg[], int argnum)
{
  module_mode = TRUE;
  return TRUE;
}

void
module_add_option()
{
  j_add_option("-module", 0, "run as a server module", opt_module);
}

boolean
is_module_mode()
{
  return module_mode;
}

void
module_setup(Recog *recog, void *data)
{
  /* register result output callback functions */
  module_regist_callback(recog, data);
  setup_output_msock(recog, data);
  //decode_output_selection("WLPSwlpsC");
  decode_output_selection("WSC");
}
  
void
module_recognition_stream_loop(Recog **recoglist, int recognum)
{
  int listen_sd;	///< Socket to listen to a client
  pid_t cid;
  int module_port;
  
  module_port = DEFAULT_MODULEPORT;
  
  /* prepare socket to listen */
  if ((listen_sd = ready_as_server(module_port)) < 0) {
    fprintf(stderr, "Error: failed to bind socket\n");
    return;
  }
  
  printf  ("///////////////////////////////\n");
  printf  ("///  Module mode ready\n");
  printf  ("///  waiting client at %5d\n", module_port);
  printf  ("///////////////////////////////\n");
  printf  ("///  ");
  
  /* no fork, just wait for one connection and proceed */
  if ((module_sd = accept_from(listen_sd)) < 0) {
    fprintf(stderr, "Error: failed to accept connection\n");
    return;
  }

  /* call main recognition loop here */
  main_recognition_stream_loop(recoglist, recognum);
  
  /* disconnect control module */
  if (module_sd >= 0) { /* connected now */
    module_send(module_sd, "<SYSINFO PROCESS=\"ERREXIT\"/>\n.\n");
    close_socket(module_sd);
    module_sd = -1;
  }
}

  
