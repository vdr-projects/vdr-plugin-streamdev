/*
 *  $Id: connectionHTTP.h,v 1.7 2010/07/19 13:49:31 schmirl Exp $
 */
 
#ifndef VDR_STREAMDEV_SERVERS_CONNECTIONHTTP_H
#define VDR_STREAMDEV_SERVERS_CONNECTIONHTTP_H

#include "connection.h"
#include "server/livestreamer.h"
#include "server/recstreamer.h"

#include <map>
#include <tools/select.h>

class cChannel;
class cMenuList;

class cConnectionHTTP: public cServerConnection {
private:
	enum eHTTPStatus {
		hsRequest,
		hsHeaders,
		hsBody,
		hsFinished,
	};

	std::string                       m_Authorization;
	eHTTPStatus                       m_Status;
	tStrStrMap                        m_Params;
	cStreamdevStreamer               *m_Streamer;
	eStreamType                       m_StreamType;
	// job: transfer
	const cChannel                   *m_Channel;
	int                               m_Apid[2];
	int                               m_Dpid[2];
	// job: replay
	cRecording                       *m_Recording;
	std::string                       m_ReplayPos;
	// job: listing
	cMenuList                        *m_MenuList;

	cMenuList* MenuListFromString(const std::string &PathInfo, const std::string &Filebase, const std::string &Fileext) const;
	cRecording* RecordingFromString(const char* FileBase, const char* FileExt) const;

	bool ProcessURI(const std::string &PathInfo);
	bool HttpResponse(int Code, bool Last, const char* ContentType = NULL, const char* Headers = "", ...);
			//__attribute__ ((format (printf, 5, 6)));
	/**
	 * Extract byte range from HTTP Range header. Returns false if no valid
	 * range is found. The contents of From and To are undefined in this
	 * case. From may be negative in which case To is undefined.
	 * TODO: support for multiple ranges.
	 */
	bool ParseRange(int64_t &From, int64_t &To) const;
protected:
	bool ProcessRequest(void);

public:
	cConnectionHTTP(void);
	virtual ~cConnectionHTTP();

	virtual void Attach(void) { if (m_Streamer != NULL) m_Streamer->Attach(); }
	virtual void Detach(void) { if (m_Streamer != NULL) m_Streamer->Detach(); }

	virtual cString ToText() const;

	virtual bool CanAuthenticate(void);

	virtual bool Command(char *Cmd);

	virtual bool Abort(void) const;
	virtual void Flushed(void);
	inline std::string GetReplayPos() { return m_ReplayPos; }
};

inline bool cConnectionHTTP::Abort(void) const
{
	return !IsOpen() || (m_Streamer && m_Streamer->Abort());
}

#endif // VDR_STREAMDEV_SERVERS_CONNECTIONVTP_H
