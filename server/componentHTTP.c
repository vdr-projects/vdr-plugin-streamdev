/*
 *  $Id: componentHTTP.c,v 1.1 2004/12/30 22:44:19 lordjaxom Exp $
 */
 
#include "server/componentHTTP.h"
#include "server/connectionHTTP.h"
#include "server/setup.h"

cComponentHTTP::cComponentHTTP(void):
		cServerComponent("HTTP", StreamdevServerSetup.HTTPBindIP,
				StreamdevServerSetup.HTTPServerPort) {
}

cComponentHTTP::~cComponentHTTP() {
}
