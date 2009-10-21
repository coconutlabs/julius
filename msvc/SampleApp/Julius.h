//
// ・Julius 初期化：ロードして以下の関数をコールバックに仕込む
// 　認識開始
// 　トリガ検知
//　　結果取得
//    各関数は対応するメッセージをウィンドウズイベントへ issue する
// ・音声認識開始
// ・音声認識中断・再開
// ・音声認識終了

#ifndef		_JULIUSCLASS_H_
#define		_JULIUSCLASS_H_

#include	"julius/juliuslib.h"

// 開発用定義
#undef APP_ADIN

// ユーザ定義イベント
#define WM_JULIUS			(WM_USER + 1)

// イベントID
enum {
	JEVENT_ENGINE_ACTIVE,
	JEVENT_ENGINE_INACTIVE,
	JEVENT_ENGINE_PAUSE,
	JEVENT_ENGINE_RESUME,
	JEVENT_AUDIO_READY,
	JEVENT_AUDIO_BEGIN,
	JEVENT_AUDIO_END,
	JEVENT_RECOG_BEGIN,
	JEVENT_RECOG_END,
	JEVENT_RECOG_FRAME,
	JEVENT_RESULT_FRAME,
	JEVENT_RESULT_PASS1,
	JEVENT_RESULT_FINAL,
	JEVENT_GRAM_UPDATE,
};


class cJulius
{
	private :
		Jconf			*m_jconf;			// Configuration parameter data
		Recog			*m_recog;			// Engine instance
		bool			m_opened;
		FILE			*m_fpLogFile;
		HANDLE			m_threadHandle;
		DWORD			m_threadId;
		HWND			m_hWnd;
#ifdef APP_ADIN
		int				m_appsource
#endif

	public :
		cJulius( void );
		~cJulius( void );

		void setLogFile( const char *filename );

		bool initialize( int argnum, char *argarray[] );
		bool initialize( char *jconffile );
		bool loadJconf( char *jconffile );
		bool createEngine( void );

		bool startProcess( HWND hWnd );
		void stopProcess( void );

		void pause( void );
		void resume( void );

		bool loadGrammar( WORD_INFO *winfo, DFA_INFO *dfa, char *dictfile, char *dfafile, RecogProcess *r );
		bool addGrammar( char *name, char *dictfile, char *dfafile, bool deleteAll = false );
		bool changeGrammar( char *name, char *dictfile, char *dfafile );
		bool deleteGrammar( char *name );
		bool deactivateGrammar( char *name );
		bool activateGrammar( char *name );

		inline HWND getWindow( void ) { return m_hWnd; };
		inline Recog *getRecog( void ) { return m_recog; };
		inline HANDLE getThreadHandle( void ) { return m_threadHandle; };

		void release( void );
};

#endif
