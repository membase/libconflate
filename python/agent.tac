import sys
sys.path.insert(0, "lib/wokkel")
sys.path.insert(0, "lib")

import ConfigParser

from twisted.application import service
from twisted.internet import task, reactor
from twisted.words.protocols.jabber import jid
from wokkel.client import XMPPClient
from wokkel.generic import VersionHandler
from wokkel.keepalive import KeepAlive
from wokkel.disco import DiscoHandler

from memagent import config
from memagent import protocol

application = service.Application("memagent")

def build_client(section):
    host = None
    try:
        host = config.CONF.get(section, 'host')
    except ConfigParser.NoSectionError:
        pass

    j = jid.internJID(config.CONF.get(section, 'jid'))

    xmppclient = XMPPClient(j, config.CONF.get(section, 'pass'), host)

    xmppclient.logTraffic = True

    # Stream handling protocols for memagent
    protocols = [protocol.MemAgentPresenceProtocol,
                 protocol.MemAgentMessageProtocol]

    for p in protocols:
        handler=p(j)
        handler.setHandlerParent(xmppclient)

    DiscoHandler().setHandlerParent(xmppclient)
    VersionHandler('memagent', config.VERSION).setHandlerParent(xmppclient)
    KeepAlive().setHandlerParent(xmppclient)
    xmppclient.setServiceParent(application)

    return xmppclient


build_client('xmpp')
