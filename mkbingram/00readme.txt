MKBINGRAM(1)                                                      MKBINGRAM(1)



NAME
       mkbingram - make binary N-gram from arpa N-gram file

SYNOPSIS
       mkbingram -nlr forward_ngram.arpa -nrl backward_ngram.arpa bingram

DESCRIPTION
       mkbingram  makes a binary N-gram file for Julius from forward (left-to-
       right) word N-gram and/or backward (right-to-left) word N-gram  LMs  in
       ARPA  standard  format.   Using the binary file, the initial startup of
       Julius becomes much faster.

       From rev. 4.0, longer N-gram (N < 10) is supported.

       When only a forward N-gram is specified by "-nlr" and  no  backward  N-
       gram  is  specified,  mkbingram generates binary N-gram for recognition
       with only the forward N-gram.  The 1st pass will use the  2-gram  entry
       in  the  given N-gram, and The 2nd pass will use the given N-gram, with
       converting forward probabilities to  backward  probabilities  by  Bayes
       rule.

       When  only  a  backward N-gram is specified by "-nrl" and no forward N-
       gram is specified, mkbingram generates binary  N-gram  for  recognition
       with  only  the  backward  N-gram.   The  1st pass will use the forward
       2-gram probability computed from the backward 2-gram using Bayes  rule.
       The 2nd pass fully use the given backward N-gram.

       When  both  forward  and backward N-grams are specified, forward 2-gram
       part and backward N-gram are  gathered  together  into  single  bingram
       file,  to  use  the forward 2-gram for the 1st pass and backward N-gram
       for the 2nd pass.  Note that both N-gram should be trained in the  same
       corpus with same parameters (i.e. cut-off thresholds), with same vocab-
       ulary.

       mkbingram can read gzipped ARPA file.

       Please note that binary N-gram file converted by mkbingram  of  version
       4.0 and later cannot be read by Julius 3.x.

OPTIONS
       -nlr forward_ngram.arpa
              Forward  (left-to-right)  word N-gram file in ARPA standard for-
              mat.

       -nrl backward_ngram.arpa
              Backward (right-to-left) word N-gram file in ARPA standard  for-
              mat.

       -d old_bingram
              Read  in  an  old  binary N-gram file (for conversion to the new
              format).

       bingram
              output binary N-gram file.

EXAMPLE
       Convert ARPA files to binary format:

           % mkbingram -nlr ARPA_2gram -nrl ARPA_rev_3gram outfile

       Convert old binary N-gram file to new format:

           % mkbingram -d old_bingram new_bingram


SEE ALSO
       julius(1)

COPYRIGHT
       Copyright (c) 1991-2007 Kawahara Lab., Kyoto University
       Copyright (c) 2000-2005 Shikano Lab., Nara  Institute  of  Science  and
       Technology
       Copyright  (c) 2005-2007 Julius project team, Nagoya Institute of Tech-
       nology

AUTHORS
       LEE Akinobu (Nagoya Institute of Technology, Japan)
       contact: julius-info at lists.sourceforge.jp

LICENSE
       Same as Julius.



4.3 Berkeley Distribution            LOCAL                        MKBINGRAM(1)
