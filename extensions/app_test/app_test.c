/*********************************************************************************************************
* Software License Agreement (BSD License)                                                               *
* Author: Sebastien Decugis <sdecugis@nict.go.jp>							 *
*													 *
* Copyright (c) 2009, WIDE Project and NICT								 *
* All rights reserved.											 *
* 													 *
* Redistribution and use of this software in source and binary forms, with or without modification, are  *
* permitted provided that the following conditions are met:						 *
* 													 *
* * Redistributions of source code must retain the above 						 *
*   copyright notice, this list of conditions and the 							 *
*   following disclaimer.										 *
*    													 *
* * Redistributions in binary form must reproduce the above 						 *
*   copyright notice, this list of conditions and the 							 *
*   following disclaimer in the documentation and/or other						 *
*   materials provided with the distribution.								 *
* 													 *
* * Neither the name of the WIDE Project or NICT nor the 						 *
*   names of its contributors may be used to endorse or 						 *
*   promote products derived from this software without 						 *
*   specific prior written permission of WIDE Project and 						 *
*   NICT.												 *
* 													 *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A *
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 	 *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 	 *
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR *
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF   *
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.								 *
*********************************************************************************************************/

/* 
 * Test application for freeDiameter.
 */

#include "app_test.h"
#include <signal.h>

/* Initialize the configuration */
struct atst_conf * atst_conf = NULL;
static struct atst_conf _conf;

static int atst_conf_init(void)
{
	atst_conf = &_conf;
	memset(atst_conf, 0, sizeof(struct atst_conf));
	
	/* Set the default values */
	atst_conf->vendor_id  = 0;		/* IETF */
	atst_conf->appli_id   = 0xffffff;	/* dummy value */
	atst_conf->cmd_id     = 0xfffffe;	/* Experimental */
	atst_conf->avp_id     = 0xffffff;	/* dummy value */
	atst_conf->mode       = MODE_SERV | MODE_CLI;
	atst_conf->dest_realm = strdup(fd_g_config->cnf_diamrlm);
	atst_conf->dest_host  = NULL;
	atst_conf->signal     = APP_TEST_DEFAULT_SIGNAL;
	
	return 0;
}

static void atst_conf_dump(void)
{
	if (!TRACE_BOOL(INFO))
		return;
	fd_log_debug( "------- app_test configuration dump: ---------\n");
	fd_log_debug( " Vendor Id .......... : %u\n", atst_conf->vendor_id);
	fd_log_debug( " Application Id ..... : %u\n", atst_conf->appli_id);
	fd_log_debug( " Command Id ......... : %u\n", atst_conf->cmd_id);
	fd_log_debug( " AVP Id ............. : %u\n", atst_conf->avp_id);
	fd_log_debug( " Mode ............... : %s%s\n", atst_conf->mode & MODE_SERV ? "Serv" : "", atst_conf->mode & MODE_CLI ? "Cli" : "" );
	fd_log_debug( " Destination Realm .. : %s\n", atst_conf->dest_realm ?: "- none -");
	fd_log_debug( " Destination Host ... : %s\n", atst_conf->dest_host ?: "- none -");
	fd_log_debug( " Signal ............. : %i\n", atst_conf->signal);
	fd_log_debug( "------- /app_test configuration dump ---------\n");
}

/* entry point */
static int atst_entry(char * conffile)
{
	TRACE_ENTRY("%p", conffile);
	
	/* Initialize configuration */
	CHECK_FCT( atst_conf_init() );
	
	/* Parse configuration file */
	if (conffile != NULL) {
		CHECK_FCT( atst_conf_handle(conffile) );
	}
	
	TRACE_DEBUG(INFO, "Extension APP/Test initialized with configuration: '%s'", conffile);
	atst_conf_dump();
	
	/* Install objects definitions for this test application */
	CHECK_FCT( atst_dict_init() );
	
	/* Install the handlers for incoming messages */
	if (atst_conf->mode & MODE_SERV) {
		CHECK_FCT( atst_serv_init() );
	}
	
	/* Start the signal handler thread */
	if (atst_conf->mode & MODE_CLI) {
		CHECK_FCT( atst_cli_init() );
	}
	
	/* Advertise the support for the test application in the peer */
	CHECK_FCT( fd_disp_app_support ( atst_appli, atst_vendor, 1, 0 ) );
	
	return 0;
}

/* Unload */
void fd_ext_fini(void)
{
	if (atst_conf->mode & MODE_CLI)
		atst_cli_fini();
	if (atst_conf->mode & MODE_SERV)
		atst_serv_fini();
}

EXTENSION_ENTRY("app_test", atst_entry);