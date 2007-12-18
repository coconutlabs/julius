JCONTROL(1)                                                        JCONTROL(1)



NAME
       jcontrol - simple program to control Julius module via API

SYNOPSIS
       jcontrol hostname [portnum]

DESCRIPTION
       jcontrol  �́C���̃z�X�g�œ��쒆�� julius ���CAPI����ăR���g���[����
       ��ȒP�ȃR���\�[���v���O�����ł��D Julius �ւ̃R �} �� �h �� �M�C �� ��
       ��Julius����̃��b�Z�[�W��M���s�����Ƃ��ł��܂��D

       �N����Cjcontrol �́C�w��z�X�g��ɂ����āu���W���[�����[�h�v�œ��쒆��
       Julius �ɑ΂��C�ڑ������݂܂��D�ڑ��m����Cjcontrol �̓��[�U�[����� �R
       �}���h���͑҂���ԂƂȂ�܂��D

       jcontrol   �� ���[�U�[�����͂����R�}���h�����߂��C�Ή�����API�R�}���h��
       Julius �֑��M���܂��D�܂��CJulius ����F�����ʂ���̓g���K��� �� �� ��
       ���b�Z�[�W�����M����Ă����Ƃ��́C���̓��e��W���o�͂֏����o���܂��D

       API�̏ڍׂɂ��Ă͊֘A�����������������D

OPTIONS
       hostname
              �ڑ���̃z�X�g���iJulius �����W���[�����[�h�œ��쒆�j

       portnum
              (optional) �|�[�g�ԍ� (default=10500)

COMMANDS (COMMON)
       �N �� ��Cjcontrol �ɑ΂��Ĉȉ��̃R�}���h�������W�����͂�����͂ł���
       ���D

       pause  �F���𒆒f����D�F���r���̏ꍇ�C�����œ��͂𒆒f���đ�2�p�X�� ��
              �F�����I����Ă��璆�f����D

       terminate
              �F���𒆒f����D�F���r���̏ꍇ�C���͂�j�����đ������f����D

       resume �F�����ĊJ�D

       inputparam arg
              �� �@ �؂�ւ����ɉ������͂ł������ꍇ�̓��͒������̈������w��D
              "TERMINATE", "PAUSE", "WAIT"�̂��������ꂩ���w��D

       version
              �o�[�W�����������Ԃ�

       status �V�X�e���̏��(active/sleep)��Ԃ��D

GRAMMAR COMMANDS
       ���@�E�P��F���p�̃R�}���h�ł��F

       changegram prefix
              �F�����@�� "prefix.dfa" �� "prefix.dict" �ɐ؂�ւ���D�J���� �g
              �v���Z�X���̕��@�͑S�ď�������C�w�肳�ꂽ���@�ɒu�������D

       addgram prefix
              �F�����@�Ƃ��� "prefix.dfa" �� "prefix.dict" ��ǉ�����D

       deletegram ID
              �w�肳�ꂽID�̔F�����@���폜����D�w�蕶�@�̓J�����g�v���Z�X����
              �폜�����DID �� Julian ���瑗���� GRAMINFO ���ɋL�q����� ��
              ��D

       deactivategram ID
              �w�肳�ꂽID�̔F�����@���C�ꎞ�I��OFF�ɂ���DOFF�ɂ��ꂽ���@�͔F
              ����������ꎞ�I�ɏ��O�����D����OFF�ɂ��ꂽ���@�� Julius �� ��
              �ێ�����C "activategram" �R�}���h�ōĂ� ON �ɂł���D

       activategram ID
              �ꎞ�I�� OFF �ɂȂ��Ă������@���Ă� ON �ɂ���D

       syncgram
              �X�V���ꂽ���@�𑦎��K������D

COMMANDS (PROCESS)
       listprocess
              ���݃G���W���ɂ���F���v���Z�X�̈ꗗ�������D

       currentprocess name
              �R�}���h�����s����Ώۂ̃J�����g�v���Z�X���w�肳�ꂽ���̂ɐؑւ�
              ��D

       shiftprocess
              �R�}���h�����s����Ώۂ̃J�����g�v���Z�X�����ɐؑւ���D

       addprocess jconffile
              �G���W���ɔF���v���Z�X��ǉ�����Djconffile �͂P�� LM �ݒ����
              �ނ��̂ŁC�T�[�o�����猩����K�v���L��D�ǉ����ꂽ LM ����єF��
              �v���Z�X�� jconffile �̖��O���v���Z�X���ƂȂ�D

       delprocess name
              �w�肳�ꂽ���O�̔F���v���Z�X���G���W������폜����D

       activateprocess name
              �ȑO�Ɉꎞ���������ꂽ�v���Z�X���ēx�L��������D

       deactivateprocess name
              �w�肳�ꂽ�v���Z�X���ꎞ����������D

       addword gram_id dictfile
              dictfile���̒P����C�J�����g�v���Z�X�� gram_id �̕��@�� �� �� ��
              ��D�i���@�E�P��F���̂݁j

EXAMPLE
       Julius ����̃��b�Z�[�W�� "> " ���s�̐擪�ɂ��Ă��̂܂ܕW���o�͂ɏo��
       ����܂��D�o�͓��e�̏ڍׂɂ��ẮC�֘A�������Q�Ƃ��Ă��������D

       (1) Julius �����W���[�����[�h�Ńz�X�g host �ŋN������D
           % julius -C xxx.jconf ... -input mic -module

       (2) (���̒[����) jcontrol ���N�����C�ʐM���J�n����D
           % jcontrol host
           connecting to host:10500...done
           > <GRAMINFO>
           >  # 0: [active] 99words, 42categories, 135nodes (new)
           > </GRAMINFO>
           > <GRAMINFO>
           >  # 0: [active] 99words, 42categories, 135 nodes
           >   Grobal:      99words, 42categories, 135nodes
           > </GRAMINFO>
           > <INPUT STATUS="LISTEN" TIME="1031583083"/>
        -> pause
        -> resume
           > <INPUT STATUS="LISTEN" TIME="1031583386"/>
        -> addgram test
           ....


SEE ALSO
       julius(1)

BUGS
       �o�O�񍐁E�₢���킹�E�R�����g �� �� ��  julius-info  at  lists.source-
       forge.jp �܂ł��肢���܂��D

VERSION
       This version is provided as part of Julius-3.5.1.

COPYRIGHT
       Copyright (c) 2002-2007 ���s��w �͌�������
       Copyright (c) 2002-2005 �ޗǐ�[�Ȋw�Z�p��w�@��w ���쌤����
       Copyright (c) 2005-2007 ���É��H�Ƒ�w Julius�J���`�[��

AUTHORS
       �� �W�L (���É��H�Ƒ�w) ���������܂����D

LICENSE
       Julius �̎g�p�����ɏ����܂��D



4.3 Berkeley Distribution            LOCAL                         JCONTROL(1)
