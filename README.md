# DC Power Meter LoRaWAN Connector Node
LoRaWAN node to send data from the GC90/GC91 DC power meters via TTN. Based on an Arduino Pro Mini and the RFM95 LoRa module.

## Hardware
* Arduino Pro Mini 3.3V 8Mhz
* RFM95/96 LoRaWAN module
* uFL socket or edge mount SMA socket
* 2 BSS138 MOSFETs (Q1/Q2) and 2x 10k resistors (R3/R4, 0805) for the level shifter
* USB A socket
* 2 resistors (R1/R2, 0805) to form a voltage divider for measuring the input voltage (optional)
* Groove connector (optional)

## License
* Hardware is based off a design by [Doug Larue](https://github.com/dlarue) with additions by me, Severin Schols. It's licensed under Creative Commons Attribution-ShareAlike 4.0 International License (CC BY-SA 4.0)
