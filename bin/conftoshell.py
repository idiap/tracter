#!/usr/bin/python
#
# Convert config (ini) file to shell variables
# Phil Garner, July 2010
#
import ConfigParser
import sys

# Read command line
ini = ''
format = ''
for arg in sys.argv[1:]:
    if   arg == '-set':
        format = 'set'
    elif arg == '-export':
        format = 'export'
    elif arg == '-setenv':
        format = 'setenv'
    else:
        ini = arg

# Set up the inifile
config = ConfigParser.RawConfigParser()
config.read(ini)

# Dump it in a different form
for s in config.sections():
    for i in config.items(s):
        if   format == 'set':
            print 'set %s_%s = %s' % (s, i[0], i[1])
        elif format == 'setenv':
            print 'setenv %s_%s %s' % (s, i[0], i[1])
        elif format == 'export':
            print 'export %s_%s=%s' % (s, i[0], i[1])
        else:
            print '%s_%s=%s' % (s, i[0], i[1])
