�T���v���v���O�C��


�� �v���O�C���ɂ���

Julius-4.1 ���v���O�C�����g����悤�ɂȂ�܂����D
���̃f�B���N�g���ɂ́C�v���O�C���̃T���v���\�[�X���܂܂�Ă��܂��D

�v���O�C���̎d�g�݂�T�v�C�����ȂǑS�ʓI�Ȏ����ɂ��ẮC
Juliusbook ���������������D
�T���v���\�[�X�ɂ́C�֐��̎d�l���R�����g�Ƃ��ď�����Ă��܂��D


�� �t�@�C���\��

    00readme.txt		���̃t�@�C��
    plugin_defs.h		�v���O�C���p��`�w�b�_
    adin_oss.c			�������̓v���O�C���̃T���v���FOSS�}�C�N����
    audio_postprocess.c		�����㏈���v���O�C���̃e���v���[�g
    fvin.c			�����ʓ��̓v���O�C���̃e���v���[�g
    feature_postprocess.c	�����ʌ㏈���v���O�C���̃e���v���[�g
    calcmix.c			�K�E�X���z�W���v�Z�v���O�C���̃T���v��
    Makefile			Linux �p Makefile


�� �v���O�C���̎d�l�ƃR���p�C���ɂ���

�v���O�C���t�@�C���̊g���q�� .jpi �ł��D
�t�@�C���̎��Ԃ́C�P�Ȃ鋤�L�I�u�W�F�N�g�ł��D
gcc �ł���Έȉ��̂悤�ɂ��ăR���p�C�����Ă��������D

	 % gcc -shared -o adin_oss.jpi adin_oss.c


�� Julius �Ƀv���O�C����ǂݍ��܂�����@

Julius �̃I�v�V���� "-plugindir dirname" ���g���܂��Ddirname �ɂ̓v��
�O�C����u���Ă���f�B���N�g�����w�肵�Ă��������D
�w�肵���f�B���N�g�����ɂ���S�Ă� .jpi �t�@�C�����ǂݍ��܂�܂��D

�Ȃ��C�I�v�V�������g������v���O�C�������݂���̂ŁC"-plugindir" ��
�ݒ�̂ł��邾���ŏ��̂ق��Ŏw�肵�������悢�ł��傤�D


�� ����e�X�g

�t���� adin_oss.c �́COSS API ���g�������̓v���O�C���̃T���v���ł��D
Julius �{�̂���� "-input myadin" �őI���ł��܂��D

Julius ���R���p�C����C�ȉ��̂悤�ɂ��Ď����Ă݂܂��傤�D

	% cd plugin (���̃f�B���N�g��)
	% make adin_oss.jpi
	% cd ..
	% ./julius/julius -plugindir plugin -input myadin

�܂��C�������̓v���O�C���� adintool �� adinrec ������Ăяo���܂��D

	% ./adinrec/adinrec -plugindir plugin -input myadin
