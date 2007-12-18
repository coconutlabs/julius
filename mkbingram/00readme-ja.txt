MKBINGRAM(1)                                                      MKBINGRAM(1)



NAME
       mkbingram - make binary N-gram from arpa N-gram file

SYNOPSIS
       mkbingram -nlr forward_ngram.arpa -nrl backward_ngram.arpa bingram

DESCRIPTION
       mkbingram は，ARPA形式の前向き/後向き N-gram をバイナリ形式のファイルに
       結合・変換するツールです．これを使用することで，Juliusの起動を大幅に 高
       速化することができます．

       Rev.4.0   からは4-gram以上のN-gramも扱えるようになりました．上限値は 10
       です．

       前向きN-gramが "-nlr" で指定され，後向きN-gramが指定さ れ な い 場 合，
       mkbingram は 前向きN-gramだけからバイナリN-gramを生成します．このバイナ
       リN-gramを使うとき，Julius はその中の 2-gram を使って第1パスを行い，第2
       パ スではその前向き確率から後向きの確率を，ベイズ則に従って算出しながら
       認識を行います．

       後向きN-gramが "-nrl" で指定され，前向きN-gramが指定さ れ な い 場 合，
       mkbingramは後ろ向きN-gramだけからバイナリN-gramを生成します．このバイナ
       リN-gramを使うとき，Julius はその中の後向き 2-gram からベイズ則に従って
       算出しながら第1パスの認識を行い，第2パスでは後向き N-gramを使った認識を
       行います．

       両方が指定されたときは，前向きN-gram中の2-gramと後向きN-gramが統合さ れ
       たバイナリN-gramが生成されます．Juliusではその前向き2-gramで第1パスを行
       い，後向きN-gramで第2パスを行います．なお両 N-gram は同一のコーパスから
       同 一の条件（カットオフ値，バックオフ計算方法等）で学習されてあり，同一
       の語彙を持っている必要があります．

       mkbingram は gzip 圧縮された ARPA ファイルをそのまま読み込めます．

       4.0以降のJuliusに付属のmkbingramを使って変換したバイナリN-gramファイ ル
       は， 3.xでは読み込めませんのでご注意ください．

OPTIONS
       -nlr forward_ngram.arpa
              ARPA標準形式の前向き単語 N-gram ファイル．

       -nrl backward_ngram.arpa
              ARPA標準形式の逆向き単語 N-gram ファイル．

       -d バイナリN-gram
              入力とするバイナリN-gramファイル（古いバイナリN-gramの再変換用）

       bingram
              出力ファイル（Julius用バイナリ形式）

EXAMPLE
       ARPA形式のN-gramをバイナリ形式に変換する：

           % mkbingram -nlr ARPA_2gram -nrl ARPA_rev_3gram outfile

       古いバイナリN-gramファイルを3.5以降の形式に変換する：

           % mkbingram -d old_bingram new_bingram


USAGE
       Julius で言語モデル指定時に，元の ARPA 形式ファイルを  "-nlr  2gramfile
       -nrl  rev3gramfile" とする代わりに mkbingram で変換したバイナリ形式ファ
       イルを "-d bingramfile" と指定します．

SEE ALSO
       julius(1)

BUGS
       バグ報告・問い合わせ・コメント な ど は  julius-info  at  lists.source-
       forge.jp までお願いします．

COPYRIGHT
       Copyright (c) 1991-2007 京都大学 河原研究室
       Copyright (c) 2000-2005 奈良先端科学技術大学院大学 鹿野研究室
       Copyright (c) 2005-2007 名古屋工業大学 Julius開発チーム

AUTHORS
       李 晃伸 (名古屋工業大学) が実装しました．

LICENSE
       Julius の使用許諾に準じます．



4.3 Berkeley Distribution            LOCAL                        MKBINGRAM(1)
