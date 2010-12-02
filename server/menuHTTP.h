#ifndef VDR_STREAMDEV_SERVERS_MENUHTTP_H
#define VDR_STREAMDEV_SERVERS_MENUHTTP_H

#include <string>
#include "../common.h"

class cChannel;

// ******************** cChannelIterator ******************
class cChannelIterator
{
	private:
		const cChannel *channel;
	protected:
		virtual const cChannel* NextChannel(const cChannel *Channel) = 0;
		static inline const cChannel* SkipFakeGroups(const cChannel *Channel);
	public:
		const cChannel* Next();
		cChannelIterator(const cChannel *First);
		virtual ~cChannelIterator() {};
};

class cListAll: public cChannelIterator
{
	protected:
		virtual const cChannel* NextChannel(const cChannel *Channel);
	public:
		cListAll();
		virtual ~cListAll() {};
};

class cListChannels: public cChannelIterator
{
	protected:
		virtual const cChannel* NextChannel(const cChannel *Channel);
	public:
		cListChannels();
		virtual ~cListChannels() {};
};

class cListGroups: public cChannelIterator
{
	protected:
		virtual const cChannel* NextChannel(const cChannel *Channel);
	public:
		cListGroups();
		virtual ~cListGroups() {};
};

class cListGroup: public cChannelIterator
{
	private:
		static const cChannel* GetNextChannelInGroup(const cChannel *Channel);
	protected:
		virtual const cChannel* NextChannel(const cChannel *Channel);
	public:
		cListGroup(const cChannel *Group);
		virtual ~cListGroup() {};
};

class cListTree: public cChannelIterator
{
	private:
		const cChannel* selectedGroup;
		const cChannel* currentGroup;
	protected:
		virtual const cChannel* NextChannel(const cChannel *Channel);
	public:
		cListTree(const cChannel *SelectedGroup);
		virtual ~cListTree() {};
};

// ******************** cChannelList ******************
class cChannelList
{
	private:
		cChannelIterator *iterator;
	protected:
		const cChannel* NextChannel() { return iterator->Next(); }
	public:
		// Helper which returns the group index
		static int GetGroupIndex(const cChannel* Group);
		// Helper which returns the group by its index
		static const cChannel* GetGroup(int Index);

		virtual std::string HttpHeader() { return "HTTP/1.0 200 OK\r\n"; };
		virtual bool HasNext() = 0;
		virtual std::string Next() = 0;
		cChannelList(cChannelIterator *Iterator);
		virtual ~cChannelList();
};

class cHtmlChannelList: public cChannelList
{
	private:
		static const char* menu;
		static const char* css;
		static const char* js;

		enum eHtmlState {
			hsRoot, hsHtmlHead, hsCss, hsJs, hsPageTop, hsPageBottom,
			hsGroupTop, hsGroupBottom,
			hsPlainTop, hsPlainItem, hsPlainBottom,
			hsItemsTop, hsItem, hsItemsBottom 
		};
		eHtmlState htmlState;
		const cChannel *current;
		eStreamType streamType;
		const char* self;
		const char* groupTarget;

		std::string StreamTypeMenu();
		std::string HtmlHead();
		std::string PageTop();
		std::string GroupTitle();
		std::string ItemText();
		std::string PageBottom();
	public:
		virtual std::string HttpHeader() {
			return cChannelList::HttpHeader()
				+ "Content-type: text/html; charset="
				+ (cCharSetConv::SystemCharacterTable() ? cCharSetConv::SystemCharacterTable() : "UTF-8")
				+ "\r\n";
		}
		virtual bool HasNext();
		virtual std::string Next();
		cHtmlChannelList(cChannelIterator *Iterator, eStreamType StreamType, const char *Self, const char *GroupTarget);
		virtual ~cHtmlChannelList();
};

class cM3uChannelList: public cChannelList
{
	private:
		char *base;
		enum eM3uState { msFirst, msContinue, msLast };
		eM3uState m3uState;
		cCharSetConv m_IConv;
	public:
		virtual std::string HttpHeader() { return cChannelList::HttpHeader() + "Content-type: audio/x-mpegurl; charset=UTF-8\r\n"; };
		virtual bool HasNext();
		virtual std::string Next();
		cM3uChannelList(cChannelIterator *Iterator, const char* Base);
		virtual ~cM3uChannelList();
};

inline const cChannel* cChannelIterator::SkipFakeGroups(const cChannel* Group)
{
	while (Group && Group->GroupSep() && !*Group->Name())
		Group = Channels.Next(Group);
	return Group;
}

#endif
