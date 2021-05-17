#!/bin/bash

# This was needed to enable the serial port on our Jetson. May be useful

echo "Setting Up Serial Port"
sudo chmod a+rw /dev/ttyACM0

exit 0
