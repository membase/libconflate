CFLAGS=-g --std=gnu99 -Wall -Werror
CPPFLAGS=-I$(HOME)/include
LDFLAGS=-g -L. -L$(HOME)/lib

LOBJS=kvpair.o util.o persist.o xmpp.o

bot: libmemagent.a bot.o
	$(CC) -o $@ bot.o $(LDFLAGS) -lmemagent -lsqlite3 -lstrophe -lexpat -lresolv -lpthread

clean:
	-rm bot *.o *.a

install: libmemagent.a
	cp libmemagent.a $(HOME)/lib
	cp memagent.h $(HOME)/include

libmemagent.a: $(LOBJS)
	$(AR) rcs $@ $(LOBJS)
	ranlib $@

$(LOBJS): Makefile memagent.h

bot.o: Makefile bot.c memagent.h
