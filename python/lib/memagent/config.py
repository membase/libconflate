#!/usr/bin/env python
"""
Configuration for memagent.
"""

import ConfigParser
import commands

CONF=ConfigParser.ConfigParser()
CONF.read('agent.conf')
VERSION=commands.getoutput("git describe").strip()
ADMINS=CONF.get("general", "admins").split(' ')
