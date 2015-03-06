/*
 * Templated class for user-defined structures in auto-allocated EEPROM
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
#include <util/crc16.h>

namespace Eep {

/*
 * Macros
 */

// (Advanced/Optional) Define NOEEPROMINIT to not
// initialize static, empty EEPROM values.  Then caller
// is responsible for EEPROM initialization.  The output
// can then be burned to the EEPROM separately.
//#define NOEEPROMINIT

// Define this to enable library debugging
#ifndef EEPDEBUG
#define H()
#define D(...)
#define DL(...)
#else
#define D(...) Serial.print(__VA_ARGS__)
#define DL(...) Serial.println(__VA_ARGS__)
#define H() D(F("Eep @0x"));\
                    D(reinterpret_cast<uintptr_t>(&block_eeprom), HEX);\
                    D(':')
#endif // EEPDEBUG

#ifndef DEFMAGIC
#define DEFMAGIC 0xEFBEADDE  // DEADBEEF!
#endif // DEFMAGIC

#ifndef DEFVERSION
#define DEFVERSION 0
#endif // DEFVERSION

#ifndef DEFCRC
#define DEFCRC 0
#endif // DEFCRC

/*
 * Basic typedefs for reference
 */
typedef uint32_t Magic;
typedef uint8_t Version;
typedef uint32_t Crc;

/*
 * Forward declarations
 */

// This is the type intended for use in calling code
template<typename Data,
         Version version_value = DEFVERSION,
         Magic magic_value = DEFMAGIC> class Eep;

// Normal use shouldn't ever need to touch this
template<typename Data> struct Block;

/*
 * Declarations
 */

template<typename Data, Version version_value, Magic magic_value>
class Eep {
  public:
    const size_t block_size;  // for convenience

    // Initialize || re-initialize if EEPROM content is invalid
    // loads into static buffer from PROGMEM reference
    Eep(const Data& data_progmem);

    // Initialize, DO NOT re-initialize, fail if EEPROM content is invalid
    // loads into static buffer
    Eep(void);

    // store static buffer content in EEPROM if formated.
    // Returns true on success, false on failure
    bool save(void);

    // store data content in EEPROM if formated.
    // Returns true on success, false on failure
    bool save(const Data& data);

    // Forcibly re-format content in EEPROM, return true on success
    bool format(void);

    // Format with some existing data, returns true on success
    bool format(const Data& data);

    // Return address to static buffer previously loaded, NULL if invalid
    Data* data(void);

    // Return address to static buffer holding loaded data, NULL if invalid
    Data* load(void);

#ifdef EEPDEBUG
    // Show actual contents for instance
    void dump(void);
#else
    void dump(void) {};  // no op
#endif // EEPDEBUG

  private:
    static Block<Data> block_eeprom;  // Address in EEProm
    Block<Data> buffer;  // local copy of data

    // Returns true if magic and version values match expected values
    bool valid(const Block<Data>& check);
    bool valid(void);

    // Load w/o validating
    void load_unvalidated(Block<Data>& dest);
    void load_unvalidated(void);

#ifdef EEPDEBUG
    void ddump(const Data& data_progmem);
#endif // EEPDEBUG
};

template<typename Data>
struct Block {
    Magic magic;
    Version version;
    Data data;
    Crc crc;

    // Calculate and return correct crc value
    Crc make_crc(const Block& block) const;
    Crc make_crc(void) const;

    // Return True if crc value is correct
    bool crc_valid(const Block& block) const;
    bool crc_valid(void) const;
};

/*
 * Static Definitions
 */

#ifndef NOEEPROMINIT
template<typename Data, Version version_value, Magic magic_value>
Block<Data> Eep<Data, version_value, magic_value>::block_eeprom EEMEM;
#endif  // NOEEPROMINIT

/*
 * Implementations
 */

template<typename Data>
Crc Block<Data>::make_crc(const Block& block) const {
    Block compute;
    memcpy(&compute, &block, sizeof(compute));
    compute.crc = DEFCRC;
    uint8_t* byte_addr = reinterpret_cast<uint8_t*>(&compute);
    Crc crc = (Crc) - 1; // all 1's
    for (uintptr_t offset = 0; offset < sizeof(compute); offset++) {
        uint8_t* data = byte_addr + offset;
        crc = _crc16_update(crc, *data);
    }
    return crc;
}

template<typename Data>
Crc Block<Data>::make_crc(void) const {
    return this->make_crc(*this);
}

template<typename Data>
bool Block<Data>::crc_valid(const Block& block) const {
    return block.crc == make_crc(block);
}

template<typename Data>
bool Block<Data>::crc_valid(void) const {
    return this->crc_valid(*this);
}


