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

#ifndef EEP_H
#define EEP_H

#include <Arduino.h>
#include <avr/eeprom.h>
#include <stdint.h>
#include <string.h>

namespace Eep {

// Define this to enable library debugging
#ifdef EEPDEBUG
    #define D(...) Serial.print(__VA_ARGS__)
    #define DL(...) Serial.println(__VA_ARGS__)
    #define H() D(F("Eep" PFX));\
                D(reinterpret_cast<uint16_t>(address), HEX);\
                D(':')
#else
    #define H() if (false)  // NOOP, will be optimized away.
    #define D(...) if (false)
    #define DL(...) if (false)
#endif // EEPDEBUG

// Un-define this if defaults are located in RAM instead of
// PROGMEM (not recommended).
#define DEFAULTS_PROGMEM

// Define this to prefix all debugging strings
// with '0x' for all HEX digits
#ifdef EEP_HEX_PFX
#define PFX "0x"
#else
#define PFX ""
#endif //EEP_HEX_PFX

#ifndef DEFMV
    #define DEFMAGIC 0xEFBEADDE
    #define DEFVERSION 0
    #define DEFVERTYP uint8_t
    #define DEFMAGTYP uint32_t
#endif //

// Define this to allow changing the version type
#ifdef FLEXVERSION
    #define EEPTEMPLATED template <class data_type,\
                                   class version_type = DEFVERTYP,\
                                   version_type version_value = DEFVERSION,\
                                   class magic_type = DEFMAGTYP,\
                                   magic_type magic_value = DEFMAGIC>

    #define EEPTEMPLATE template <class data_type,\
                                  class version_type,\
                                  version_type version_value,\
                                  class magic_type,\
                                  magic_type magic_value>

    #define EEPNAME Eep<data_type, version_type,\
                        version_value, magic_type, magic_value>
#else
    typedef DEFVERTYP version_type;
    #define EEPTEMPLATED template <class data_type,\
                                   version_type version_value = DEFVERSION,\
                                   class magic_type = DEFMAGTYP,\
                                   magic_type magic_value = DEFMAGIC>

    #define EEPTEMPLATE template <class data_type,\
                                  version_type version_value,\
                                  class magic_type,\
                                  magic_type magic_value>

    #define EEPNAME Eep<data_type, version_value,\
                        magic_type, magic_value>
#endif //FLEXVERSION

EEPTEMPLATED
class Eep {
    public:
    typedef EEPNAME self_type;

    // Declaration public, Implementation is private
    // Allows staticly allocating w/ EEMEM macro
    class Block;
    Block* address;
    Block buffer;

    // Returns true if magic and version values match expected values
    bool valid();

    // Sets lock in data in EEPROM to sync. w/ external state
    // Returns true on success, false on failure
    bool lock(void);

    // Return true if data lock is set, false if not, or data invalid.
    bool locked(void);

    // Releases data lock, returns true on success, false on failure
    bool unlock(void);

    // store static buffer content in EEPROM
    // Returns true on success, false on failure
    bool save(void);

    // store data content in EEPROM
    // Returns true on success, false on failure
    bool save(data_type* data);

    // Return address to static buffer previously loaded, NULL if invalid
    data_type* data(void);

    // Validate && load data from EEPROM
    // Return address to static buffer holding loaded data, NULL if invalid
    data_type* load(void);

    #ifdef EEPDEBUG
    // Show actual contents @ address
    static void dump(self_type::Block* address);

    // Show actual contents for instance
    void dump(void) { self_type::dump(address); }
    #endif // EEPDEBUG

    // Initialize || re-initialize if EEPROM content is invalid
    // loads into static buffer
    Eep(const data_type& defaults, self_type::Block* eeprom_address);

