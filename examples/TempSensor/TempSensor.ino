/*

Copyright (c) Silver Spring Networks, Inc.
All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the ""Software""), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Silver Spring Networks, Inc.
shall not be used in advertising or otherwise to promote the sale, use or other
dealings in this Software without prior written authorization from Silver Spring
Networks, Inc.

*/

/*! \mainpage
 *
 *
 * \section intro_sec Introduction
 *
 * The Arduino MilliShield CoAP Server Library was written and tested for
 * Arduino M0, M0 Pro and Due under Windows only.<br>
 *
 * This code base consists of one Arduino library written by Silver Spring Networks: <br>
 *
 * \subsection coap_server The CoAP Server library
 *
 * The Arduino setup() and loop() functions are in the example sketch mshield.ino <br>
 *
 * \section install_sec Installation
 *
 * \subsection step1  Step 1:  Download and install Arduino IDE 1.8.2 from https://www.arduino.cc/en/Main/Software or http://www.arduino.org/downloads depending on your Arduino hardware:
 * \subsection step2  Step 2:  Copy the folder ssni_coap_sensor to C:\Users\<user>\Documents\Arduino\libraries  Replace <user> with your username.
 * \subsection step3  Step 3:  Optional: build the TempSensor example in the library
 * \subsection step4  Step 4:  install the Arduino SAMD board library
 * \subsection step6 Step 5b:  install dependant libraries for sample, use Manage Libraries to locate and install the following libraries:
 * \subsection step7 Step 5c:     Adafruit Unified Sensor v1.0.2
 * \subsection step8 Step 5d:     DHT Sensor Library v1.3.0
 *
 */

#include "SSN_Milli_CoAP.h"
#include "temp_sensor.h"

#if defined(ARDUINO_ARCH_SAMD)
  // Use SerialUSB for Serial Monitor on Zero based boards
	#define SER_MON						SerialUSB
	#define MILLI_UART        			Serial1
#endif

#if defined(ARDUINO_ARCH_SAM)
	#define SER_MON						SerialUSB
	#define MILLI_UART        			Serial
#endif

#define SERIAL_BAUD_RATE 115200


/******************************************************************************/
//
// List of sensors defined using strings, which will be part of the CoAP URI
//

// The default sensor for this Reference App is a temperature sensor (DHT11)
// To access this sensor from a CoAP Client, the URI is /sensor/arduino/temp
#define TEMP_SENSOR           			"temp"


/******************************************************************************/
//
// It's possible to do a CoAP Observe on one sensor
// Once a minute, your CoAP Client will receive a response message
// containing sensor data with timestamp and unit
//

SSN_Milli_CoAP coap;

/*
 * This function handles the URI prior to the query
 * Add your own URI here
 */
error_t crarduino( struct coap_msg_ctx *req, struct coap_msg_ctx *rsp )
{
    struct optlv *o;
    void *it = NULL;

    /*
     * No URI path beyond /arduino, except /temp is supported, so reject if present.
     *
     */
    copt_get_next_opt_type((const sl_co*)&(req->oh), COAP_OPTION_URI_PATH, &it);
    if ((o = copt_get_next_opt_type((const sl_co*)&(req->oh), COAP_OPTION_URI_PATH, &it)))
    {
        // This is the default URI
        if (!coap_opt_strcmp( o, TEMP_SENSOR ))
        {
            return crtemperature( req, rsp, it );
        }


        /* Below, replace MY_SENSOR with your own name of your particular sensor  */
        /* Use the enclosed template (TT_resource.cpp and TT_resource.h) to       */
        /* implement the crmysensor function and associated methods               */
        //if (!coap_opt_strcmp( o, MY_SENSOR ))
        //{
        //    /* Replace mysensor below with a name for your sensor               */
        //    /* The function 'crmysensor' is implemented in a new C++ file       */
        //    return crmysensor( req, rsp, it );
        //}

        rsp->code = COAP_RSP_404_NOT_FOUND;

    } // if

    rsp->plen = 0;

    return ERR_OK;

} // crarduino

/********************************************************************************/

// The Arduino init function
void setup()
{
	SER_MON.begin(SERIAL_BAUD_RATE);
	SER_MON.println("Arduino Temperature Example");

	// Init the temp sensor
	arduino_temp_sensor_init();

	coap.setup(SSN_MILLI_WAKEUP_PIN, SER_MON, LOG_DEBUG, MILLI_UART);
	coap.RegisterObserver("temp", &arduino_get_temp);
}

/********************************************************************************/

// The main loop
void loop()
{
  // Run CoAP Server
  coap_s_run();
}

/********************************************************************************/
