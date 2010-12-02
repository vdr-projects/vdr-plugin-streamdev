/*
 *  $Id: assembler.h,v 1.1.1.1 2004/12/30 22:44:04 lordjaxom Exp $
 */

#ifndef VDR_STREAMDEV_ASSEMBLER_H
#define VDR_STREAMDEV_ASSEMBLER_H

#include <vdr/config.h>
#include <vdr/thread.h>

class cTBSocket;

class cStreamdevAssembler: public cThread {
private:
	cTBSocket         *m_Socket;
	cMutex             m_Mutex;
	cCondVar           m_WaitFill;
	int                m_Pipe[2];
	bool               m_Active;
protected:
	virtual void Action(void);

public:
	cStreamdevAssembler(cTBSocket *Socket);
	virtual ~cStreamdevAssembler();

	int ReadPipe(void) const { return m_Pipe[0]; }
	void WaitForFill(void);
};

#endif // VDR_STREAMDEV_ASSEMBLER_H

