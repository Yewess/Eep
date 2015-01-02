/*
 * Templated class for initializing/containing EEPROM data in user-defined structure
 * Copyright (C) 2014 Christopher C. Evich
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

#ifndef HELLOWORLD_H
#define HELLOWORLD_H

// Arbitrary class of data to persist w/in EEProm.
// Must be "standard layout" class, or have static initialized
// members.  The amount of eeprom space is the only limiting factor.
// There is only a small (5-byte) overhead per data structure, though
// this can be scaled down to as little as 2-bytes.
class EepromData {
    public:
    char h[7] = "hello";
    char w[7] = "world";
    uint32_t answer = 42;
};

const EepromData defaults PROGMEM;  // Used when contents are unset/invalid
                                    // stored in program-space.

// Increment version number whenever data format changes (above)
const uint8_t eepromVersion = 1;

// Make the customized type easier to reference from sketch
typedef Eep::Eep<EepromData, eepromVersion> Eep_type;

// Staticly allocate space in EEProm area w/ EEMEM macro from <avr/eeprom.h>
// This will cause HelloWorld.eep to be generated, and can be flashed using ISP
// programmer, or uploaded directly if using a mega or other *duino w/ eeprom
// upload support in bootloader (i.e. NOT optiboot).
Eep_type::Block eemem EEMEM;  // Allocate space in EEProm area.  NEVER dereference
                              // this address in sketch, "Bad Things Will Happen" (TM)

#endif  // HELLOWORLD_H