template<typename Data, Version version_value, Magic magic_value>
Eep<Data, version_value, magic_value>::Eep(const Data& data_progmem) :
    block_size(sizeof(Block<Data>)) {
    if (!load()) {
        H();
        DL(F("\tResetting to defaults"));
#ifdef EEPDEBUG
        ddump(data_progmem);
#endif // EEPDEBUG
        memcpy_P(&this->buffer.data, &data_progmem, sizeof(data_progmem));
        if (!format()) {
            H();
            DL(F("\tFormatting Failed!"));
            return;
        }
    }
    H();
    DL(F("\tInitialized"));
    this->dump();
}

template<typename Data, Version version_value, Magic magic_value>
Eep<Data, version_value, magic_value>::Eep(void) :
    block_size(sizeof(Block<Data>)) {
    if (!load()) {
        H();
        DL(F("\tInvalid data, NOT resetting."));
        return;
    }
    H();
    DL(F("\tInitialized"));
    this->dump();
}

template<typename Data, Version version_value, Magic magic_value>
void Eep<Data, version_value, magic_value>::load_unvalidated(Block<Data>& dest) {
    H();
    D(F("\tLoading "));
    D(this->block_size, DEC);
    D(F(" bytes from @ 0x"));
    D(reinterpret_cast<uintptr_t>(&this->block_eeprom), HEX);
    D(F(" to @"));
    DL(reinterpret_cast<uintptr_t>(&dest), HEX);
    eeprom_busy_wait();
    eeprom_read_block(&dest,  // local memory
                      &this->block_eeprom, // EEProm address
                      this->block_size);
}

template<typename Data, Version version_value, Magic magic_value>
void Eep<Data, version_value, magic_value>::load_unvalidated(void) {
    load_unvalidated(this->buffer);
}

template<typename Data, Version version_value, Magic magic_value>
bool Eep<Data, version_value, magic_value>::valid(const Block<Data>& check) {
    H();
    D(F("\tContents @ 0x"));
    D(reinterpret_cast<uintptr_t>(&check), HEX);
    bool valid_magic = check.magic == magic_value;
    bool valid_version = check.version == version_value;
    bool crc_valid = check.crc_valid();
    if (valid_magic && valid_version && crc_valid)
        D(F(" valid"));
    else
        D(F(" invalid"));
    D(F(" magic: 0x"));
    D(check.magic, HEX);
    D(F(" ver: "));
    D(check.version, DEC);
    D(F(" CRC: 0x"));
    DL(check.crc, HEX);
    if (valid_magic && valid_version && crc_valid)
        return true;
    else {
        H();
        D(F("\tExpecting: "));
        Block<Data> expected;
        expected.magic = magic_value;
        expected.version = version_value;
        // Assume the data is correct
        memcpy(&expected.data, &check.data, sizeof(expected.data));
        expected.crc = expected.make_crc();
        D(F(" magic: 0x"));
        D(expected.magic, HEX);
        D(F(" ver: "));
        D(expected.version, DEC);
        D(F(" CRC: 0x"));
        DL(expected.crc, HEX);
        return false;
    }
}

template<typename Data, Version version_value, Magic magic_value>
bool Eep<Data, version_value, magic_value>::valid() {
    return this->valid(this->buffer);
}

template<typename Data, Version version_value, Magic magic_value>
bool Eep<Data, version_value, magic_value>::save(const Data& data) {
    H();
    DL(F("\tVerifying format"));
    this->dump();
    Block<Data> in_eeprom;  // Don't overwrite static buffer
    this->load_unvalidated(in_eeprom);
    if (this->valid(in_eeprom)) {
        Block<Data> to_save;
        H();
        D(F("\tSaving "));
        // Make double-sure these are correct
        to_save.magic = magic_value;
        to_save.version = version_value;
        memcpy(&to_save.data, &data, sizeof(to_save.data));
        to_save.crc = to_save.make_crc();
        D(F("CRC: 0x"));
        DL(to_save.crc, HEX);
        // protect against power-fail during save
        to_save.magic = ~magic_value; // always invalid value
        eeprom_busy_wait();
        eeprom_update_block(&to_save, // local memory
                            &this->block_eeprom, // EEProm address
                            this->block_size);
        // No-power-fail, write correct magic
        to_save.magic = magic_value;
        eeprom_update_block(&to_save, // local memory
                            &this->block_eeprom, // EEProm address
                            this->block_size);
        H();
        D(F("\tWrote "));
        D(this->block_size, DEC);
        DL(F(" bytes"));
        this->dump();
        return this->valid(to_save);
    } else {  // Formatting in eeprom is wrong
        H();
        DL(F("\tSaving failed, EEPROM data format invalid."));
        return false;
    }
}

