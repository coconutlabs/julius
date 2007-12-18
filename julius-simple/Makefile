LIBSENT=../libsent
LIBJULIUS=../libjulius

CC=gcc
CFLAGS=-g -O2 

CPPFLAGS=-I. -I$(LIBJULIUS)/include -I$(LIBSENT)/include  `$(LIBSENT)/libsent-config --cflags` `$(LIBJULIUS)/libjulius-config --cflags`
LDFLAGS= -L$(LIBJULIUS) -ljulius -L$(LIBSENT) -lsent `$(LIBSENT)/libsent-config --libs` `$(LIBJULIUS)/libjulius-config --libs` -lm

############################################################

all: julius-simple

julius-simple: julius-simple.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o julius-simple julius-simple.c $(LDFLAGS)

clean:
	$(RM) *.o *.bak *~ core TAGS

distclean:
	$(RM) *.o *.bak *~ core TAGS
	$(RM) julius-simple
