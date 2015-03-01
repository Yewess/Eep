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

// Define to see extra juicy details of what's happening
#define EEPDEBUG

#include <Eep.h>
#include "HelloWorld.h"

void setup(void) {
    Serial.begin(115200);
    delay(1);  // Serial client sometimes needs extra time to initialize
    Serial.println(F("\nSetup()"));

    // DO NOT USE eemem any other way! (address is relative to .eeprom section)
    Eep_type eep(defaults, &eemem);

    #ifdef EEPDEBUG
    // Serial.print() out contents 8 bytes at a time
    eep.dump();
    #endif //EEPDEBUG

    // EEPRom contents was loaded during initialization (above)
    // no need to use eep.load() again, just grab a static buffer reference.
    EepromData* data = eep.data();
    if (data) {  // NULL pointer signals load/reset failure
        Serial.println(F("EEPRom content is valid"));
        Serial.print(F("The data is: "));
        Serial.print(data->h);
        Serial.print(F(" "));
        Serial.print(data->w);
        Serial.print(F(" answer is: "));
        Serial.println(data->answer);
        // Since this sketch defined EepromData, it's
        // safe to modify static buffer directly.
        if (data->answer == 24) {
            data->answer = 42;
            memcpy(&data->h, "hello", 7);
            memcpy(&data->w, "world", 7);
            // eep.save() operates semi-atomicly, a power-loss during
            // save will cause defaults to be restored.
            if (eep.save())
                Serial.println(F("Data updated"));
            else
                Serial.println(F("Update failed"));
        } else {
            // Possible to save from local data instead of static buffer.
            EepromData newdata;
            newdata.answer = 24;
            memcpy(&newdata.h, "world", 7);
            memcpy(&newdata.w, "hello", 7);
            if (eep.save(&newdata))
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
    Eep_type eep(&eemem);  // DO NOT USE eemem any other way!

    #ifdef EEPDEBUG
    eep.dump();
    #endif //EEPDEBUG

    EepromData* data = eep.data();  // Grab static buffer reference
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
