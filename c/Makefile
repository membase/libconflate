CFLAGS=-g --std=gnu99
CPPFLAGS=-I$(HOME)/include
LDFLAGS=-g -L. -L$(HOME)/lib

LOBJS=xmpp.o servers.o util.o

bot: libmemagent.a bot.o
	$(CC) -o $@ bot.o $(LDFLAGS) -lmemagent -lstrophe -lexpat -lresolv -lpthread

clean:
	-rm bot *.o *.a

libmemagent.a: $(LOBJS)
	$(AR) rcs $@ $(LOBJS)
	ranlib $@