template<typename Data, Version version_value, Magic magic_value>
bool Eep<Data, version_value, magic_value>::save(void) {
    return this->save(this->buffer.data);
}

template<typename Data, Version version_value, Magic magic_value>
bool Eep<Data, version_value, magic_value>::format(const Data& data) {
    H();
    DL(F("\tFormatting EEPROM..."));
    Block<Data> to_format;
    to_format.magic = magic_value;
    to_format.version = version_value;
    memcpy(&to_format.data, &data, sizeof(to_format.data));
    to_format.crc = to_format.make_crc();
    // lock in case of power-failure during write
    to_format.magic = ~magic_value; // lock value
    eeprom_busy_wait();
    eeprom_update_block(&to_format, // local memory
                        &this->block_eeprom,  // EEProm address
                        this->block_size);
    // No-power-fail, write correct magic
    to_format.magic = magic_value; // lock value
    eeprom_update_block(&to_format, // local memory
                        &this->block_eeprom,  // EEProm address
                        this->block_size);
    this->dump();
    return this->valid(to_format);
}

template<typename Data, Version version_value, Magic magic_value>
bool Eep<Data, version_value, magic_value>::format() {
    return this->format(this->buffer.data);
}

template<typename Data, Version version_value, Magic magic_value>
Data* Eep<Data, version_value, magic_value>::data(void) {
    H();
    D(F("\tProviding Static buffer @ 0x"));
    DL(reinterpret_cast<uintptr_t>(&this->buffer), HEX);
    // Check stored validity, buffer may have changed
    Block<Data> check;
    this->load_unvalidated(check);
    if (this->valid(check))
        return &this->buffer.data;
    else
        return reinterpret_cast<Data*>(NULL);
}

template<typename Data, Version version_value, Magic magic_value>
Data* Eep<Data, version_value, magic_value>::load(void) {
    H();
    D(F("\tStatic buffer @"));
    DL(reinterpret_cast<uintptr_t>(&this->buffer), HEX);
    this->dump();
    this->load_unvalidated();
    if (this->valid())
        return &this->buffer.data;
    else
        return reinterpret_cast<Data*>(NULL);
}

#ifdef EEPDEBUG
template<typename Data, Version version_value, Magic magic_value>
void Eep<Data, version_value, magic_value>::dump(void) {
    H();
    D(F("\tDumping @ 0x"));
    uintptr_t start_block_eeprom = reinterpret_cast<uintptr_t>(&this->block_eeprom);
    D(start_block_eeprom, HEX);
    D(F(" ("));
    D(this->block_size);
    D(F(" bytes):"));
    if (this->block_size > 2048 || this->block_size < 8) {
        D(F("Bad block size: "));
        D(this->block_size);
        DL(F(" bytes"));
        while(true);  // loop forever
    }
    for (uint16_t offset = 0; offset < this->block_size; offset += 8) {
        DL();
        H();
        D(F("\t@ 0x"));
        D(start_block_eeprom + offset, HEX);
        D(F(":\t"));  // tab avoids need to pad value
        for (uint8_t bite = 0; bite < 8; bite++)
            if (offset + bite > block_size)
                break;
            else {
                uint16_t complete_block_eeprom = start_block_eeprom + offset + bite;
                D(F("0x"));
                uint8_t value = eeprom_read_byte(
                                    reinterpret_cast<uint8_t*>(complete_block_eeprom));
                if (value < 0x10) // Pad
                    D(F("0"));
                D(value, HEX);
                D(F(" "));
            }
    }
    DL();
}

template<typename Data, Version version_value, Magic magic_value>
void Eep<Data, version_value, magic_value>::ddump(const Data& data_progmem) {
    static uintptr_t start_address = reinterpret_cast<uintptr_t>(&data_progmem);
    H();
    D(F("\tDumping defaults ("));
    D(sizeof(data_progmem));
    D(F(" bytes) @ 0x"));
    D(start_address, HEX);
    D(F(":"));
    for (uint16_t offset = 0; offset < sizeof(data_progmem); offset += 8) {
        DL();
        H();
        D(F("\t@ 0x"));
        D(start_address + offset, HEX);
        D(F(":\t"));  // tab avoids need to pad value
        for (uint8_t bite = 0; bite < 8; bite++)
            if (offset + bite > sizeof(data_progmem))
                break;
            else {
                uint16_t complete_address = start_address + offset + bite;
                D(F("0x"));
                uint8_t value = pgm_read_byte(
                                    reinterpret_cast<uint8_t*>(complete_address));
                if (value < 0x10) // Pad
                    D(F("0"));
                D(value, HEX);
                D(F(" "));
            }
    }
    DL();
}
#endif // EEPDEBUG

}; // namespace Eep
#endif  // EEP_H
