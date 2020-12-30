/*
 * DC Meter consume01
 * 
 * Data format v0:
 *  0 - version + config, for now 0x00
 *  1-2 - voltage in 0.01V, unsigned int
 *  3-4 - current in mA, unsigned int
 *  5-8 - usage in mWh, unsigned long
 *  9-12 - cummulative usage in mWh, unsigned long
 * 
 */
#include <lmic.h>
#include <hal/hal.h>
#include "config.h"

void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}
void os_getDevKey (u1_t* buf) { memcpy_P(buf, APPKEY, 16);}

const unsigned TX_INTERVAL = 20;

static uint8_t mydata[20];
static osjob_t sendjob;

const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN, // hardwired to AtMega RESET
  .dio = {4,5,LMIC_UNUSED_PIN}//  .dio = {4, 5, LMIC_UNUSED_PIN},
};

const byte command[] = {0x77, 0x33, 0xC0, 0x41};
uint32_t voltage, current, power, usage, cum_usage;
uint16_t voltage_conv, current_conv;
uint16_t temperature;
#include <SoftwareSerial.h>

SoftwareSerial gc9x(3, 2); // RX, TX

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));

            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            /*Serial.print(F("Frequency: "));
            Serial.println(LMIC.freq);
            Serial.print(F("RSSI: "));
            Serial.println(LMIC.rssi);
            Serial.print(F("SNR: "));
            Serial.println(LMIC.snr);
            Serial.print(F("txpow: "));
            Serial.println(LMIC.txpow);*/
            Serial.print(F("adrTxPow: "));
            Serial.println(LMIC.adrTxPow);
            Serial.print(F("txChnl: "));
            Serial.println(LMIC.txChnl);
            Serial.println();
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
      gatherData();
      printData();

      voltage_conv = (unsigned int)(voltage / 10.0);
      current_conv = (unsigned int)current;

      mydata[0] = 0b00000000;
      memcpy(mydata + 1, &voltage_conv, sizeof(voltage_conv));
      memcpy(mydata + 1 + sizeof(voltage_conv), &current_conv, sizeof(current_conv));
      memcpy(mydata + 1 + sizeof(voltage_conv) + sizeof(current_conv), &usage, sizeof(usage));
      memcpy(mydata + 1 + sizeof(voltage_conv) + sizeof(current_conv) + sizeof(usage), &cum_usage, sizeof(cum_usage));

        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, 16, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}


void setup() {
    Serial.begin(9600);
    Serial.println(F("DC Meter Reader"));
  gc9x.begin(19200);
    // LMIC init
    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Let LMIC compensate for +/- 1% clock error
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}


uint8_t readWhenAvailable() {
  while (gc9x.available() == 0) {
    delay(10);
  }
  return gc9x.read();
}

void gatherData() {
  uint8_t a,b,c,d;
  gc9x.write(command,4);
  // Header
  readWhenAvailable();
  readWhenAvailable();
  readWhenAvailable();
  readWhenAvailable();
  // address
  readWhenAvailable();
  // voltage
  a = readWhenAvailable();
  b = readWhenAvailable();
  c = readWhenAvailable();
  d = readWhenAvailable();
  voltage = (long)a << 24 |  (long)b << 16 |  (long)c << 8 | d;
  // current
  a = readWhenAvailable();
  b = readWhenAvailable();
  c = readWhenAvailable();
  d = readWhenAvailable();
  current = (long)a << 24 |  (long)b << 16 |  (long)c << 8 | d;
  // power
  a = readWhenAvailable();
  b = readWhenAvailable();
  c = readWhenAvailable();
  d = readWhenAvailable();
  power = (long)a << 24 |  (long)b << 16 |  (long)c << 8 | d;
  // usage
  a = readWhenAvailable();
  b = readWhenAvailable();
  c = readWhenAvailable();
  d = readWhenAvailable();
  usage = (long)a << 24 |  (long)b << 16 |  (long)c << 8 | d;
  // loadtime
  readWhenAvailable();
  readWhenAvailable();
  readWhenAvailable();
  readWhenAvailable();
  // capacity
  readWhenAvailable();
  readWhenAvailable();
  readWhenAvailable();
  readWhenAvailable();
  // cummulative usage
  a = readWhenAvailable();
  b = readWhenAvailable();
  c = readWhenAvailable();
  d = readWhenAvailable();
  cum_usage = (long)a << 24 |  (long)b << 16 |  (long)c << 8 | d;
  // cum. capacity
  readWhenAvailable();
  readWhenAvailable();
  readWhenAvailable();
  readWhenAvailable();
  // cum. loadtime
  readWhenAvailable();
  readWhenAvailable();
  readWhenAvailable();
  readWhenAvailable();
  // temperature
  c = readWhenAvailable();
  d = readWhenAvailable();
  temperature = (c << 8) + d;
  // switch
  readWhenAvailable();
}

void printData() {
  Serial.print("Voltage: ");
  Serial.print(voltage);
  Serial.println(" mV");
  Serial.print("Current: ");
  Serial.print(current);
  Serial.println(" mA");
  Serial.print("Power: ");
  Serial.print(power);
  Serial.println(" mW");
  Serial.print("Usage: ");
  Serial.print(usage);
  Serial.println(" mWH");
  Serial.print("Cumulative Usage: ");
  Serial.print(cum_usage);
  Serial.println(" mWH");
  Serial.print("Temperature: ");
  Serial.print(temperature/10.0);
  Serial.println(" °C");

  Serial.println("-----------------------------");
}
