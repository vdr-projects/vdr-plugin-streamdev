/*
 *  $Id: componentVTP.c,v 1.1 2004/12/30 22:44:19 lordjaxom Exp $
 */
 
#include "server/componentVTP.h"
#include "server/connectionVTP.h"
#include "server/setup.h"

cComponentVTP::cComponentVTP(void): 
		cServerComponent("VTP", StreamdevServerSetup.VTPBindIP,
				StreamdevServerSetup.VTPServerPort) {
}

cComponentVTP::~cComponentVTP() {
}
