#!/bin/bash
# set slave (hr20's) address; must be >0
ADDR=10

#-DRFM_TUNING=1 \
make clean
make RFM="\
-DRFM=1 \
-DRFM_FREQ=868 \
-DRFM_BAUD_RATE=9600 \
-DSECURITY_KEY_0=0x01 \
-DSECURITY_KEY_1=0x23 \
-DSECURITY_KEY_2=0x45 \
-DSECURITY_KEY_3=0x67 \
-DSECURITY_KEY_4=0x89 \
-DSECURITY_KEY_5=0x01 \
-DSECURITY_KEY_6=0x23 \
-DSECURITY_KEY_7=0x45 \
-DRFM_DEVICE_ADDRESS=${ADDR} \
-DRFM_WIRE_MARIOJTAG=1" \
REMOTE_SETTING_ONLY=-DREMOTE_SETTING_ONLY=0 \
MOTOR_COMPENSATE_BATTERY=-DMOTOR_COMPENSATE_BATTERY=1

