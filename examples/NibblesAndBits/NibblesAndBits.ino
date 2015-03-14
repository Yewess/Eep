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

//#define EEPDEBUG

#include <Eep.h>

struct Junk {
    bool small;
    int32_t large;
    static const Eep::Magic junkMagic;  // does not occupy storage space
    static const Eep::Version junkVersion;  // this either
};
const Eep::Magic Junk::junkMagic = 0x87654321UL;
const Eep::Version Junk::junkVersion = 1;

enum Stuff { StuffA, StuffB, StuffC };

union Things {
    Stuff nibbles;
    Junk bits;
};

struct Doodads {
    Stuff foo;
    Junk bar;
    Things baz;
};

// Optionally, you can produce a <name>_defaults constant in PROGMEM and
// EEMEM initialization from named member initializers.
NewEepDefaults(DoodadsEep, // Will define & declare DoodadsEep_defaults
               Doodads,
               Junk::junkVersion,
               ~Junk::junkMagic,
               .foo = StuffA,  // Initialize Doodads members
               .bar = {.small = true, .large = -123456789L},
               .baz = {.nibbles = StuffC}
);


// Alternativly, don't store defaults, only define EEMEM.  This
// doesn't even need to be in the same file!
//
//                                                             bits
//                                                     nibbles   |
//                                                        |      |
//                                                        v      v
NewEep(JunkEep, Junk, Junk::junkVersion, Junk::junkMagic, false, 0x42);

void setup(void) {
    Serial.begin(115200);
    delay(1);  // Serial client sometimes needs extra time to initialize
    Serial.println(F("\nSetup()"));

    // Data must be flashed separately, no defaults provided!
    JunkEep junkEep;

    // This one will automaticall load from defaults if EEMEM is invalid
    DoodadsEep doodadsEep(DoodadsEep_defaults);

    // Data is validated on access, NULL returned if/when invalid.
    Junk* junk = junkEep.data();
    Doodads* doodads = doodadsEep.data();
    if (junk)
        Serial.println(F("Yay, you flashed 'junk'"));
    if (doodads && doodads->bar.large == -123456789LL)
        Serial.println(F("Successfully loaded 'doodads'"));
}

void loop(void) {
    DoodadsEep doodadsEep;
    uintptr_t doodads_offset =
        reinterpret_cast<uintptr_t>(doodadsEep.eeprom_data());
    size_t doodads_size = sizeof(Doodads);
    Serial.print(F("The offset in eeprom of Doodad's data is "));
    Serial.print(doodads_offset, HEX);
    Serial.print(F(" (0-based) and "));
    Serial.print(doodads_size);
    Serial.println(F(" bytes long."));

    size_t doodads_block_size = sizeof(Eep::Block<Doodads>);
    Serial.print(F("doodadsEep size is: "));
    Serial.print(doodads_block_size);
    Serial.println(F(" bytes."));

    // For testing/demonstration purposes, invalidate doodads data
    // to show corruption detection and defaults loaded on next reset.
    for (uint16_t bite = 0; bite < doodads_block_size; bite++) {
        eeprom_update_byte(
            reinterpret_cast<uint8_t*>(bite + doodads_offset),
            0x00);
    }
    while (true);
}
