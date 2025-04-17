#!/bin/bash
# Export the display for X11 applications
export DISPLAY=:0
# Set PULSE_SERVER to use the Windows host's PulseAudio server
export PULSE_SERVER=tcp:$(grep nameserver /etc/resolv.conf | awk '{print $2}');
echo "PulseAudio server set to $PULSE_SERVER"
