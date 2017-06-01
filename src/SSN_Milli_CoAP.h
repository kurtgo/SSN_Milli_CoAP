

#include <utils/errors.h>
#include <coap_server/coapextif.h>
#include <coap_server/coap_rsp_msg.h>
#include <coap_server/coapobserve.h>
#include <coap_server/coappdu.h>
#include <coap_server/coapsensorobs.h>
#include <coap_server/coapsensoruri.h>
#include <coap_server/coaputil.h>
#include <coap_server/coap_server.h>
#include <coap_server/exp_coap.h>
#include <hdlc/crc_xmodem.h>
#include <hdlc/hdlc.h>
#include <hdlc/hdlcs.h>
#include <utils/bufutil.h>
#include <utils/hbuf.h>
#include <utils/includes.h>
#include <utils/log.h>
#include <coap_rsrcs/arduino_time.h>
#pragma once

#define MAX_SENSORS 3


/******************************************************************************/
//
// CoAP Observe Max-Age, see Section 5.10.5 of rfc7252
//
#define COAP_MSG_MAX_AGE_IN_SECS		90


/******************************************************************************/
//
// 1. Specify a pointer to the printing Serial object (for Serial Monitor)
// 2. Specify a pointer to the UART Serial object (for the HDLC connection)
//



/******************************************************************************/
//
// This define sets the logging level
// The available values are defined in log.h
// If you specify LOG_EMERG, all logging in the library is turned off
// If you specify LOG_INFO, you'll get some printing to the Serial Monitor
// If you specify LOG_DEBUG, a lot of messages will be printed

#define LOG_LEVEL						LOG_DEBUG


/******************************************************************************/
//
// Set the time-out for the UART between Arduino and MilliShield
//
#define UART_TIMEOUT_IN_MS				2000


/******************************************************************************/
/* The largest HDLC payload size                                              */
/* The maximum payload length in the mNIC is 255                              */
#define MAX_HDLC_INFO_LEN				(255)

/******************************************************************************/
//
// Local time zone relative to UTC
// Examples: Pacific: -8, Eastern: -5, London: 0, Paris: +1, Sydney: +10
//
#define LOCAL_TIME_ZONE					(-8)

#define COAP_MSG_MAX_AGE_IN_SECS		90

#define SSN_MILLI_COAP_VERSION_NUMBER "1.3.0"
typedef error_t (*coap_cb3)(struct coap_msg_ctx *req, struct coap_msg_ctx *rsp, void *it);

#define SSN_MILLI_WAKEUP_PIN 3  //

static int registry_count;
extern int coap_wakeup_pin;

class SSN_Milli_CoAP
{
	static struct {
		char *uri;
		coap_cb3 callback;
	} registry[MAX_SENSORS];

public:

	static int RegisterUri(char *uri, coap_cb3 callback)
	{
		if (registry_count < MAX_SENSORS) {
			registry[registry_count].uri = uri;
			registry[registry_count].callback = callback;
			++registry_count;
			return registry_count;
		}
		return 0;
	}

	int setup(int wake_pin, Serial_ &log, int log_level, HardwareSerial &hdlc_uart)
	{
		coap_wakeup_pin = wake_pin;

		// Set-up serial port for logging output
		// Print debug messages to the Serial Monitor
		// The pointer to the serial object is defined in mshield.h
		// NOTE: it takes a few seconds before you can start printing
		log_init( &log, log_level );

		// Init the clock and set the local time zone
		rtc_time_init(LOCAL_TIME_ZONE);

		// Init the CoAP Server
		coap_s_init( &hdlc_uart, COAP_MSG_MAX_AGE_IN_SECS, UART_TIMEOUT_IN_MS, MAX_HDLC_INFO_LEN );

		return ERR_OK;
	}

	void RegisterObserver( const char * uri, ObsFuncPtr p )
	{
		set_observer( uri, p );
	}


private:
	/*
	 * @brief crarduino
	 *
	 *
	 */
	static error_t crarduino( struct coap_msg_ctx *req, struct coap_msg_ctx *rsp )
	{
		struct optlv *o;
		void *it = NULL;

		/*
		 * No URI path beyond /arduino, except /humi and /temp is supported, so
		 * reject if present.
		 */
		copt_get_next_opt_type((const sl_co*)&(req->oh), COAP_OPTION_URI_PATH, &it);
		if ((o = copt_get_next_opt_type((const sl_co*)&(req->oh), COAP_OPTION_URI_PATH, &it)))
		{
			int idx;
			for (idx=0;idx<registry_count;++idx) {
				if (!coap_opt_strcmp( o, registry[idx].uri ))
					return registry[idx].callback(req,rsp, it);
			}

			rsp->code = COAP_RSP_404_NOT_FOUND;

		} // if

		rsp->plen = 0;

		return ERR_OK;

	} // crarduino


};

