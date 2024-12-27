/*
 *  componentHTTP.c
 */
 
#include "server/componentHTTP.h"
#include "server/connectionHTTP.h"
#include "server/setup.h"

cComponentHTTP::cComponentHTTP(void):
		cServerComponent("HTTP", StreamdevServerSetup.HTTPBindIP, 
		                 StreamdevServerSetup.HTTPServerPort) 
{
}

cServerConnection *cComponentHTTP::NewClient(void)
{
	return new cConnectionHTTP;
}
