======================================================================
                  Large Vocabulary Continuous Speech
                          Recognition Engine

                                Julius

                                                (Rev 4.1   2008/10/03)
                                                (Rev 4.0.2 2008/05/27)
                                                (Rev 4.0   2007/12/19)
                                                (Rev 3.5.3 2006/12/29)
                                                (Rev 3.4.2 2004/04/30)
                                                (Rev 2.0   1999/02/20)
                                                (Rev 1.0   1998/02/20)

 Copyright (c) 1991-2008 ���s��w �͌�������
 Copyright (c) 1997-2000 ��񏈗��U�����Ƌ���(IPA)
 Copyright (c) 2000-2005 �ޗǐ�[�Ȋw�Z�p��w�@��w ���쌤����
 Copyright (c) 2005-2008 ���É��H�Ƒ�w Julius�J���`�[��
 All rights reserved
======================================================================

Julius �ɂ���
=================

Julius �́C�����F���V�X�e���̊J���E�����̂��߂̃I�[�v���\�[�X�̍����\
�Ȕėp���b�A�������F���G���W���ł��D������b�̘A�������F������ʂ�PC
��łقڎ����ԂŎ��s�ł��܂��D�܂��C�����ėp���������C���������⌾�ꃂ
�f���E�������f���Ȃǂ̉����F���̊e���W���[����g�ݑւ��邱�ƂŁC�l�X��
���L���p�r�ɉ��p�ł��܂��D ����v���b�g�t�H�[���� Linux, Windows�C
���̑� Unix ���ł��D�ڍׁE�֘A���͈ȉ��� URL �������������D

    http://julius.sourceforge.jp/


Julius-4.1
===========

4.0 ���� 4.0.2 �ł́C�����̃o�O���C������C�������ׂ̍������P��
�s���܂����D�V�I�v�V�����Ƃ��� "-fallback1pass" �� "-usepower" ��
�ǉ�����CLinux �ł̓I�[�f�B�I API �̃f�t�H���g�� OSS ���� ALSA ��
�ύX�ɂȂ�܂����D

4.0.2 ���� 4.1 �ł́C�}���`�X�g���[���������f���CMSD-HMM�������f���C
CVN ����� frequency warping for VTLN ���T�|�[�g����܂����D�܂��C
���W���[�����[�h�̃N���C�A���g�� perl �ŃT���v�� "jclient-perl" ��
�ǉ�����܂����D

4.1 �ŉ�����ꂽ�ł��傫�ȋ@�\�́C���I�����N��p�����v���O�C���g���̃T
�|�[�g�ł��D���L�I�u�W�F�N�g���v���O�C���Ƃ��Ď��s���ɓǂݍ��ނ��Ƃ���
���C�G���W���̃\�[�X��������Ȃ��Ă��ȒP�ɋ@�\�g�����邱�Ƃ��ł��܂��D
�f�B���N�g�� "plugin" �ȉ��ɁC�T���v���̃\�[�X�R�[�h���܂܂�Ă���C�e
�X�g�ł���悤�ɂȂ��Ă��܂��D�R�[�h�ɂ͒�`���ׂ��֐��̎d�l���L�q����
�Ă��܂��̂ŁC�Q�l�ɂ��Ă��������D

�܂��CJulius �Ɋ֘A����h�L�������g�� "The Juliusbook" �Ƃ��ē����Ɍ�
�J�����悤�ɂȂ�܂����D�h�L�������g�̃\�[�X�� Docbook XML �ŏ�����Ă�
��Chtml �ł� pdf �ł��񋟂���Ă��܂��D�܂��A�[�J�C�u���̃I�����C���}
�j���A���������\�[�X���琶������Ă��܂��D���e�� 4.1 �ɍ��킹�čX�V��
��Ă��܂��D�ꕔ�������̕���������܂����C�����Julius�{�̂̃����[�X��
���킹�ĉ��M�E�X�V����Ă����܂��D

Juliusbook �ɂ��Ă͕ʓr�_�E�����[�h���Ă��������D

���̑��C�C���_�̏ڍׂɂ��Ă� Release-ja.txt �������������D


�t�@�C���̍\��
===============

	00readme-ja.txt		�ŏ��ɓǂޕ����i���̃t�@�C���j
	LICENSE.txt		���C�Z���X����
	Release-ja.txt		�����[�X�m�[�g/�ύX����
	configure		configure�X�N���v�g
	configure.in		
	Sample.jconf		jconf �ݒ�t�@�C���T���v��
	julius/			Julius �{�̃\�[�X
	libsent/		Julius ���C�u�����\�[�X
	adinrec/		�^���c�[�� adinrec
	adintool/		�����^��/����M�c�[�� adintool
	generate-ngram/		N-gram�������c�[��
	gramtools/		���@�쐬�c�[���Q
	jcontrol/		�T���v���l�b�g���[�N�N���C�A���g jcontrol
	mkbingram/		�o�C�i��N-gram�쐬�c�[�� mkbingram
	mkbinhmm/		�o�C�i��HMM�쐬�c�[�� mkbinhmm
	mkgshmm/		GMS�p�������f���ϊ��c�[�� mkgshmm
	mkss/			�m�C�Y���σX�y�N�g���Z�o�c�[�� mkss
	jclient-perl/		A simple perl version of module mode client
	plugin/			�v���O�C���\�[�X�R�[�h�̃T���v���Ǝd�l����
	man/			�}�j���A����
	support/		�J���p�X�N���v�g


�g�p���@�E�h�L�������g
=======================

�{�A�[�J�C�u�ɕt�����Ă���̂̓\�[�X�R�[�h�C�o�[�W�����՗��C�T���v����
jconf �ݒ�t�@�C������ъe��I�����C���}�j���A��(.man)�݂̂ł��D������
�h�L�������g�� Julius �� Web �y�[�W�ɂčŐV�ł����邱�Ƃ��ł��܂��D��
�ׂẴI�v�V�����̐�����R���p�C�����@�C�`���[�g���A������l�X�Ȏg�p��
�@�C�e�@�\�̏Љ�C�����������̎���������܂��̂ŁC��������䗗�������D

    �z�[���y�[�W�Fhttp://julius.sourceforge.jp/

�܂��C��L�z�[���y�[�W�ɂ����āCJulius��p����������A�v���P�[�V�����J
���Ɋւ�����������s�����߂́u�J���҃t�H�[�����v��ݒu���Ă���܂��D
�ŐV�� Julius �� CVS �X�V���Ȃǂ����e����܂��D
�ǂ����A�N�Z�X���������D

    Julius Forum: http://julius.sourceforge.jp/forum/


���C�Z���X
===========

Julius �̓I�[�v���\�[�X�\�t�g�E�F�A�ł��D
�w�p�p�r�E���p���܂߁C���p�Ɋւ��ē��ɐ����͂���܂���D
���p�����ɂ��ẮC�����̕��� "LICENSE.txt" �ɂ���܂��̂ł��ǂ݉������D


�A����
===========

Julius �Ɋւ��邲����E���₢���킹�́C��LWeb�y�[�W��̃t�H�[�����C
���邢�͉��L�̃��[���A�h���X�܂ł��₢���킹������
('at' �� '@' �ɓǂݑւ��Ă�������)

	julius-info at lists.sourceforge.jp

�ȏ�
