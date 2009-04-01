#!/usr/bin/env python

from __future__ import with_statement

import time

from twisted.python import log
from twisted.internet import protocol, reactor, threads
from twisted.words.xish import domish
from twisted.words.protocols.jabber.jid import JID
from twisted.words.protocols.jabber.xmlstream import IQ

from wokkel.xmppim import MessageProtocol, PresenceClientProtocol
from wokkel.xmppim import AvailablePresence

import config
import string

class MemAgentMessageProtocol(MessageProtocol):

    def __init__(self, jid):
        super(MemAgentMessageProtocol, self).__init__()
        self._pubid = 1
        self.jid = jid.full()

    def connectionMade(self):
        log.msg("Connected!")

        # Mark connection?

    def connectionLost(self, reason):
        log.msg("Disconnected!")

        # Mark connection?

    def send_plain(self, jid, content):
        msg = domish.Element((None, "message"))
        msg["to"] = jid
        msg["from"] = self.jid
        msg["type"] = 'chat'
        msg.addElement("body", content=content)

        self.send(msg)

    def send_html(self, jid, body, html):
        msg = domish.Element((None, "message"))
        msg["to"] = jid
        msg["from"] = self.jid
        msg["type"] = 'chat'
        html = u"<html xmlns='http://jabber.org/protocol/xhtml-im'><body xmlns='http://www.w3.org/1999/xhtml'>"+unicode(html)+u"</body></html>"
        msg.addRawXml(u"<body>" + unicode(body) + u"</body>")
        msg.addRawXml(unicode(html))

        self.send(msg)

    def onMessage(self, msg):
        try:
            log.msg("Got a message from %s" % msg['from'])
            self.__onMessage(msg)
        except KeyError:
            log.err()

    def __onMessage(self, msg):
        if msg["type"] == 'chat' and hasattr(msg, "body") and msg.body:
            a=unicode(msg.body).strip().split(None, 1)
            args = a[1] if len(a) > 1 else None
            # Process a chat message.
        else:
            log.msg("Non-chat/body message: %s" % msg.toXml())

class MemAgentPresenceProtocol(PresenceClientProtocol):

    started = time.time()
    connected = time.time()

    def __init__(self, jid):
        super(MemAgentPresenceProtocol, self).__init__()
        self.jid = jid.full()

    def connectionMade(self):
        self.connected = time.time()
        self.update_presence()

        # Register presence?

    def update_presence(self):
        self.available(None, None, {None: "Agenting."})

    def availableReceived(self, entity, show=None, statuses=None, priority=0):
        log.msg("Available from %s (%s, %s, pri=%s)" % (
            entity.full(), show, statuses, priority))

    def unavailableReceived(self, entity, statuses=None):
        log.msg("Unavailable from %s" % entity.full())

    def subscribedReceived(self, entity):
        log.msg("Subscribe received from %s" % (entity.userhost()))

    def unsubscribedReceived(self, entity):
        log.msg("Unsubscribed received from %s" % (entity.userhost()))
        self.unsubscribe(entity)
        self.unsubscribed(entity)

    def subscribeReceived(self, entity):
        log.msg("Subscribe received from %s" % (entity.userhost()))
        self.subscribe(entity)
        self.subscribed(entity)

    def unsubscribeReceived(self, entity):
        log.msg("Unsubscribe received from %s" % (entity.userhost()))
        self.unsubscribe(entity)
        self.unsubscribed(entity)

