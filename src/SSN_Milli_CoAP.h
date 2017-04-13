

#include <utils/errors.h>
#include <coap_server/coapextif.h>
#include <coap_server/coapmsg.h>
#include <coap_server/coapobserve.h>
#include <coap_server/coappdu.h>
#include <coap_server/coapsensorobs.h>
#include <coap_server/coapsensoruri.h>
#include <coap_server/coaputil.h>
#include <coap_server/coap_server.h>
#include <coap_server/exp_coap.h>
#include <coap_rsrcs/resrc_coap_if.h>
#include <hdlc/crc_xmodem.h>
#include <hdlc/hdlc.h>
#include <hdlc/hdlcs.h>
#include <coap_rsrcs/led.h>
#include <utils/bufutil.h>
#include <utils/hbuf.h>
#include <utils/includes.h>
#include <utils/log.h>

#define MAX_SENSORS 3

#define SSN_MILLI_COAP_VERSION_NUMBER "1.3.0"
typedef error_t (*coap_cb3)(struct coap_msg_ctx *req, struct coap_msg_ctx *rsp, void *it);
int coap_wakeup_pin;
static struct {
	char *uri;
	coap_cb3 callback;
} registry[MAX_SENSORS];
static int registry_count;

class SSN_Milli_CoAP
{

public:

	static int init(int wakeup_pin) {
		int res;

		registry_count = 0;
		coap_wakeup_pin = wakeup_pin;

		// Set-up serial port for logging output
		// Print debug message via Native USB
		// NOTE: it takes a few seconds before you can start printing
		// The object Serial is defined in mshield.h
		log_init(&Serial,CONSOLE_BAUD_RATE);


		// Init CoAP registry
		coap_registry_init();

		/*
		 * Since coap_s_uri_proc only matches on the first path option, and it's
		 * against the whole path, registering L_URI_LOGISTICS guarantees a match
		 * or that URI, and its children. Putting it first means it's found faster.
		 * We will never match the subsequent L_URI_LOGISTICS paths due to the
		 * separation of the options but the concatenation of the path.
		 * TODO Optimise this so that only one URI is required to be registered
		 * while satisfying .well-known/core discovery.
		 */
		coap_uri_register(L_URI_ARDUINO, crarduino, CLA_ARDUINO);

		/* Set Max-Age; CoAP Server Response Option 14 */
		coap_set_max_age(COAP_MSG_MAX_AGE_IN_SECONDS);

		// Open HDLCS
		// The object SerialUART is defined in mshield.h
		res = hdlcs_open( &SerialUART, UART_TIMEOUT_IN_MILLISECONDS );
		if (res)
		{
			dlog(LOG_ERR, "HDLC initialization failed");

		} // if
		return res;
	}


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


	static void poll() {
		struct mbuf *appd;
		struct mbuf *arsp;
		boolean is_con;

		/* Only actuate if disconnected */
		is_con = hdlcs_is_connected();
		if ( false == is_con )
		{
			// Actuate LED
			arduino_led_actuate();

		} // if

		/* Run the secondary-station HDLC state machine */
		hdlcs_run();

		/* Serve incoming request, if any */
		appd = hdlcs_read();
		if (appd)
		{
			/* Run the CoAP server */
			arsp = coap_s_proc(appd);
			if (arsp)
			{
				/* Send CoAP response, if any */
				hdlcs_write(arsp->data, arsp->len);

			} // if

		} // if

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

