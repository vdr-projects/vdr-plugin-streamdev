/*
 *  $Id: i18n.c,v 1.8.2.4 2009/01/29 07:49:04 schmirl Exp $
 */
 
#include "i18n.h"

const char *i18n_name = NULL;

const tI18nPhrase Phrases[] = {
	{	"VDR Streaming Server",		// English
		"VDR Streaming Server",		// Deutsch
		"",		// Slovenski
		"Server trasmissione VDR",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Serveur de streaming VDR",		// Français
		"",		// Norsk
		"VDR-suoratoistopalvelin",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika / Greek
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"VDR Streaming áÕàÒÕà",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"VTP Streaming Client",		// English
		"VTP Streaming Client",		// Deutsch
		"",		// Slovenski
		"Client trasmissione VTP",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Client de streaming VTP",		// Français
		"",		// Norsk
		"VTP-suoratoistoasiakas ",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika / Greek
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"VTP Streaming ÚÛØÕÝâ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Start VDR-to-VDR Server",		// English
		"VDR-zu-VDR Server starten",		// Deutsch
		"",		// Slovenski
		"Avvia Server VDR-a-VDR",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Démarrer le serveur VDR-to-VDR",		// Français
		"",		// Norsk
		"Käynnistä VDR-palvelin",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika / Greek
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÁâÐàâ VDR-to-VDR áÕàÒÕà",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Start HTTP Server",		// English
		"HTTP Server starten",		// Deutsch
		"",		// Slovenski
		"Avvia Server HTTP",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Démarrer le serveur HTTP",		// Français
		"",		// Norsk
		"Käynnistä HTTP-palvelin",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÁâÐàâ HTTP áÕàÒÕàÐ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"HTTP Streamtype",		// English
		"HTTP Streamtyp",		// Deutsch
		"",		// Slovenski
		"Tipo flusso HTTP",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Type de Streaming HTTP",		// Français
		"",		// Norsk
		"HTTP-lähetysmuoto",		// Suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÂØß HTTP ßÞâÞÚÐ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Start Client",		// English
		"Client starten",		// Deutsch
		"",		// Slovenski
		"Avvia Client",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Démarrage du client",		// Français
		"",		// Norsk
		"Käynnistä VDR-asiakas",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÁâÐàâ ÚÛØÕÝâÐ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"VDR-to-VDR Server Port",		// English
		"Port des VDR-zu-VDR Servers",		// Deutsch
		"",		// Slovenski
		"Porta Server VDR-a-VDR",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Port du serveur VDR-to-VDR",		// Français
		"",		// Norsk
		"VDR-palvelimen portti",		// Suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"VDR-to-VDR ßÞàâ áÕàÒÕàÐ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"HTTP Server Port",		// English
		"Port des HTTP Servers",		// Deutsch
		"",		// Slovenski
		"Porta Server HTTP",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Port du serveur HTTP",		// Français
		"",		// Norsk
		"HTTP-palvelimen portti",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"HTTP áÕàÒÕà ¿Þàâ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Maximum Number of Clients",		// English
		"Maximalanzahl an Clients",		// Deutsch
		"",		// Slovenski
		"Numero massimo di Client",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Nombre maximun de clients",		// Français
		"",		// Norsk
		"Suurin sallittu asiakkaiden määrä",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"¼ÐÚá. ÚÞÛØçÕáâÒÞ ÚÛØÕÝâÞÒ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Remote IP",		// English
		"IP der Gegenseite",		// Deutsch
		"",		// Slovenski
		"Indirizzo IP del Server",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Adresse IP du serveur",		// Français
		"",		// Norsk
		"Etäkoneen IP-osoite",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÃÔÐÛÕÝÝëÙ IP",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Remote Port",		// English
		"Port der Gegenseite",		// Deutsch
		"",		// Slovenski
		"Porta Server Remoto",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Port du serveur",		// Français
		"",		// Norsk
		"Etäkoneen portti",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÃÔÐÛÕÝÝëÙ ßÞàâ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Common Settings",		// English
		"Allgemeines",		// Deutsch
		"",		// Slovenski
		"Impostazioni comuni",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Paramètres communs",		// Français
		"",		// Norsk
		"Yleiset asetukset",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"½ÐáâàÞÙÚØ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"VDR-to-VDR Server",		// English
		"VDR-zu-VDR Server",		// Deutsch
		"",		// Slovenski
		"Server VDR-a-VDR",		// Italiano
		"",		// Nederlands
		"",		// Português
		"VDR-to-VDR Serveur",		// Français
		"",		// Norsk
		"VDR-palvelin",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"VDR-to-VDR áÕàÒÕà",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"HTTP Server",		// English
		"HTTP Server",		// Deutsch
		"",		// Slovenski
		"Server HTTP",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Serveur HTTP",		// Français
		"",		// Norsk
		"HTTP-palvelin",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"HTTP áÕàÒÕà",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Synchronize EPG",		// English
		"EPG synchronisieren",		// Deutsch
		"",		// Slovenski
		"Sincronizza EPG",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Synchroniser l'EPG",		// Français
		"",		// Norsk
		"Päivitä ohjelmaopas",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÁØÝåàÞÝØ×ÐæØï EPG",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Minimum Priority",		// English
		"Minimale Priorität",		// Deutsch
		"",		// Slovenski
		"",		// Italiano
		"",		// Nederlands
		"",		// Português
		"",		// Français
		"",		// Norsk
		"",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Maximum Priority",		// English
		"Maximale Priorität",		// Deutsch
		"",		// Slovenski
		"",		// Italiano
		"",		// Nederlands
		"",		// Português
		"",		// Français
		"",		// Norsk
		"",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Suspend Live TV",		// English
		"Live-TV pausieren",		// Deutsch
		"",		// Slovenski
		"Sospendi TV dal vivo",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Suspendre Live TV",		// Français
		"",		// Norsk
		"Pysäytä suora TV-lähetys",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"¾áâÐÝÞÒÚÐ Live TV",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Suspend behaviour",		// English
		"Pausierverhalten",		// Deutsch
		"",		// Slovenski
		"Tipo sospensione",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Suspendre",		// Français
		"",		// Norsk
		"Pysäytystoiminto",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"¿ÞÒÕÔÕÝØÕ ÞáâÐÝÞÒÚØ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Offer suspend mode",		// English
		"Pausieren anbieten",		// Deutsch
		"",		// Slovenski
		"Offri mod. sospensione",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Offrir le mode suspendre",		// Français
		"",		// Norsk
		"tyrkytä",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"¿àÕÔÛÐÓÐâì ÞáâÐÝÞÒÚã",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Always suspended",		// English
		"Immer pausiert",		// Deutsch
		"",		// Slovenski
		"Sempre sospeso",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Toujours suspendre",		// Français
		"",		// Norsk
		"aina",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"²áÕÓÔÐ ÞáâÐÝÞÒÛÕÝ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Never suspended",		// English
		"Nie pausiert",		// Deutsch
		"",		// Slovenski
		"Mai sospeso",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Jamais suspendre",		// Français
		"",		// Norsk
		"ei koskaan",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"½ØÚÞÓÔÐ ÝÕ ÞáâÐÝÞÒÛÕÝ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Suspend Server",		// English
		"Server pausieren",		// Deutsch
		"",		// Slovenski
		"Sospendi Server",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Suspendre le serveur",		// Français
		"",		// Norsk
		"Pysäytä palvelin",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"¾áâÐÝÞÒØâì áÕàÒÕà",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Server is suspended",		// English
		"Server ist pausiert",		// Deutsch
		"",		// Slovenski
		"Server sospeso",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Le serveur est suspendu",		// Français
		"",		// Norsk
		"Palvelin on pysäytetty",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÁÕàÒÕà ÞáâÐÝÞÒÛÕÝ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Couldn't suspend Server!",		// English
		"Konnte Server nicht pausieren!",		// Deutsch
		"",		// Slovenski
		"Impossibile sospendere il server!",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Impossible de suspendre le serveur!",		// Français
		"",		// Norsk
		"Palvelinta ei onnistuttu pysäyttämään!",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÝÕ ÜÞÓã ÞáâÐÝÞÒØâì áÕàÒÕà",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Client may suspend",		// English
		"Client darf pausieren",		// Deutsch
		"",		// Slovenski
		"Permetti sospensione al Client",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Le client peut suspendre",		// Français
		"",		// Norsk
		"Asiakas saa pysäyttää palvelimen",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ºÛØÕÝâ ÜÞÖÕâ ÞáâÐÝÐÒÛØÒÐâì",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{	"Bind to IP",		// English
		"Binde an IP",		// Deutsch
		"",		// Slovenski
		"IP associati",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Attacher aux IP",		// Français
		"",		// Norsk
		"Sido osoitteeseen",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"¿àØáÞÕÔØÝØâìáï Ú IP",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
        },
	{	"Filter Streaming",		// English
		"Filter-Daten streamen",		// Deutsch
		"",		// Slovenski
		"Filtra trasmissione",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Filtre streaming",		// Français
		"",		// Norsk
		"Suodatetun tiedon suoratoisto",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÄØÛìâà ßÞâÞÚÐ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
        },
	{	"Streaming active",		// English
		"Streamen im Gange",		// Deutsch
		"",		// Slovenski
		"Trasmissione attiva",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Streaming actif",		// Français
		"",		// Norsk
		"Suoratoistopalvelin aktiivinen",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÁâàØÜØÝÓ ÐÚâØÒÕÝ",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
        },
	{	"Hide Mainmenu Entry",		// English
		"Hauptmenüeintrag verstecken",		// Deutsch
		"",		// Slovenski
		"Nascondi voce menu principale",		// Italiano
		"",		// Nederlands
		"",		// Português
		"Masquer dans le menu principal",		// Français
		"",		// Norsk
		"Piilota valinta päävalikosta",		// suomi
		"",		// Polski
		"",		// Español
		"",		// Ellinika
		"",		// Svenska
		"",		// Romaneste
		"",		// Magyar
		"",		// Catala
		"ÁßàïâÐâì Ò ÓÛÐÒÝÞÜ ÜÕÝî",		// Russian
		"",		// Hrvatski
		"",		// Eesti
		"",		// Dansk
		"",		// Czech
#if VDRVERSNUM >= 10502
		"",		// Türkçe
#endif
	},
	{ NULL }
};
