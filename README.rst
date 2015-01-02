Eep
===

Templated class for initializing/containing EEPROM data in user-defined structure

The ATMega chip in most Arduino's have some amount of EEPROM storage built in.  General
libraries are available for reading and writing various types of data to this storage.
However none address the issue of addressing the data safely, or initializing it with
default values when it's empty.  This especially becomes a problem if/when you have
multiple data structures located in EEProm utilized for different purposes at various
times.  With this library, you can have as many structures as you want and none
of them will clash as long as they're wrapped properly.

This library solves the addressing problem by utilizing the ``avr-libc`` built-in ``EEMEM``
macro. This macro marks parts of the sketch for storage in the eeprom "section" instead of
regular program section.  Consequently, it also provides a compile-time address for the
data, which can be used to enforce type-safe operations on it.  In this way, addressing
is automatic and completely transparent to the sketch.

Lastly, the library provides a versioning mechanism along with carefully crafted
write-ordering to validate the data and detect power-loss on write conditions.  On reset,
if the data is found invalid, default values are automatically loaded, and the fact is
signaled back to the sketch (in case it's important).

Later versions of the Arduino IDE (``1.0.5``) will automatically extract the eeprom
section into an ``.eep`` file.  Turn on verbose-compile in preferences to locate this file
at build time.  It is possible to upload only the eeprom contents if your platform does
**not** utilize the ``optiboot`` bootloader (unfortunately that includes Arduino UNO
:cry:)  Otherwise, default values are stored in program-memory to save RAM, or can be
bypassed (with alternate constructor) if loading defaults is not important.

Example Usage
=============

(from ``examples/HelloWorld``)

::

    #include <Eep.h>

    class EepromData {
        public:
        char h[7] = "hello";
        char w[7] = "world";
        uint32_t answer = 42;
    };

    const EepromData defaults PROGMEM;

    typedef Eep::Eep<EepromData, 1> Eep_type;

    Eep_type::Block eemem EEMEM;

    EepromData* eepromData = NULL;

    void setup(void) {
        Eep_type eep(defaults, &eemem);
        eepromData = eep.data();
        if (eepromData) {  // contents valid
            ...
        } else {  // contents reset to defaults
            ...
        }
    }
    ```

Copyright (C) 2014 Christopher C. Evich
