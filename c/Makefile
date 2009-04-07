CFLAGS=-g --std=gnu99
CPPFLAGS=-I/Users/dustin/include
LDFLAGS=-g -L. -L/Users/dustin/lib

bot: libmemagent.a bot.o
	$(CC) -o $@ bot.o $(LDFLAGS) -lmemagent -lexpat -lstrophe -lresolv

clean:
	-rm bot *.o *.a

libmemagent.a: memagent.o
	$(AR) rcs $@ $<
	ranlib $@