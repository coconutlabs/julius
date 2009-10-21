//
// �EJulius �������F���[�h���Ĉȉ��̊֐����R�[���o�b�N�Ɏd����
// �@�F���J�n
// �@�g���K���m
//�@�@���ʎ擾
//    �e�֐��͑Ή����郁�b�Z�[�W���E�B���h�E�Y�C�x���g�� issue ����
// �E�����F���J�n
// �E�����F�����f�E�ĊJ
// �E�����F���I��

#ifndef		_JULIUSCLASS_H_
#define		_JULIUSCLASS_H_

#include	"julius/juliuslib.h"

// �J���p��`
#undef APP_ADIN

// ���[�U��`�C�x���g
#define WM_JULIUS			(WM_USER + 1)

// �C�x���gID
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
