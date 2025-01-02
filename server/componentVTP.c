/*
 *  componentVTP.c
 */
 
#include "server/componentVTP.h"
#include "server/connectionVTP.h"
#include "server/setup.h"

cComponentVTP::cComponentVTP(void): 
		cServerComponent("VTP", StreamdevServerSetup.VTPBindIP, 
		                 StreamdevServerSetup.VTPServerPort)
{
}

cServerConnection *cComponentVTP::NewClient(void)
{
	return new cConnectionVTP;
}
