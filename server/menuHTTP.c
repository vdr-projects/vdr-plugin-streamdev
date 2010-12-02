#include <vdr/channels.h>
#include "server/menuHTTP.h"

//**************************** cChannelIterator **************
cChannelIterator::cChannelIterator(const cChannel *First): channel(First)
{}

const cChannel* cChannelIterator::Next()
{
	const cChannel *current = channel;
	channel = NextChannel(channel);
	return current;
}

//**************************** cListAll **************
cListAll::cListAll(): cChannelIterator(Channels.First())
{}

const cChannel* cListAll::NextChannel(const cChannel *Channel)
{
	if (Channel)
		Channel = SkipFakeGroups(Channels.Next(Channel));
	return Channel;
}

//**************************** cListChannels **************
cListChannels::cListChannels(): cChannelIterator(Channels.Get(Channels.GetNextNormal(-1)))
{}

const cChannel* cListChannels::NextChannel(const cChannel *Channel)
{
	if (Channel)
		Channel = Channels.Get(Channels.GetNextNormal(Channel->Index()));
	return Channel;
}

// ********************* cListGroups ****************
cListGroups::cListGroups(): cChannelIterator(Channels.Get(Channels.GetNextGroup(-1)))
{}

const cChannel* cListGroups::NextChannel(const cChannel *Channel)
{
	if (Channel)
		Channel = Channels.Get(Channels.GetNextGroup(Channel->Index()));
	return Channel;
}
//
// ********************* cListGroup ****************
cListGroup::cListGroup(const cChannel *Group): cChannelIterator(GetNextChannelInGroup(Group))
{}

const cChannel* cListGroup::GetNextChannelInGroup(const cChannel *Channel)
{
	if (Channel)
		Channel = SkipFakeGroups(Channels.Next(Channel));
	return Channel && !Channel->GroupSep() ? Channel : NULL;
}

const cChannel* cListGroup::NextChannel(const cChannel *Channel)
{
	return GetNextChannelInGroup(Channel);
}
//
// ********************* cListTree ****************
cListTree::cListTree(const cChannel *SelectedGroup): cChannelIterator(Channels.Get(Channels.GetNextGroup(-1)))
{
	selectedGroup = SelectedGroup;
	currentGroup = Channels.Get(Channels.GetNextGroup(-1));
}

const cChannel* cListTree::NextChannel(const cChannel *Channel)
{
	if (currentGroup == selectedGroup)
	{
		if (Channel)
			Channel = SkipFakeGroups(Channels.Next(Channel));
		if (Channel && Channel->GroupSep())
			currentGroup = Channel;
	}
	else
	{
		if (Channel)
			Channel = Channels.Get(Channels.GetNextGroup(Channel->Index()));
		currentGroup = Channel;
	}
	return Channel;
}

// ******************** cChannelList ******************
cChannelList::cChannelList(cChannelIterator *Iterator) : iterator(Iterator)
{}

cChannelList::~cChannelList()
{
	delete iterator;
}

int cChannelList::GetGroupIndex(const cChannel *Group)
{
	int index = 0;
	for (int curr = Channels.GetNextGroup(-1); curr >= 0; curr = Channels.GetNextGroup(curr))
	{
		if (Channels.Get(curr) == Group)
			return index;
		index++;
	}
	return -1;
}

const cChannel* cChannelList::GetGroup(int Index)
{
	int group = Channels.GetNextGroup(-1);
	while (Index-- && group >= 0)
		group = Channels.GetNextGroup(group);
	return group >= 0 ? Channels.Get(group) : NULL;
}

// ******************** cHtmlChannelList ******************
const char* cHtmlChannelList::menu =
	"[<a href=\"/\">Home</a> (<a href=\"all.html\" tvid=\"RED\">no script</a>)] "
	"[<a href=\"tree.html\" tvid=\"GREEN\">Tree View</a>] "
	"[<a href=\"groups.html\" tvid=\"YELLOW\">Groups</a> (<a href=\"groups.m3u\">Playlist</a>)] "
	"[<a href=\"channels.html\" tvid=\"BLUE\">Channels</a> (<a href=\"channels.m3u\">Playlist</a>)] ";

