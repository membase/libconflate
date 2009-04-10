CFLAGS=-g --std=gnu99 -Wall -Werror
CPPFLAGS=-I$(HOME)/include
LDFLAGS=-g -L. -L$(HOME)/lib

LOBJS=xmpp.o servers.o util.o persist.o

bot: libmemagent.a bot.o
	$(CC) -o $@ bot.o $(LDFLAGS) -lmemagent -lsqlite3 -lstrophe -lexpat -lresolv -lpthread

clean:
	-rm bot *.o *.a

libmemagent.a: $(LOBJS)
	$(AR) rcs $@ $(LOBJS)
	ranlib $@