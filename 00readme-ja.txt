======================================================================
                  Large Vocabulary Continuous Speech
                          Recognition Engine

                                Julius

                                                (Rev 4.0.1 2008/03/12)
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


Julius-4.0.1
=============

Julius-4.0.1 �͎��4.0�̃o�O�C�����s���܂����D�܂��C���̃����[�X���C
Linux �ł� ALSA API ���W���ƂȂ�܂����D(�]���� OSS �֖߂������ꍇ�́C
�R���p�C������ "--with-mictype=oss" ���w�肵�Ă�������)


Julius-4.0
=============

Julius-4.0 �͂V�N�Ԃ�̃��W���[�o�[�W�����A�b�v�ł���C�g������_�
�̊m�ۂ�ڎw���������I�ȃ����[�X�ƂȂ��Ă��܂��DJulius�̓����f�[�^�\��
�̑啝�Ȍ���������у��W���[�������s�������ʁC�G���W���{�̂̃��C�u��
�����C���ꃂ�f���̓�������ъg���C�}���`�f�R�[�f�B���O�ȂǑ����̐�i�I
�@�\���������܂����D��ȐV�K�_�͈ȉ��̂Ƃ���ł��i�S�Ăł͂���܂���j�D

�� �G���W���{�̂̃��C�u�����������API�̐���
�� Julius / Julian �̈�{��
�� �������f����p�����}���`�f�R�[�f�B���O�̑Ή�
�� ���ꃂ�f���̓��I�ǉ��E�폜
�� 4-gram �ȏ�̒�����N-gram�ւ̑Ή�
�� ���[�U��`����֐��̃T�|�[�g
�� confusion network �o��
�� ������Ԍ��o (VAD) �̋����iGMM�E�f�R�[�_�x�[�X�j
�� �c�[���̋@�\�ǉ��E�V�c�[���ǉ��E�������Ǘ����P��

Julius-3.x ����͎g�����E�ݒ�t�@�C���ɂ����ď�ʌ݊������m�ۂ��Ă���C
�]���Ɠ��l�Ɏg�����Ƃ��o���܂��D�܂��F���̐��x����ё��x�́C���i�K�ł�
�O�o�[�W���� (3.5.3) �Ƃقړ������\���ێ����Ă��܂��D


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
���Ɋւ�����������s�����߂́u�J���҃t�H�[�����v��ݒu����\��ł��D


���C�Z���X
===========

Julius/Julian �̓I�[�v���\�[�X�\�t�g�E�F�A�ł��D
�w�p�p�r�E���p���܂߁C���p�Ɋւ��ē��ɐ����͂���܂���D
���p�����ɂ��ẮC�����̕��� "LICENSE.txt" �ɂ���܂��̂ł��ǂ݉������D


�A����
===========

Julius �Ɋւ��邲����E���₢���킹�́C��LWeb�y�[�W��̃t�H�[�����C
���邢�͉��L�̃��[���A�h���X�܂ł��₢���킹������
('at' �� '@' �ɓǂݑւ��Ă�������)

	julius-info at lists.sourceforge.jp

�ȏ�
