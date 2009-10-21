#include	<julius/juliuslib.h>
#include	"Julius.h"

///// コールバック関数
#define JCALLBACK(A, B) \
static void A ( Recog *recog, void *data) \
{\
	cJulius *j = (cJulius *) data;\
	SendMessage(j->getWindow(), WM_JULIUS, B, 0L);\
}

JCALLBACK(callback_engine_active,	JEVENT_ENGINE_ACTIVE);
JCALLBACK(callback_engine_inactive,	JEVENT_ENGINE_INACTIVE);
JCALLBACK(callback_audio_ready,		JEVENT_AUDIO_READY);
JCALLBACK(callback_audio_begin,		JEVENT_AUDIO_BEGIN);
JCALLBACK(callback_audio_end,		JEVENT_AUDIO_END);
JCALLBACK(callback_recog_begin,		JEVENT_RECOG_BEGIN);
JCALLBACK(callback_recog_end,		JEVENT_RECOG_END);
JCALLBACK(callback_recog_frame,		JEVENT_RECOG_FRAME);
JCALLBACK(callback_engine_pause,	JEVENT_ENGINE_PAUSE);
JCALLBACK(callback_engine_resume,	JEVENT_ENGINE_RESUME);

static void callback_result_final(Recog *recog, void *data)
{
	cJulius *j = (cJulius *)data;

	int i;
	WORD_INFO *winfo;
	WORD_ID *seq;
	int seqnum;
	Sentence *s;
	RecogProcess *r;
	static char str[2048];
	static wchar_t wstr[2048];
	size_t size = 0;
	WPARAM wparam = 0;

	r = j->getRecog()->process_list;
	if (! r->live) return;
	if (r->result.status < 0) {      /* no results obtained */
		switch(r->result.status) {
		case J_RESULT_STATUS_REJECT_POWER:
			strcpy(str, "<input rejected by power>");
			break;
		case J_RESULT_STATUS_TERMINATE:
			strcpy(str, "<input teminated by request>");
			break;
		case J_RESULT_STATUS_ONLY_SILENCE:
			strcpy(str, "<input rejected by decoder (silence input result)>");
			break;
		case J_RESULT_STATUS_REJECT_GMM:
			strcpy(str, "<input rejected by GMM>");
			break;
		case J_RESULT_STATUS_REJECT_SHORT:
			strcpy(str, "<input rejected by short input>");
			break;
		case J_RESULT_STATUS_FAIL:
			strcpy(str, "<search failed>");
			break;
		}
		return;
    }

    winfo = r->lm->winfo;
	s = &(r->result.sent[0]);
	seq = s->word;
	seqnum = s->word_num;
	str[0] = '\0';
	for(i=0;i<seqnum;i++) strcat(str, winfo->woutput[seq[i]]);

	mbstowcs_s( &size, wstr, str, strlen(str)+1);

	wparam = (r->result.status << 16) + JEVENT_RESULT_FINAL;

	SendMessage(j->getWindow(), WM_JULIUS, wparam, (LPARAM)wstr);
}

// ポーズ用停止関数

static void callback_wait_for_resume(Recog *recog, void *data)
{
	cJulius *j = (cJulius *)data;
	// このスレッド（認識スレッド）の実行を中断
	SuspendThread( j->getThreadHandle() );
	// 別スレッドからResumeThread()が呼ばれるまで待機
}

//-----------------------------------------------------------------------------------------

#ifdef APP_ADIN
static int callback_adin_fetch_input(SP16 *sampleBuffer, int reqlen)
{
	// 共有バッファに新たな書き込みがあるか、あるいは前回の残りがあるかチェック
	// あればそれを最大 reqlen だけ sampleBuffer に書き込む
}
#endif

//-----------------------------------------------------------------------------------------

