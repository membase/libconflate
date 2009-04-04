from twisted.internet import defer
from twisted.words.protocols.jabber import error, jid, xmlstream
from twisted.words.xish import domish

from twisted.python import log

from wokkel.iwokkel import IDisco
from wokkel.subprotocols import IQHandlerMixin, XMPPHandler
from wokkel.compat import toResponse

NS = 'http://jabber.org/protocol/commands'

IQ_SET = '/iq[@type="set"]'
COMMAND = IQ_SET + '/command[@xmlns="' + NS + '"]'

_handlers = {}

class BaseCommandHandler(XMPPHandler, IQHandlerMixin):

    iqHandlers = {COMMAND: '_onCommand'}

    def connectionInitialized(self):
        log.msg("Subscribing to %s" % COMMAND)
        self.xmlstream.addObserver(COMMAND, self.handleRequest)

    def _onCommand(self, iq):
        log.msg("Got a command.")

        cmd = iq.children[0].getAttribute('node')

        if cmd in _handlers:
            _handlers[cmd](self, iq, iq.children[0].children[0])
        else:
            log.msg("No handler for command %s" % cmd)
            res = toResponse(iq, 'error')
            error = res.addElement('error')
            error['code'] = '404'
            error['type'] = 'modify'
            error.addElement(('urn:ietf:params:xml:ns:xmpp-stanzas',
                            'item-not-found'))
            self.send(res)
            iq.handled = True

class BaseHandler(object):

    handles = None

class ServerListHandler(BaseHandler):

    handles = 'serverlist'

    def __call__(self, base, iq, cmd):
        log.msg("Handling a server list.")
        res = toResponse(iq, 'result')
        base.send(res)

for __t in (t for t in globals().values() if isinstance(type, type(t))):
    if BaseHandler in __t.__mro__:
        i = __t()
        if i.handles:
            _handlers[i.handles] = i
