Eep
===

Templated class for auto-allocated EEPROM user-defined structure

The ATMega chip in most Arduino's have some amount of EEPROM storage built in.  General
libraries are available for reading and writing various types of data to this storage.
However none take care of automatic addressing or detecting corruption during power-fail on save condition.  With this library, you can have as many structures as you want and none
of them will clash.

This library solves the addressing problem by utilizing the ``avr-libc`` built-in ``EEMEM``
macro. This macro sets a special attrubute that marks parts of the sketch for storage
in the eeprom "section" instead of regular program section.  This assigns a compile-time
address in in EEPROM space.

The library also automaticaly wraps data with identifying information along
with a CRC.  When coupled with carefully crafted write-ordering, these provide
assurance that undetectable data corruption is extremely unlikely.

Finally, if you want to save space, later versions of the Arduino IDE (``1.0.5``)
will emit the eeprom section into as ``.eep`` file.  Turn on verbose-compile in
preferences to locate this file.  This data may then be flashed separately
using an ISP programmer.  Note, the ``optiboot`` on Arduino UNO doesn't allow
flashing with the bootloader, you must use an ISP.

Example Usage
=============

(from ``examples/HelloWorld``)

::

    #include <Eep.h>

    struct Data {
        char h[6];
        char w[6];
        uint32_t answer;
    };

    const Data dataDefaults PROGMEM = {
        .h = "hello",
        .w = "world",
        .answer = 42U
    };

    typedef Eep::Eep<Data> DataEep;

    void setup(void) {
        Serial.begin(115200);
        DataEep eep(dataDefaults);
        Data* data = eep.data();
        if (data) {
            Serial.println(F("EEPRom content is valid"));
            Serial.print(F("The data is: "));
            Serial.print(data->h);
            Serial.print(F(" "));
            Serial.print(data->w);
            Serial.print(F(" answer is: "));
            Serial.println(data->answer);
        } else
            Serial.println(F("Eeprom content is invalid!"));
    }

    void loop(void) {
    }

Copyright (C) 2014 Christopher C. Evich
