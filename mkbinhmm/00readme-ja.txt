MKBINHMM(1)                                                        MKBINHMM(1)



NAME
       mkbinhmm - convert HMM definition file to binary format for Julius

SYNOPSIS
       mkbinhmm [-C HTK_Config] hmmdefs_file binhmm_file

DESCRIPTION
       mkbinhmm は，HTK形式のアスキー形式のHMM定義ファイルをJulius用のバイナリ
       形式へ変換する．これのバイナリ形式のHMM定義ファイルを使用すること に よ
       り， Juliusの起動を高速化することができる．

       mkbinhmm は gzip 圧縮されたHMM定義ファイルをそのまま読み込めます．

       変換時にHMMの学習用パラメータファイルの生成に用いた HTK Config ファイル
       を "-C" あるいは "-htkconf" で指定することで，その音響特徴量抽出条件 を
       出力ファイルに埋め込むことができます．Julius は埋め込まれたパラメータを
       見つけると，その値を読み込んで自動的に音声データからの音響特徴量条件 と
       し て用います．これによって，モデル学習時に使用した特徴量の設定をJulius
       で自動的にセットすることができます．

       入力としてバイナリ形式に変換済みのHMMを指定することもできます．こ れ を
       使って，既存のバイナリHMMに特徴量抽出条件パラメータを埋め込むことができ
       ます．入力に既にパラメータが埋め込まれてかつ Config ファイルが指定さ れ
       ている場合は，上書きされて出力されます．

OPTIONS
       -C ConfigFile
              学習時に特徴量抽出に使用したHTK Configファイルを指定する．指定さ
              れた場合，その値が出力ファイルのヘッダに埋め込まれる．

       -htkconf ConfigFile
              "-C" と同じ．

USAGE
       バイナリ形式HMM定義モデルをJuliusで使うには，Julius で音響モデル指定 時
       に， 元の ASCII形式ファイルの代わりにこのファイルを指定するだけでよい．
       ascii/binary の形式はJuliusで自動判別される．パラメータが埋め込まれてい
       る場合は Julius がそれを読み出してセットする．

SEE ALSO
       julius(1)

BUGS
       バ グ 報 告・ 問 い 合わせ・コメントなどは julius-info at lists.source-
       forge.jp までお願いします．

COPYRIGHT
       Copyright (c) 2003-2006 京都大学 河原研究室
       Copyright (c) 2003-2005 奈良先端科学技術大学院大学 鹿野研究室
       Copyright (c) 2005-2006 名古屋工業大学 Julius開発チーム

AUTHORS
       李 晃伸 (名古屋工業大学) が実装しました．

LICENSE
       Julius の使用許諾に準じます．



4.3 Berkeley Distribution            LOCAL                         MKBINHMM(1)
