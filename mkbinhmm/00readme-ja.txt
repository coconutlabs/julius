MKBINHMM(1)                                                        MKBINHMM(1)



NAME
       mkbinhmm - convert HMM definition file to binary format for Julius

SYNOPSIS
       mkbinhmm [-C HTK_Config] hmmdefs_file binhmm_file

DESCRIPTION
       mkbinhmm �́CHTK�`���̃A�X�L�[�`����HMM��`�t�@�C����Julius�p�̃o�C�i��
       �`���֕ϊ�����D����̃o�C�i���`����HMM��`�t�@�C�����g�p���邱�� �� ��
       ��C Julius�̋N�������������邱�Ƃ��ł���D

       mkbinhmm �� gzip ���k���ꂽHMM��`�t�@�C�������̂܂ܓǂݍ��߂܂��D

       �ϊ�����HMM�̊w�K�p�p�����[�^�t�@�C���̐����ɗp���� HTK Config �t�@�C��
       �� "-C" ���邢�� "-htkconf" �Ŏw�肷�邱�ƂŁC���̉��������ʒ��o���� ��
       �o�̓t�@�C���ɖ��ߍ��ނ��Ƃ��ł��܂��DJulius �͖��ߍ��܂ꂽ�p�����[�^��
       ������ƁC���̒l��ǂݍ���Ŏ����I�ɉ����f�[�^����̉��������ʏ��� ��
       �� �ėp���܂��D����ɂ���āC���f���w�K���Ɏg�p���������ʂ̐ݒ��Julius
       �Ŏ����I�ɃZ�b�g���邱�Ƃ��ł��܂��D

       ���͂Ƃ��ăo�C�i���`���ɕϊ��ς݂�HMM���w�肷�邱�Ƃ��ł��܂��D�� �� ��
       �g���āC�����̃o�C�i��HMM�ɓ����ʒ��o�����p�����[�^�𖄂ߍ��ނ��Ƃ��ł�
       �܂��D���͂Ɋ��Ƀp�����[�^�����ߍ��܂�Ă��� Config �t�@�C�����w�肳 ��
       �Ă���ꍇ�́C�㏑������ďo�͂���܂��D

OPTIONS
       -C ConfigFile
              �w�K���ɓ����ʒ��o�Ɏg�p����HTK Config�t�@�C�����w�肷��D�w�肳
              �ꂽ�ꍇ�C���̒l���o�̓t�@�C���̃w�b�_�ɖ��ߍ��܂��D

       -htkconf ConfigFile
              "-C" �Ɠ����D

USAGE
       �o�C�i���`��HMM��`���f����Julius�Ŏg���ɂ́CJulius �ŉ������f���w�� ��
       �ɁC ���� ASCII�`���t�@�C���̑���ɂ��̃t�@�C�����w�肷�邾���ł悢�D
       ascii/binary �̌`����Julius�Ŏ������ʂ����D�p�����[�^�����ߍ��܂�Ă�
       ��ꍇ�� Julius �������ǂݏo���ăZ�b�g����D

SEE ALSO
       julius(1)

BUGS
       �o �O �� ���E �� �� ���킹�E�R�����g�Ȃǂ� julius-info at lists.source-
       forge.jp �܂ł��肢���܂��D

COPYRIGHT
       Copyright (c) 2003-2006 ���s��w �͌�������
       Copyright (c) 2003-2005 �ޗǐ�[�Ȋw�Z�p��w�@��w ���쌤����
       Copyright (c) 2005-2006 ���É��H�Ƒ�w Julius�J���`�[��

AUTHORS
       �� �W�L (���É��H�Ƒ�w) ���������܂����D

LICENSE
       Julius �̎g�p�����ɏ����܂��D



4.3 Berkeley Distribution            LOCAL                         MKBINHMM(1)
