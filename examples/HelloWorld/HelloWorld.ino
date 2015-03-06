/*
 * Example code for Eep template class
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


// Define to see extra juicy details of what's happening
#define EEPDEBUG

#include <Eep.h>

// Arbitrary type of data to persist w/in EEProm. Make as many as you want.
// They must be "standard layout" class, struct, or aggregate.
// No fancy stuff like constructors, though member-functions are okay.
// There is a small overhead (handful of bytes) per data structure, used
// to frame the data and protect against corruption during power failure.
struct Data {
    char h[6];
    char w[6];
    uint32_t answer;
};

// Default values to use when storage is uninitialized
// or corrupt.
const Data dataDefaults PROGMEM = {
    .h = "hello",
    .w = "world",
    .answer = 42U
};

// Optional.  The data can be wrapped with a unique identifier
// to help distinguish it from other data structures and protect
// it during write operations.
const Eep::Magic dataMagic = 0xBAADF00D;

// Optional.  The data will be wrapped with a version number to
// indenfify changes in the format over time.  This should be
// changed any time the data structure format is changed.
const Eep::Version dataVersion = 1;

// This just makes it easier to reference below
typedef Eep::Eep<Data, dataVersion, dataMagic> DataEep;

void setup(void) {
    Serial.begin(115200);
    delay(1);  // Serial client sometimes needs extra time to initialize
    Serial.println(F("\nSetup()"));

#ifdef EEPDEBUG
    // For testing/demonstration purposes, start with factory-state
    // eeprom, black out twice-times the Data size (just to be safe)
    Serial.print(F("Overwriting 2 x "));
    size_t size = sizeof(Eep::Block<Data>);
    Serial.print(size);
    Serial.println(F(" bytes in EEPROM..."));
    for (uint16_t bite = 0; bite < size * 2; bite++)
        eeprom_update_byte(reinterpret_cast<uint8_t*>(bite), 0x00);
#endif //EEPDEBUG

    // Initialize and reset to default values if invalid/uninitialized
    DataEep eep(dataDefaults);

    // EEPRom contents loaded & validated during initialization (above).
    // Re-validate and obtain pointer to static data buffer
    Data* data = eep.data();
    if (data) {  // NULL pointer signals load/reset failure
        Serial.println(F("EEPRom content is valid"));
        Serial.print(F("The data is: "));
        Serial.print(data->h);
        Serial.print(F(" "));
        Serial.print(data->w);
        Serial.print(F(" answer is: "));
        Serial.println(data->answer);
        // It's safe to modify static buffer directly.
        if (data->answer != 42) {
            // Change data inside static buffer
            data->answer = 42;
            memcpy(&data->h, "hello", 6); // 5 chars + 1 null
            memcpy(&data->w, "world", 6);
            // eep.save() operates atomicly, power-loss during
            // save will not cause corrupt data.
            if (eep.save())  // save from static bufffer
                Serial.println(F("Data updated"));
            else
                Serial.println(F("Update failed"));
        } else {
            // Equally safe to maintain a separate copy
            Data newdata;
            newdata.answer = 24;
            memcpy(&newdata.h, "world", 6);
            memcpy(&newdata.w, "hello", 6);
            if (eep.save(newdata))
                Serial.println(F("Data updated"));
            else
                Serial.println(F("Update failed"));
        }
    } else
        // Because defaults are automatically restored,
        // execution should never make it here.
        Serial.println(F("Eeprom content is invalid!"));
}

void loop(void) {
    // This alternate constructor will NOT restore defaults automaticaly.
    DataEep eep;  // Doesn't reset to defaults

    Data* data = eep.data();  // Grab static buffer reference
    if (data) {
        Serial.println(F("EEPRom content is still valid"));
        Serial.print(F("The data is: "));
        Serial.print(data->h);
        Serial.print(F(" "));
        Serial.print(data->w);
        Serial.print(F(" answer is: "));
        Serial.println(data->answer);
    } else
        // Alternate constructor can be used to detect power-loss during save
        // condition.
        Serial.println(F("Eeprom content is invalid!"));

    while (true)
        delay(1000);
}