// 認識スレッドのメイン関数
DWORD WINAPI recogThreadMain(LPVOID vdParam)
{
	int ret;
	Recog *recog = (Recog *)vdParam;
	ret = j_recognize_stream(recog);
	if (ret == -1) ExitThread(FALSE);
	ExitThread(TRUE);
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------


//================
// コンストラクタ
//================
cJulius::cJulius( void ) : m_jconf( NULL ), m_recog( NULL ), m_opened( false ), m_threadHandle( NULL ), m_fpLogFile( NULL )
{
#ifdef APP_ADIN
	m_appsource = 0;
#endif
	setLogFile( "juliuslog.txt" );
}

//==============
// デストラクタ
//==============
cJulius::~cJulius( void )
{
	release();
	if ( m_fpLogFile ) {
		fclose(m_fpLogFile);
		m_fpLogFile = NULL;
	}
}

//=========
// 初期化1
//=========
bool cJulius::initialize( int argnum, char *argarray[])
{
	bool ret;

	release();

	m_jconf = j_config_load_args_new( argnum, argarray );

	if (m_jconf == NULL) {		/* error */
	    return false;
	}

	ret = createEngine();

	return ret;
}

//=========
// 初期化2
//=========
bool cJulius::initialize( char *filename )
{
	bool ret;

	release();

	if (! loadJconf( filename )) return false;

	ret = createEngine();

	return ret;
}

//=========
// 初期化2
//=========
bool cJulius::loadJconf( char *filename )
{
	if (m_jconf) {
		if (j_config_load_file(m_jconf, filename) == -1) {
			return false;
		}
	} else {
		m_jconf = j_config_load_file_new( filename );
		if (m_jconf == NULL) {		/* error */
		    return false;
		}
	}
	return true;
}

//==================
// ログファイル出力
//==================
void cJulius::setLogFile( const char *filename )
{
	if (m_fpLogFile) {
		fclose( m_fpLogFile );
	}
	m_fpLogFile = fopen( filename, "w" );
	if (m_fpLogFile) {
		jlog_set_output( m_fpLogFile );
	}
}

//================
// エンジン初期化
//================
bool cJulius::createEngine( void )
{
#ifdef APP_ADIN
	ADIn *a;
#endif

	if (!m_jconf) return false;
	if (m_recog) return false;

#ifdef APP_ADIN
	if (m_appsource != 0) {
		// アプリ自身の入力のためにjconf設定を変更
		switch(m_appsource) {
			case 1: // buffer input, batch
				m_recog->jconf->input.type = INPUT_WAVEFORM;
				m_recog->jconf->input.speech_input = SP_RAWFILE;
				m_recog->jconf->decodeopt.realtime_flag = FALSE;
				break;
			case 2: // buffer input, incremental
				m_recog->jconf->input.type = INPUT_WAVEFORM;
				m_recog->jconf->input.speech_input = SP_RAWFILE;
				m_recog->jconf->decodeopt.realtime_flag = TRUE;
				break;
		}
	}
#endif

	// エンジンを起動
	m_recog = j_create_instance_from_jconf(m_jconf);
	if (m_recog == NULL) {
		return false;
	}

	// コールバックを登録
	callback_add(m_recog, CALLBACK_EVENT_PROCESS_ONLINE,		::callback_engine_active, this);
	callback_add(m_recog, CALLBACK_EVENT_PROCESS_OFFLINE,		::callback_engine_inactive, this);
	callback_add(m_recog, CALLBACK_EVENT_SPEECH_READY,			::callback_audio_ready, this);
	callback_add(m_recog, CALLBACK_EVENT_SPEECH_START,			::callback_audio_begin, this);
	callback_add(m_recog, CALLBACK_EVENT_SPEECH_STOP,			::callback_audio_end, this);
	callback_add(m_recog, CALLBACK_EVENT_RECOGNITION_BEGIN,		::callback_recog_begin, this);
	callback_add(m_recog, CALLBACK_EVENT_RECOGNITION_END,		::callback_recog_end, this);
	callback_add(m_recog, CALLBACK_EVENT_PASS1_FRAME,			::callback_recog_frame, this);
	callback_add(m_recog, CALLBACK_EVENT_PAUSE,					::callback_engine_pause, this);
	callback_add(m_recog, CALLBACK_EVENT_RESUME,				::callback_engine_resume, this);
	callback_add(m_recog, CALLBACK_RESULT,						::callback_result_final, this);
	callback_add(m_recog, CALLBACK_PAUSE_FUNCTION,				::callback_wait_for_resume, this);

#ifdef APP_ADIN
	// 音声入力デバイスを初期化
	if (m_appsource != 0) {
		// アプリ入力用に自前で初期化
		a = m_recog->adin;
		switch(m_appsource) {
			case 1: // buffer input, batch
				a->ad_standby			= NULL;
				a->ad_begin				= NULL;
				a->ad_end				= NULL;
				a->ad_resume			= NULL;
				a->ad_pause				= NULL;
				a->ad_terminate			= NULL;
				a->ad_read				= callback_adin_fetch_input;
				a->ad_input_name		= NULL;
				a->silence_cut_default	= FALSE;
				a->enable_thread		= FALSE;
				break;
			case 2: // buffer input, incremental
				a->ad_standby			= NULL;
				a->ad_begin				= NULL;
				a->ad_end				= NULL;
				a->ad_resume			= NULL;
				a->ad_pause				= NULL;
				a->ad_terminate			= NULL;
				a->ad_read				= callback_adin_fetch_input;
				a->ad_input_name		= NULL;
				a->silence_cut_default	= FALSE;
				a->enable_thread		= FALSE;
				break;
		}
	    a->ds = NULL;
		a->down_sample = FALSE;
		if (adin_standby(a, m_recog->jconf->input.sfreq, NULL) == FALSE) return false;
		if (adin_setup_param(a, m_recog->jconf) == FALSE) return false;
		a->input_side_segment = FALSE;
	} else {
		// JuliusLib に初期化を任せる
		if (! j_adin_init( m_recog ) ) return false;
	}
#else
	if (! j_adin_init( m_recog ) ) return false;
#endif

	return true;
}

//====================
// 処理を開始する
//====================
bool cJulius::startProcess( HWND hWnd )
{

	if ( ! m_recog ) return false;

	// メッセージ送信先を保存
	m_hWnd = hWnd;

	if (m_opened == false) {
		// デバイスを開く
		switch(j_open_stream(m_recog, NULL)) {
		case 0:			/* succeeded */
			break;
		case -1:      		/* error */
			// fprintf(stderr, "error in input stream\n");
			return false;
		case -2:			/* end of recognition process */
			//fprintf(stderr, "failed to begin input stream\n");
			return false;
		}
		// スレッドを開く
		m_threadHandle = CreateThread(NULL, 0, ::recogThreadMain, (LPVOID)m_recog, 0, &m_threadId);
		if (m_threadHandle == NULL) {
			j_close_stream(m_recog);
			return false;
		}
		SetThreadPriority(m_threadHandle, THREAD_PRIORITY_HIGHEST);

		m_opened = true;
	}

	return true;
}

//================
// 処理を終了する
//================
void cJulius::stopProcess( void )
{
	if (m_opened) {
		// ストリームを閉じる（スレッドは終了するはず）
		j_close_stream(m_recog);
		m_opened = false;
	}
}

//==========
// 一時停止
//==========
void cJulius::pause( void )
{
	// 停止を要求
	// 止まったら、認識スレッドは pause イベント発効後 PAUSE_FUNCTION コールバックを実行
	j_request_terminate(m_recog);
}

//==========
// 動作再開
//==========
void cJulius::resume( void )
{
	// 再開要求を先にセット
	j_request_resume(m_recog);
	// 認識スレッドの動作を再開。認識スレッドは resume イベント発効後認識処理を再開する
	ResumeThread( m_threadHandle );
}

//==============
// 文法読み込み
//==============
bool cJulius::loadGrammar( WORD_INFO *winfo, DFA_INFO *dfa, char *dictfile, char *dfafile, RecogProcess *r )
{
	boolean ret;

	// load grammar
	switch( r->lmvar ) {
	case LM_DFA_WORD:
		ret = init_wordlist(winfo, dictfile, r->lm->am->hmminfo, 
			r->lm->config->wordrecog_head_silence_model_name,
			r->lm->config->wordrecog_tail_silence_model_name,
			(r->lm->config->wordrecog_silence_context_name[0] == '\0') ? NULL : r->lm->config->wordrecog_silence_context_name,
			r->lm->config->forcedict_flag);
		if (ret == FALSE) {
			return false;
		}
		break;
	case LM_DFA_GRAMMAR:
		ret = init_voca(winfo, dictfile, r->lm->am->hmminfo, FALSE, r->lm->config->forcedict_flag);
		if (ret == FALSE) {
			return false;
		}
	    ret = init_dfa(dfa, dfafile);
		if (ret == FALSE) {
			return false;
		}
		break;
	}

	return true;
}

//==============
// 文法追加
//==============
bool cJulius::addGrammar( char *name, char *dictfile, char *dfafile, bool deleteAll )
{
	WORD_INFO *winfo;
	DFA_INFO *dfa;
	RecogProcess *r = m_recog->process_list;
	boolean ret;

	// load grammar
	switch( r->lmvar ) {
	case LM_DFA_WORD:
		winfo = word_info_new();
		dfa = NULL;
		ret = loadGrammar( winfo, NULL, dictfile, NULL, r );
		if ( ! ret ) {
			word_info_free(winfo);
			return false;
		}
		break;
	case LM_DFA_GRAMMAR:
		winfo = word_info_new();
		dfa = dfa_info_new();
		ret = loadGrammar( winfo, dfa, dictfile, dfafile, r );
		if ( ! ret ) {
			word_info_free(winfo);
			dfa_info_free(dfa);
			return false;
		}
	}
	if ( deleteAll ) {
		/* delete all existing grammars */
		multigram_delete_all(r->lm);
	}
	/* register the new grammar to multi-gram tree */
	multigram_add(dfa, winfo, name, r->lm);
	/* need to rebuild the global lexicon */
	/* tell engine to update at requested timing */
	schedule_grammar_update(m_recog);
	/* make sure this process will be activated */
	r->active = 1;

	SendMessage(getWindow(), WM_JULIUS, JEVENT_GRAM_UPDATE, 0L);

	return true;
}

//==============
// 文法入れ替え
//==============
bool cJulius::changeGrammar( char *name, char *dictfile, char *dfafile )
{
	return addGrammar(name, dictfile, dfafile, true);
}

//==============
// 文法削除
//==============
bool cJulius::deleteGrammar( char *name )
{
	RecogProcess *r = m_recog->process_list;
	int gid;

	gid = multigram_get_id_by_name(r->lm, name);
	if (gid == -1) return false;

	if (multigram_delete(gid, r->lm) == FALSE) { /* deletion marking failed */
		return false;
	}
	/* need to rebuild the global lexicon */
	/* tell engine to update at requested timing */
	schedule_grammar_update(m_recog);

	SendMessage(getWindow(), WM_JULIUS, JEVENT_GRAM_UPDATE, 0L);

	return true;
}

//==============
// 文法一時無効
//==============
bool cJulius::deactivateGrammar( char *name )
{
	RecogProcess *r = m_recog->process_list;
	int gid;
	int ret;

	gid = multigram_get_id_by_name(r->lm, name);
	if (gid == -1) return false;

	ret = multigram_deactivate(gid, r->lm);
	if (ret == 1) {
		/* already active */
		return true;
	} else if (ret == -1) {
		/* not found */
		return false;
	}
	/* tell engine to update at requested timing */
	schedule_grammar_update(m_recog);
	SendMessage(getWindow(), WM_JULIUS, JEVENT_GRAM_UPDATE, 0L);

	return true;
}
//====================
// 文法一時無効を解除
//====================
bool cJulius::activateGrammar( char *name )
{
	RecogProcess *r = m_recog->process_list;
	int gid;
	int ret;

	gid = multigram_get_id_by_name(r->lm, name);
	if (gid == -1) return false;

	ret = multigram_activate(gid, r->lm);
	if (ret == 1) {
		/* already active */
		return true;
	} else if (ret == -1) {
		/* not found */
		return false;
	}
	/* tell engine to update at requested timing */
	schedule_grammar_update(m_recog);
	SendMessage(getWindow(), WM_JULIUS, JEVENT_GRAM_UPDATE, 0L);

	return true;
}

//=========
// 解放
//=========
void cJulius::release( void )
{
	stopProcess();

	if (m_threadHandle) {
		CloseHandle(m_threadHandle);
		m_threadHandle = NULL;
	} else {
		if ( m_recog ) {
			j_recog_free( m_recog );  // jconf will be released inside this
			m_recog = NULL;
			m_jconf = NULL;
		}
		if ( m_jconf ) {
			j_jconf_free( m_jconf );
			m_jconf = NULL;
		}
	}

}