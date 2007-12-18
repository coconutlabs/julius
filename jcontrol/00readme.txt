JCONTROL(1)                                                        JCONTROL(1)



NAME
       jcontrol - simple program to control Julius module via API

SYNOPSIS
       jcontrol hostname [portnum]

DESCRIPTION
       jcontrol is a simple console program to control julius running on other
       host via network API.  It can send command to Julius, and receive  mes-
       sages from Julius.

       When  invoked,  jcontrol  tries to connect to Julius running in "module
       mode" on specified hostname.  After  connection  established,  jcontrol
       waits for user commands from standard input.

       When  user types a command to jcontrol, it will be interpreted and cor-
       responding API command will be sent  to  Julius.   When  a  message  is
       received from Julius, its content will be output to standard output.

       For details about the API, see the related documents.

OPTIONS
       hostname
              Host name where Julius is runnning in module mode.

       portnum
              (optional) port number. (default=10500)

COMMANDS (COMMON)
       After startup, the command string below can be input from stdin.

       pause  Stop recognition, cutting speech input at that point if any.

       terminate
              Stop recognition, discarding the current speech input if any.

       resume (re)start recognition.

       inputparam arg
              Tell  Julius  how  to  deal with speech input in case grammar is
              changed just when recognition is running.  Specify one:  "TERMI-
              NATE", "PAUSE", "WAIT"

       version
              Return version number.

       status Return trigger status (active/sleep).

COMMANDS (GRAMMAR)
       Below are Grammar-related command strings:

       changegram prefix
              Change  recognition  grammar  to "prefix.dfa" and "prefix.dict".
              All the current grammars used in Julius are deleted and replaced
              to the specifed grammar.

       addgram prefix
              tell  Julius  to  use  additional grammar "prefix.dfa" and "pre-
              fix.dict" for recognition.  The specified grammars are added  to
              the list of recognition grammars, and then activated.

       deletegram ID
              tell  Julius  to  delete  grammar  of  the  specified "ID".  The
              deleted grammar will be erased from Julius.  The grammar "ID" is
              sent from Julius at each time grammar information has changed.

       deactivategram ID
              tell  Julius  to  de-activate  a grammar.  The specified grammar
              will become temporary OFF, and skipped from recognition process.
              These de-activated grammars are kept in Julius, and can be acti-
              vated by "activategram" command.

       activategram ID
              tell Julius to activate previously de-activated grammar.

       syncgram
              tell Julius to update grammar status now.

COMMANDS (PROCESS)
       listprocess
              returns list of existing recognition process.

       currentprocess name
              switches the currently manipulating process to it.

       shiftprocess
              rotate the curenttly manipulating process.

       addprocess jconffile
              adds a set of LM process SR process to the running engine.   The
              jconf  file should contain only one LM specification, and can be
              an accessible path at the server.  The new processes  will  have
              the name of the jconf file.

       delprocess name
              removes a SR process with specified name from engine.

       activateprocess name
              enables  a  SR  process  previously deactivated and turn it into
              live status.

       deactivateprocess name
              disables a specified SR process and turn it into dead status.

       addword gram_id dictfile
              Send words in the dictfile to engine and add them to the grammar
              specified by gram_id at current process.

EXAMPLE
       The  dump  messages  from  Julius  are  output  to tty with prefix "> "
       appended to each line.

       See related documents for more details.

       (1) start Julius in module mode at host 'host'.
           % julius -C xxx.jconf ... -input mic -module

       (2) (on other tty) start jcontrol, and start communication.
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

VERSION
       This version is provided as part of Julius-4.0.

COPYRIGHT
       Copyright (c) 2002-2007 Kawahara Lab., Kyoto University
       Copyright (c) 2002-2005 Shikano Lab., Nara  Institute  of  Science  and
       Technology
       Copyright  (c) 2005-2007 Julius project team, Nagoya Institute of Tech-
       nology

AUTHORS
       LEE Akinobu (Nagoya Institute of Technology, Japan)
       contact: julius-info at lists.sourceforge.jp

LICENSE
       Same as Julius.



4.3 Berkeley Distribution            LOCAL                         JCONTROL(1)
