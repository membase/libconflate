CFLAGS=-g --std=gnu99 -Wall -Werror
CPPFLAGS=-I$(HOME)/include
LDFLAGS=-g -L. -L$(HOME)/lib

LOBJS=kvpair.o util.o persist.o xmpp.o

bot: libconflate.a bot.o
	$(CC) -o $@ bot.o $(LDFLAGS) -lconflate -lsqlite3 -lstrophe -lexpat -lresolv -lpthread

clean:
	-rm bot *.o *.a

install: libconflate.a
	cp libconflate.a $(HOME)/lib
	cp conflate.h $(HOME)/include

libconflate.a: $(LOBJS)
	$(AR) rcs $@ $(LOBJS)
	ranlib $@

$(LOBJS): Makefile conflate.h

bot.o: Makefile bot.c conflate.h
