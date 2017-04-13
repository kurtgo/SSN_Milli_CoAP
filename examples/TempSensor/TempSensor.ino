#include "Arduino.h"

#if defined(ARDUINO_ARCH_SAMD)
  // Use SerialUSB for Serial on Zero based boards
  #define Serial 						SerialUSB
#endif

#if defined(ARDUINO_ARCH_SAM)
  //#define Serial						SerialUSB
#endif

// Specify Serial object used to access the UART for the HDLC connection
// NOTE: the baud rate of the HDLC connection is fixed
#define SerialUART	         			Serial1

// UART between Arduino and MilliShield
#define UART_TIMEOUT_IN_MILLISECONDS	2000

// Specify baud rate for the debug console
#define CONSOLE_BAUD_RATE     			115200

// Local time zone relative to UTC; Examples PST: -8, MST: -7, Central: -6, EST: -5
#define LOCAL_TIME_ZONE					(-8)

// LED parameters
#define LED_PIN_NUMBER					4
#define LED_BLINK_DURATION_IN_SECONDS	5
#define LED_BLINK_SLOW_PERIOD_MS		800
#define LED_BLINK_FAST_PERIOD_MS		100

// Max-Age, see Section 5.10.5 of rfc7252
#define COAP_MSG_MAX_AGE_IN_SECONDS		90

#include "SSN_Milli_CoAP.h"
#include <DHT_U.h>
#include <RTCZero.h>
#include "led.h"
#include "arduino_time.h"

SSN_Milli_CoAP ssn_coap;

// The Arduino init function
void setup()
{
  char ver[64];
  int res;


  ssn_coap.init(cbarduino);

  /* Configure LED */
  arduino_init_resources();

  // Init temp sensor
  arduino_temp_sensor_init();

  // register get/put callback for temp
  ssn_coap.RegisterUri("temp", crtemperature);

  // Print version number, time and date
  sprintf( ver, "Arduino MilliShield Software Version Number: %s\n", ssn_coap.VERSION_NUMBER );
  print_buf(ver);
  sprintf( ver, "Time: %s\n", __TIME__ );
  print_buf(ver);
  sprintf( ver, "Date: %s\n", __DATE__ );
  print_buf(ver);

} // setup

// The main loop
void loop()
{
	ssn_coap.poll();
} // loop


