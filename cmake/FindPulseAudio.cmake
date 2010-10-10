#
# Find PulseAudio
#
# There is also one in KDE:
# http://www.mail-archive.com/kde-buildsystem@kde.org/msg04034.html
#
# Phil Garner
# October 2010
#
include(FindPkgConfig)

pkg_check_modules(PULSEAUDIO libpulse-simple)
