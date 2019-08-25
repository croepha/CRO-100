
set -e
echo

[  ]&&{
junk/arduino-1.8.9/arduino \
 --board arduino:avr:mega \
 --verify flashing_sketch/flashing_sketch.ino \
 --port /dev/ttyUSB1
}

[ ! ]&&{
junk/arduino-1.8.9/arduino \
 --board arduino:avr:mega \
 --verify cassette_tape_emulator/cassette_tape_emulator.ino \
 --port /dev/ttyUSB1
}


mkdir -p build/

[  ]&&{

clang++ -O0 -g test_rom.cpp -o build/test_rom.exec -fsanitize=address
build/test_rom.exec
}


[ ! ]&&{

clang++ -O0 -g -Wno-writable-strings do_flash.cpp -o build/do_flash.exec -fsanitize=address
}

[ ! ]&&{
clang++ -O0 -g -Wno-writable-strings $(pwd)/create_segment_flash_data.cpp -o build/create_segment_flash_data.exec -fsanitize=address
}

[ ! ]&&{
clang++ -O0 -g -Wno-writable-strings $(pwd)/create_clipping_rom_data.cpp -o build/create_clipping_rom_data.exec -fsanitize=address
}

#clang++ rom0.cpp -o rom0.exec
#./rom0.exec

#clang++ port_sim.cpp -o port_sim.exec
#./port_sim.exec


true