# Si5351mcu Changelog File #

## v0.3 (June 14, 2017) ##

* Feature: the lib now handle the include and start of the I2C (Wire) library internally via the init procedures
* Added a new generic init() procedure to handle the default parameters
* The init() function is required from now on (MANDATORY)
* Fixed the way we handled the base xtal and the correction factor

## v0.2rc (April 23, 2017) ##

* Added power level support for each output independently, watch out!: setting the power level will enable the output.
* Set default power to the lowest one (2mA) from the maximun possible (8mA).
* Fixed the need for a reset after each correction, it does it now automatically
* Added a init function to pass a custom xtal
* Modified the example to show no need for a init unless you use a different xtal
* Improved the keywords.txt file to improve edition on the Arduino IDE
* Included a "features" section on the README.md

## v0.1rc (April 20, 2017) ##

* Added enable(), disable() and off() functions.
* Added support for handling all the three outputs of the Si5351A, (CLK1 & CLK2 are mutually-exclusive)
* Updated the example with the new functions.
* Improved library logic by reusing and optimizing functions.
* Improved the documentation and comments (lib code, README and example)
* The compiled code is slightly smaller now (~1% on an ATMega328p)
* Added Changelog and version files.
* Extensive tests made to validate every function.

## Initial Release, v0.0 (April 9, 2017) ##

* Basic functionality.
