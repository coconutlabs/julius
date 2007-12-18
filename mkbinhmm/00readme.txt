MKBINHMM(1)                                                        MKBINHMM(1)



NAME
       mkbinhmm - convert HMM definition file to binary format for Julius

SYNOPSIS
       mkbinhmm [-C HTK_Config] hmmdefs_file binhmm_file

DESCRIPTION
       mkbinhmm convert an ascii hmmdefs in HTK format to a binary HMM defini-
       tion file for Julius.

       mkbinhmm can read gzipped hmmdefs file.

       By specifying the HTK Config file you used for training, you can  embed
       the  acoustic  analysis  conditions and variables used for the training
       into the output file.  You can specify the Config file by  either  "-C"
       or  "-htkconf".   The  vaules  in  the Config file will be converted to
       Julius specifications and embedded to the output file.   At  run  time,
       the values will be loaded into Julius and appropriate acoustic analysis
       parameters for the model will be automatically set.  This will  greatly
       help ensuring the same acoustic conditions to be applied at both train-
       ing and application time for waveform recognition.

       You can also specify a binary HMM as input  file.   This  is  for  only
       embedding Config parameters into the already existing binary files.  If
       the input binhmm already has  acoustic  analysis  parameters  embedded,
       they will be overridden by the specified HTK Config values.

OPTIONS
       -C ConfigFile
              HTK  Config  file  you used at training time.  If specified, the
              values are embedded to the output file.

       -htkconf ConfigFile
              Same as "-C".

USAGE
       At Julius, this binary hmmdefs can be used in the same way as the orig-
       inal ascii format, i.e. "-h".  The ascii/binary format will be automat-
       icall detected by Julius.  If Config parameters are embedded, the  val-
       ues are loaded into Julius and acoustic analysis parameters will be set
       to the values.

SEE ALSO
       julius(1)

COPYRIGHT
       Copyright (c) 2003-2006 Kawahara Lab., Kyoto University
       Copyright (c) 2003-2005 Shikano Lab., Nara  Institute  of  Science  and
       Technology
       Copyright  (c) 2005-2006 Julius project team, Nagoya Institute of Tech-
       nology

AUTHORS
       LEE Akinobu (Nagoya Institute of Technology, Japan)
       contact: julius-info at lists.sourceforge.jp

LICENSE
       Same as Julius.



4.3 Berkeley Distribution            LOCAL                         MKBINHMM(1)
