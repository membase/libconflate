CFLAGS=-g --std=gnu99
CPPFLAGS=-I$(HOME)/include
LDFLAGS=-g -L. -L$(HOME)/lib

bot: libmemagent.a bot.o
	$(CC) -o $@ bot.o $(LDFLAGS) -lmemagent -lexpat -lstrophe -lresolv

clean:
	-rm bot *.o *.a

libmemagent.a: memagent.o
	$(AR) rcs $@ $<
	ranlib $@