const char* cHtmlChannelList::css =
	"<style type=\"text/css\">\n"
	"<!--\n"
	"a:link, a:visited, a:hover, a:active, a:focus { color:#333399; }\n"
	"body { font:100% Verdana, Arial, Helvetica, sans-serif; background-color:#9999FF; margin:1em; }\n"
	".menu { position:fixed; top:0px; left:1em; right:1em; height:3em; text-align:center; background-color:white; border:inset 2px #9999ff; }\n"
	"h2 { font-size:150%; margin:0em; padding:0em 1.5em; }\n"
	".contents { margin-top:5em; background-color:white; }\n"
	".group { background:url(data:image/gif;base64,R0lGODdhAQAeAIQeAJub/5yc/6Cf/6Oj/6am/6qq/66u/7Gx/7S0/7i4/7u8/7+//8LD/8bG/8nK/83N/9HQ/9TU/9fX/9va/97e/+Lh/+Xl/+no/+3t//Dw//Pz//b3//v7//7+/////////ywAAAAAAQAeAAAFGCAQCANRGAeSKAvTOA8USRNVWReWaRvXhQA7) repeat-x; border:inset 2px #9999ff; }\n"
	".items { border-top:dashed 1px; margin-top:0px; margin-bottom:0px; padding:0.7em 5em; }\n"
	".apid { padding-left:28px; margin:0.5em; background:url(data:image/gif;base64,R0lGODlhGwASAKEBAAAAAP///////////yH5BAEKAAEALAAAAAAbABIAAAJAjI+pywj5WgPxVAmpNRcHqnGb94FPhE7m+okts7JvusSmSys2iLv6TstldjMfhhUkcXi+zjFUVFo6TiVVij0UAAA7) no-repeat; }\n"
	".dpid { padding-left:28px; margin:0.5em; background:url(data:image/gif;base64,R0lGODlhGwASAKEBAAAAAP///////////yH5BAEKAAEALAAAAAAbABIAAAJFjI+py+0BopwAUoqivRvr83UaZ4RWMnVoBbLZaJbuqcCLGcv0+t5Vvgu2hLrh6pfDzVSpnlGEbAZhnIutZaVmH9yuV1EAADs=) no-repeat; }\n"
	"button { width:2em; margin:0.2em 0.5em; vertical-align:top; }\n"
	"-->\n"
	"</style>";

const char* cHtmlChannelList::js =
	"<script language=\"JavaScript\">\n"
	"<!--\n"

	"function eventTarget(evt) {\n"
	"  if (!evt) evt = window.event;\n"
	"  if (evt.target) return evt.target;\n"
	"  else if (evt.srcElement) return evt.srcElement;\n"
	"  else return null;\n"
	"}\n"

	// toggle visibility of a group
	"function clickHandler(evt) {\n"
	"  var button = eventTarget(evt);\n"
	"  if (button) {\n"
	"    var group = document.getElementById('c' + button.id);\n"
	"    if (group) {\n"
	"      button.removeChild(button.firstChild);\n"
	"      if (group.style.display == 'block') {\n"
	"        button.appendChild(document.createTextNode(\"+\"));\n"
	"        group.style.display = 'none';\n"
	"      } else {\n"
	"        button.appendChild(document.createTextNode(\"-\"));\n"
	"        group.style.display = 'block';\n"
	"      }\n"
	"    }\n"
	"  }\n"
	"}\n"

        // insert a click button infront of each h2 and an id to the corresponding list
	"function init() {\n"
	"  var titles = document.getElementsByTagName('h2');\n"
	"  for (var i = 0; i < titles.length; i++) {\n"
	"    var button = document.createElement('button');\n"
	"    button.id = 'g' + i;\n"
	"    button.onclick = clickHandler;\n"
	"    button.appendChild(document.createTextNode('+'));\n"
	"    titles[i].insertBefore(button, titles[i].firstChild);\n"
	"    var group = titles[i].nextSibling;\n"
	"    while (group) {\n"
	"      if (group.className && group.className == 'items') {\n"
	"        group.id = 'cg' + i;\n"
	"        break;\n"
	"      }\n"
	"    group = group.nextSibling;\n"
	"    }\n"
	"  }\n"
	"}\n"

	"window.onload = init;\n"

        // hide lists before the browser renders it
	"if (document.styleSheets[0].insertRule)\n"
	"  document.styleSheets[0].insertRule('.items { display:none }', 0);\n"
	"else if (document.styleSheets[0].addRule)\n"
	"  document.styleSheets[0].addRule('.items', 'display:none');\n"

	"//-->\n"
	"</script>";