    // Initialize, DO NOT re-initialize, fail if EEPROM content is invalid
    // loads into static buffer
    Eep(self_type::Block* eeprom_address);
};

EEPTEMPLATED
class EEPNAME::Block {
    private:
    friend EEPNAME;
    magic_type magic = magic_value;
    version_type version = version_value;
    data_type data;
};

/*
 * Inline implementations
 */

EEPTEMPLATE
inline bool EEPNAME::valid() {
    H();
    // static_cast makes sure optimization doesn't change const width
    if ((buffer.magic == static_cast<magic_type>(magic_value)) &&
        (buffer.version == static_cast<version_type>(version_value))) {
        D(F("\tContents valid (magic: " PFX));
        D(buffer.magic, HEX);
        D(F(" version: "));
        D(buffer.version, DEC);
        DL(F(")"));
        return true;
    } else {
        D(F("\tContents invalid (magic: " PFX));
        D(buffer.magic, HEX);
        D(F(" version: "));
        D(buffer.version, DEC);
        DL(F(")"));
        H();
        D(F("\tExpecting magic: " PFX));
        D(static_cast<magic_type>(magic_value), HEX);
        D(F(" version: "));
        DL(static_cast<version_type>(version_value), DEC);
        return false;
    }
}

EEPTEMPLATE
inline bool EEPNAME::lock(void) {
    H();
    DL(F("\tLocking"));
    // Only lock if magic/version are valid
    if (valid()) {
        buffer.magic = static_cast<magic_type>(~magic_value);
        eeprom_busy_wait();
        eeprom_update_block(&buffer.magic,                   // local memory
                            address + offsetof(Block, magic),// EEProm address
                            sizeof(magic_type));
        eeprom_busy_wait();
        H();
        DL(F("\tContents locked"));
        return true;
    }
    H();
    DL(F("\tLocking failed"));
    return false;
}

EEPTEMPLATE
inline bool EEPNAME::locked(void) {
    eeprom_busy_wait();
    eeprom_read_block(&buffer.magic,                         // local memory
                      address + offsetof(Block, magic),      // EEProm address
                      sizeof(magic_type));
    eeprom_busy_wait();
    H();
    D(F("\tRead magic: " PFX));
    D(buffer.magic, HEX);
    D(F(" - "));
    if (buffer.magic == static_cast<magic_type>(~magic_value)) {
        DL(F("Contents confirmed locked"));
        return true;
    } else if (buffer.magic == static_cast<magic_type>(magic_value)) {
        DL(F("Contents confirmed unlocked"));
        return false;
    } else {
        D(F("Read invalid magic value, expecting " PFX));
        D(static_cast<magic_type>(magic_value), HEX);
        D(F(" or " PFX));
        DL(static_cast<magic_type>(~magic_value), HEX);
        return false;  // so unlock fails
    }
}

EEPTEMPLATE
inline bool EEPNAME::unlock(void) {
    H();
    DL(F("\tUnlocking"));
    if (!locked()) {
        H();
        DL(F("\tUnlock failed"));
        return false;
    }
    magic_type correct = static_cast<magic_type>(magic_value);
    eeprom_busy_wait();
    eeprom_update_block(&correct,                            // local memory
                        address + offsetof(Block, magic),    // EEProm address
                        sizeof(magic_type));
    eeprom_busy_wait();
    H();
    DL(F("\tContents unlocked"));
    buffer.magic = correct;
    return !locked();
}

EEPTEMPLATE
inline bool EEPNAME::save(void) {
    H();
    DL(F("\tSaving"));
    if (valid()) {
        //lock();  // protect against power-fail during save
        buffer.magic = static_cast<magic_type>(~magic_value); // lock value
        // Just to be extra sure, write version also
        buffer.version = static_cast<version_type>(version_value);
        eeprom_busy_wait();
        eeprom_update_block(&buffer,                        // local memory
                            address,                        // EEProm address
                            sizeof(Block));                 // Only update data
        H();
        D(F("\tWrote "));
        D(sizeof(Block), DEC);
        DL(F(" bytes"));
        eeprom_busy_wait();
        return unlock();
    } else {
        H();
        DL(F("\tSaving failed"));
        return false;
    }
}

EEPTEMPLATE
inline bool EEPNAME::save(data_type* data) {
    if ((data) && (data != &buffer.data))
        memcpy(&buffer.data, data, sizeof(data_type));
    return save();
}

EEPTEMPLATE
inline data_type* EEPNAME::data(void) {
    if (valid())
        return &buffer.data;
    else
        return reinterpret_cast<data_type*>(0);
}

EEPTEMPLATE
inline data_type* EEPNAME::load(void) {
    H();
    D(F("\tLoading "));
    D(sizeof(Block), DEC);
    DL(F(" bytes"));
    eeprom_read_block(&buffer,                              // local memory
                      address,                              // EEProm address
                      sizeof(Block));
    return data();
}

#ifdef EEPDEBUG
EEPTEMPLATE
void EEPNAME::dump(self_type::Block* address) {
    H();
    D(F("\tDumping EEProm addresses and contents:"));
    static uint8_t* start_address = reinterpret_cast<uint8_t*>(address);
    for (uint8_t* byte_address = start_address;
         byte_address < start_address + sizeof(Block);
         byte_address++) {
        // contents should be aligned, first iteration should hit
        if (reinterpret_cast<uint16_t>(byte_address) % 8 == 0) {
            D(F("\n@" PFX));
            D(reinterpret_cast<uint16_t>(byte_address), HEX);
            D(F(":\t"));  // tab avoids need to pad value
        }
        #ifdef EEP_HEX_PFX
        D(F("0x"));
        #endif // EEP_HEX_PFX
        uint8_t value = eeprom_read_byte(byte_address);
        if (value < 0x10) // Pad
            D(F("0"));
        D(value, HEX);
        #ifdef EEP_HEX_PFX
        D(F("  "));
        #endif // EEP_HEX_PFX
    }
    DL();
}
#endif // EEPDEBUG

EEPTEMPLATE
EEPNAME::Eep(const data_type& defaults,
             self_type::Block* eeprom_address) : address(eeprom_address) {
    if (!load()) {
        H();
        D(F("\tResetting to defaults (locked magic: " PFX));
        // protect against power-failure
        buffer.magic = static_cast<magic_type>(~magic_value);
        D(buffer.magic, HEX);
        DL(F(")"));
        buffer.version = version_value;
        #ifdef DEFAULTS_PROGMEM
            memcpy_P(&buffer.data, &defaults, sizeof(data_type));
        #else
            memcpy(&buffer.data, &defaults, sizeof(data_type));
        #endif // DEFAULTS_PROGMEM
        eeprom_busy_wait();
        eeprom_update_block(&buffer,                        // local memory
                            address,                        // EEProm address
                            sizeof(Block));
        H();
        D(F("\tInitialized "));
        D(sizeof(Block), DEC);
        DL(F(" bytes"));
        eeprom_busy_wait();
        unlock();
    }
    H();
    DL(F("\tInitialized"));
}

EEPTEMPLATE
EEPNAME::Eep(self_type::Block* eeprom_address) : address(eeprom_address) {
    if (!load()) {
        H();
        DL(F("\tInvalid data, NOT resetting." PFX));
    }
    H();
    DL(F("\tInitialized"));
}

}; // namespace Eep

// Prevent conflict or use outside header
#ifndef EEP_CPP
    #undef EEPTEMPLATED
    #undef EEPTEMPLATE
    #undef EEPNAME
    #undef PFX
    #undef H
    #undef D
    #undef DL
#endif //EEP_CPP

#endif  // EEP_H
