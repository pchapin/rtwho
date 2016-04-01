#
# FILE        : Makefile
# LAST REVISED: 2007-09-19
# AUTHOR      : (C) Copyright 2007 by the VTC Computer Club
#

NETSTREAM=../../netstream
CPPOPTS=-c -Wall -g -I$(NETSTREAM) -pthread

all:		rtwhod

rtwhod:		rtwhod.o $(NETSTREAM)/libnetstream.a
	g++ -o rtwhod -g rtwhod.o -pthread -L $(NETSTREAM) -lnetstream

rtwhod.o:	rtwhod.cpp
	g++ $(CPPOPTS) rtwhod.cpp

#
# netstream related targets
#
$(NETSTREAM)/libnetstream.a:
	(cd $(NETSTREAM); make)

#
# Utility targets
#

clean:
	rm -f rtwhod *.o core *~