std::string cHtmlChannelList::StreamTypeMenu()
{
	std::string typeMenu;
	typeMenu += (streamType == stTS ? (std::string) "[TS] " :
			(std::string) "[<a href=\"/TS/" + self + "\">TS</a>] ");
	typeMenu += (streamType == stPS ? (std::string) "[PS] " :
			(std::string) "[<a href=\"/PS/" + self + "\">PS</a>] ");
	typeMenu += (streamType == stPES ? (std::string) "[PES] " :
			(std::string) "[<a href=\"/PES/" + self + "\">PES</a>] ");
	typeMenu += (streamType == stES ? (std::string) "[ES] " :
			(std::string) "[<a href=\"/ES/" + self + "\">ES</a>] ");
	typeMenu += (streamType == stEXT ? (std::string) "[EXT] " :
			(std::string) "[<a href=\"/EXT/" + self + "\">EXT</a>] ");
	return typeMenu;
}

cHtmlChannelList::cHtmlChannelList(cChannelIterator *Iterator, eStreamType StreamType, const char *Self, const char *GroupTarget): cChannelList(Iterator)
{
	streamType = StreamType;
	self = strdup(Self);
	groupTarget = (GroupTarget && *GroupTarget) ? strdup(GroupTarget) : NULL;
	htmlState = hsRoot;
	current = NULL;
}

cHtmlChannelList::~cHtmlChannelList()
{
	free((void *) self);
	free((void *) groupTarget);
}

bool cHtmlChannelList::HasNext()
{
	return htmlState != hsPageBottom;
}

std::string cHtmlChannelList::Next()
{
	switch (htmlState)
	{
		case hsRoot:
			htmlState = hsHtmlHead;
			break;
		case hsHtmlHead:
			htmlState = hsCss;
			break;
		case hsCss:
			htmlState = *self ? hsPageTop : hsJs;
			break;
		case hsJs:
			htmlState = hsPageTop;
			break;
		case hsPageTop:
			current = NextChannel();
			htmlState = current ? (current->GroupSep() ? hsGroupTop : hsPlainTop) : hsPageBottom;
			break;
		case hsPlainTop:
			htmlState = hsPlainItem;
			break;
		case hsPlainItem:
			current = NextChannel();
			htmlState = current && !current->GroupSep() ? hsPlainItem : hsPlainBottom;
			break;
		case hsPlainBottom:
			htmlState = current ? hsGroupTop : hsPageBottom;
			break;
		case hsGroupTop:
			current = NextChannel();
			htmlState = current && !current->GroupSep() ? hsItemsTop : hsGroupBottom;
			break;
		case hsItemsTop:
			htmlState = hsItem;
			break;
		case hsItem:
			current = NextChannel();
			htmlState = current && !current->GroupSep() ? hsItem : hsItemsBottom;
			break;
		case hsItemsBottom:
			htmlState = hsGroupBottom;
			break;
		case hsGroupBottom:
			htmlState = current ? hsGroupTop : hsPageBottom;
			break;
		case hsPageBottom:
		default:
			esyslog("streamdev-server cHtmlChannelList: invalid call to Next()");
			break;
	}
	switch (htmlState)
	{
		// NOTE: JavaScript requirements:
		// Group title is identified by <h2> tag
		// Channel list must be a sibling of <h2> with class "items"
		case hsHtmlHead:	return "<html><head>" + HtmlHead();
		case hsCss:		return css;
		case hsJs:		return js;
		case hsPageTop:		return "</head><body>" + PageTop() + "<div class=\"contents\">";
		case hsGroupTop:	return "<div class=\"group\"><h2>" + GroupTitle() + "</h2>";
		case hsItemsTop:
		case hsPlainTop:	return "<ol class=\"items\">";
		case hsItem:
		case hsPlainItem:	return ItemText();
		case hsItemsBottom:
		case hsPlainBottom:	return "</ol>";
		case hsGroupBottom:	return "</div>";
		case hsPageBottom:	return "</div>" + PageBottom() + "</body></html>";
		default:		return "";
	}
}

