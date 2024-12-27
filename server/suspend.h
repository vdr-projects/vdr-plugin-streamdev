/*
 *  suspend.h
 */
 
#ifndef VDR_STREAMDEV_SUSPEND_H
#define VDR_STREAMDEV_SUSPEND_H

#include <vdr/player.h>

class cSuspendLive: public cPlayer, public cThread {
protected:
	virtual void Activate(bool On);
	virtual void Action(void);

	void Stop(void);

public:
	cSuspendLive(void);
	virtual ~cSuspendLive();
};

class cSuspendCtl: public cControl {
private:
  cSuspendLive *m_Suspend;
	static bool m_Active;

public:
  cSuspendCtl(void);
  virtual ~cSuspendCtl();
  virtual void Hide(void) {}
  virtual eOSState ProcessKey(eKeys Key);

	static bool IsActive(void) { return m_Active; }
};

#endif // VDR_STREAMDEV_SUSPEND_H
