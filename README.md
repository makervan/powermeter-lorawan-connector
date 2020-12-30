# DC Power Meter LoRaWAN Connector Node
LoRaWAN node to send data from the GC90/GC91 DC power meters via TTN. Based on an Arduino Pro Mini and the RFM95 LoRa module.

## Hardware
* Arduino Pro Mini 3.3V 8Mhz
* RFM95/96 LoRaWAN module
* [uFL socket](https://www.digikey.de/product-detail/en/linx-technologies-inc/CONUFL001-SMD-T/CONUFL001-SMD-TCT-ND/7427733) or edge mount SMA socket
* 2 BSS138 MOSFETs (Q1/Q2) and 2x 10k resistors (R3/R4, 0805) for the level shifter
* USB A socket
* 2 resistors (R1/R2, 0805) to form a voltage divider for measuring the input voltage (optional, values depending on voltage range)
* Groove connector (optional)

## Payload decoder
If you use the TTN network, you can use the following payload decoder:

```
function Decoder(bytes, port) {
  // Decode an uplink message from a buffer
  // (array) of bytes to an object of fields.
  var decoded = {};
  decoded.voltage = (bytes[2] << 8 | bytes[1]) / 100;
  decoded.current = (bytes[4] << 8 | bytes[3]) / 1000;
  decoded.usage = (bytes[8] << 24 | bytes[7] << 16 | bytes[6] << 8 | bytes[5]) / 1000;
  decoded.cum_usage = (bytes[12] << 24 | bytes[11] << 16 | bytes[10] << 8 | bytes[9]) / 1000;

  return decoded;
}
```

## License
* Hardware is based off a design by [Doug Larue](https://github.com/dlarue) with additions by me, Severin Schols. It's licensed under Creative Commons Attribution-ShareAlike 4.0 International License (CC BY-SA 4.0)
* Software (firmware and the payload decoder) are licensed under MIT License
