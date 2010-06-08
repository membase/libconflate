# Building

First, you need to build and install libstrophe:

## Libstrophe

    git clone git://github.com/dustin/libstrophe.git
    cd libstrophe
    git submodule init
    git submodule update
    ./config/autorun.sh
    ./configure
    make

And then install it:

    mkdir ~/{lib,include}
    cp *.h ~/include
    cp *.a ~/lib

## The Library and Example Bot

Now you're ready to build the library and example application.

    git clone git://github.com/northscale/libconflate.git
    cd libconflate
    make

# Trying the Example

There's a bit of infrastructure required to see it all work, but the
example (bot.c) is ready to talk to your xmpp server and handle
messages.

    ./examples/bot myjid@example.com mypassword

(optionally, you can specify a third argument to tell it exactly what
xmpp server to connect to if you'd like to avoid SRV lookups)
