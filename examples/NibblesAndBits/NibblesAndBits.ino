/*
 * Example code for Eep template class
 * Copyright (C) 2015 Christopher C. Evich
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#define EEPDEBUG
// This allows allocating and initializing EEPROM manually.
// The resulting .eep file can then be burned seperately.
// e.g. avrdude -p m328p -c usbtiny -D -P usb
//      -U eeprom:w:/tmp/build5073703394233512791.tmp/NibblesAndBits.cpp.eep:i
#define NOEEPROMINIT

#include <Eep.h>

enum Stuff { StuffA, StuffB, StuffC };

struct Junk {
    bool small;
    int64_t large;
};

union Things {
    Stuff nibbles;
    Junk bits;
};

struct Doodads {
    Stuff foo;
    Junk bar;
    Things baz;
};

const Eep::Magic stuffMagic = 0x12345678;
const Eep::Magic junkMagic = 0x87654321;
const Eep::Magic doodadsMagic = 0x42424242;

typedef Eep::Eep<Stuff, 1, stuffMagic> StuffEep;
typedef Eep::Eep<Junk, 1, junkMagic> JunkEep;
typedef Eep::Eep<Doodads, 1, doodadsMagic> DoodadsEep;

template<> Eep::Block<Stuff> StuffEep::block_eeprom EEMEM = {
    .magic = stuffMagic,
    .version = 1,
    .data = StuffB,
    // This re-calculation of CRC value when found on load.
    .crc = crcmagic
};

template<> Eep::Block<Junk> JunkEep::block_eeprom EEMEM = {
    .magic = junkMagic,
    .version = 1,
    .data = {false, 0x42},
    // This will never be returned as an actual CRC value.
    .crc = crcmagic
};

template<> Eep::Block<Doodads> DoodadsEep::block_eeprom EEMEM = {
    .magic = doodadsMagic,
    .version = 1,
    .data = {
        .foo = StuffC,
        .bar = {true, -1234567890LL},
        .baz = {.nibbles = StuffC}
    },
    .crc = crcmagic
};

void setup(void) {
    Serial.begin(115200);
    delay(1);  // Serial client sometimes needs extra time to initialize
    Serial.println(F("\nSetup()"));

    // Data must be flashed separately, no defaults provided!
    StuffEep stuffEep;
    JunkEep junkEep;
    DoodadsEep doodadsEep;

    // Data is still validated however.
    Stuff* stuff = stuffEep.data();
    Junk* junk = junkEep.data();
    Doodads* doodads = doodadsEep.data();
    if (stuff && junk && doodads)
        Serial.println(F("Yay, you flashed them all!"));
    else
        Serial.println(F("Eeprom content is invalid!"));
}

void loop(void) {
    DoodadsEep doodadsEep;
    const Doodads* doodads = doodadsEep.eeprom_data();
    Serial.print(F("The offset in eeprom of Doodad's data is:"));
    Serial.println(reinterpret_cast<uintptr_t>(doodads), HEX);
    while (true);
}
