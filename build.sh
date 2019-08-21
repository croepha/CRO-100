
set -e

junk/arduino-1.8.9/arduino \
 --board arduino:avr:mega \
 --verify flashing_sketch/flashing_sketch.ino \
 --port /dev/ttyUSB1


#clang++ rom0.cpp -o rom0.exec
#./rom0.exec

#clang++ port_sim.cpp -o port_sim.exec
#./port_sim.exec

