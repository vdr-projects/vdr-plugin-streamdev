/*
 *  menu.h
 */
 
#ifndef VDR_STREAMDEV_MENU_H
#define VDR_STREAMDEV_MENU_H

#include <vdr/osdbase.h>
#include "connection.h"

class cStreamdevServerMenu: public cOsdMenu {
private:
	void SetHelpKeys();
	eOSState Disconnect();
	eOSState Suspend();
protected:
	virtual eOSState ProcessKey(eKeys Key);

public:
	cStreamdevServerMenu();
	virtual ~cStreamdevServerMenu();
};

#endif // VDR_STREAMDEV_MENU_H
