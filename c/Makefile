CFLAGS=--std=gnu99
CPPFLAGS=-I/Users/dustin/include
LDFLAGS=-L. -L/Users/dustin/lib

bot: libmemagent.a bot.o
	$(CC) -o $@ bot.o $(LDFLAGS) -lmemagent -lexpat -lstrophe -lresolv

libmemagent.a: memagent.o
	$(AR) rcs $@ $<
	ranlib $@