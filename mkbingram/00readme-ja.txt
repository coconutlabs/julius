MKBINGRAM(1)                                                      MKBINGRAM(1)



NAME
       mkbingram - make binary N-gram from arpa N-gram file

SYNOPSIS
       mkbingram -nlr forward_ngram.arpa -nrl backward_ngram.arpa bingram

DESCRIPTION
       mkbingram �́CARPA�`���̑O����/����� N-gram ���o�C�i���`���̃t�@�C����
       �����E�ϊ�����c�[���ł��D������g�p���邱�ƂŁCJulius�̋N����啝�� ��
       �������邱�Ƃ��ł��܂��D

       Rev.4.0   �����4-gram�ȏ��N-gram��������悤�ɂȂ�܂����D����l�� 10
       �ł��D

       �O����N-gram�� "-nlr" �Ŏw�肳��C�����N-gram���w�肳 �� �� �� �� ���C
       mkbingram �� �O����N-gram��������o�C�i��N-gram�𐶐����܂��D���̃o�C�i
       ��N-gram���g���Ƃ��CJulius �͂��̒��� 2-gram ���g���đ�1�p�X���s���C��2
       �p �X�ł͂��̑O�����m�����������̊m�����C�x�C�Y���ɏ]���ĎZ�o���Ȃ���
       �F�����s���܂��D

       �����N-gram�� "-nrl" �Ŏw�肳��C�O����N-gram���w�肳 �� �� �� �� ���C
       mkbingram�͌�����N-gram��������o�C�i��N-gram�𐶐����܂��D���̃o�C�i
       ��N-gram���g���Ƃ��CJulius �͂��̒��̌���� 2-gram ����x�C�Y���ɏ]����
       �Z�o���Ȃ����1�p�X�̔F�����s���C��2�p�X�ł͌���� N-gram���g�����F����
       �s���܂��D

       �������w�肳�ꂽ�Ƃ��́C�O����N-gram����2-gram�ƌ����N-gram�������� ��
       ���o�C�i��N-gram����������܂��DJulius�ł͂��̑O����2-gram�ő�1�p�X���s
       ���C�����N-gram�ő�2�p�X���s���܂��D�Ȃ��� N-gram �͓���̃R�[�p�X����
       �� ��̏����i�J�b�g�I�t�l�C�o�b�N�I�t�v�Z���@���j�Ŋw�K����Ă���C����
       �̌�b�������Ă���K�v������܂��D

       mkbingram �� gzip ���k���ꂽ ARPA �t�@�C�������̂܂ܓǂݍ��߂܂��D

       4.0�ȍ~��Julius�ɕt����mkbingram���g���ĕϊ������o�C�i��N-gram�t�@�C ��
       �́C 3.x�ł͓ǂݍ��߂܂���̂ł����ӂ��������D

OPTIONS
       -nlr forward_ngram.arpa
              ARPA�W���`���̑O�����P�� N-gram �t�@�C���D

       -nrl backward_ngram.arpa
              ARPA�W���`���̋t�����P�� N-gram �t�@�C���D

       -d �o�C�i��N-gram
              ���͂Ƃ���o�C�i��N-gram�t�@�C���i�Â��o�C�i��N-gram�̍ĕϊ��p�j

       bingram
              �o�̓t�@�C���iJulius�p�o�C�i���`���j

EXAMPLE
       ARPA�`����N-gram���o�C�i���`���ɕϊ�����F

           % mkbingram -nlr ARPA_2gram -nrl ARPA_rev_3gram outfile

       �Â��o�C�i��N-gram�t�@�C����3.5�ȍ~�̌`���ɕϊ�����F

           % mkbingram -d old_bingram new_bingram


USAGE
       Julius �Ō��ꃂ�f���w�莞�ɁC���� ARPA �`���t�@�C����  "-nlr  2gramfile
       -nrl  rev3gramfile" �Ƃ������� mkbingram �ŕϊ������o�C�i���`���t�@
       �C���� "-d bingramfile" �Ǝw�肵�܂��D

SEE ALSO
       julius(1)

BUGS
       �o�O�񍐁E�₢���킹�E�R�����g �� �� ��  julius-info  at  lists.source-
       forge.jp �܂ł��肢���܂��D

COPYRIGHT
       Copyright (c) 1991-2007 ���s��w �͌�������
       Copyright (c) 2000-2005 �ޗǐ�[�Ȋw�Z�p��w�@��w ���쌤����
       Copyright (c) 2005-2007 ���É��H�Ƒ�w Julius�J���`�[��

AUTHORS
       �� �W�L (���É��H�Ƒ�w) ���������܂����D

LICENSE
       Julius �̎g�p�����ɏ����܂��D



4.3 Berkeley Distribution            LOCAL                        MKBINGRAM(1)