std::string cHtmlChannelList::HtmlHead()
{
	return (std::string) "";
}

std::string cHtmlChannelList::PageTop()
{
	return (std::string) "<div class=\"menu\"><div>" + menu + "</div><div>" + StreamTypeMenu() + "</div></div>";
}

std::string cHtmlChannelList::PageBottom()
{
	return (std::string) "";
}

std::string cHtmlChannelList::GroupTitle()
{
	if (groupTarget)
	{
		return (std::string) "<a href=\"" + groupTarget + "?group=" +
			(const char*) itoa(cChannelList::GetGroupIndex(current)) +
			"\">" + current->Name() + "</a>";
	}
	else
	{
		return (std::string) current->Name();
	}
}

std::string cHtmlChannelList::ItemText()
{
	std::string line;
	std::string suffix;

	switch (streamType) {
		case stTS: suffix = (std::string) ".ts"; break;
		case stPS: suffix = (std::string) ".vob"; break;
		// for Network Media Tank
		case stPES: suffix = (std::string) ".vdr"; break; 
		default: suffix = "";
	}
	line += (std::string) "<li value=\"" + (const char*) itoa(current->Number()) + "\">";
	line += (std::string) "<a href=\"" + (std::string) current->GetChannelID().ToString() + suffix + "\"";

	// for Network Media Tank
	line += (std::string) " vod ";
	if (current->Number() < 1000)
	    line += (std::string) " tvid=\"" + (const char*) itoa(current->Number()) + "\""; 

	line += (std::string) ">" + current->Name() + "</a>";

	int count = 0;
	for (int i = 0; current->Apid(i) != 0; ++i, ++count)
		;
	for (int i = 0; current->Dpid(i) != 0; ++i, ++count)
		;

	if (count > 1)
	{
		int index = 1;
		for (int i = 0; current->Apid(i) != 0; ++i, ++index) {
			line += (std::string) " <a href=\"" + (std::string) current->GetChannelID().ToString() +
					"+" + (const char*)itoa(index) + suffix + "\" class=\"apid\" vod>" + current->Alang(i) + "</a>";
			}
		for (int i = 0; current->Dpid(i) != 0; ++i, ++index) {
			line += (std::string) " <a href=\"" + (std::string) current->GetChannelID().ToString() +
					"+" + (const char*)itoa(index) + suffix + "\" class=\"dpid\" vod>" + current->Dlang(i) + "</a>";
			}
	}
	line += "</li>";
	return line;
}

// ******************** cM3uChannelList ******************
cM3uChannelList::cM3uChannelList(cChannelIterator *Iterator, const char* Base)
: cChannelList(Iterator),
  m_IConv(cCharSetConv::SystemCharacterTable(), "UTF-8")
{
	base = strdup(Base);
	m3uState = msFirst;
}

cM3uChannelList::~cM3uChannelList()
{
	free(base);
}

bool cM3uChannelList::HasNext()
{
	return m3uState != msLast;
}

std::string cM3uChannelList::Next()
{
	if (m3uState == msFirst)
	{
		m3uState = msContinue;
		return "#EXTM3U";
	}

	const cChannel *channel = NextChannel();
	if (!channel)
	{
		m3uState = msLast;
		return "";
	}

	std::string name = (std::string) m_IConv.Convert(channel->Name());

	if (channel->GroupSep())
	{
		return (std::string) "#EXTINF:-1," + name + "\r\n" +
			base + "group.m3u?group=" +
			(const char*) itoa(cChannelList::GetGroupIndex(channel));
	}
	else
	{
		return (std::string) "#EXTINF:-1," +
			(const char*) itoa(channel->Number()) + " " + name + "\r\n" +
			base + (std::string) channel->GetChannelID().ToString();
	}
}

