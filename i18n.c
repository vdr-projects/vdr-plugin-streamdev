/*
 *  $Id: i18n.c,v 1.7 2008/04/07 14:40:39 schmirl Exp $
 */
 
#include "i18n.h"

const char *i18n_name = NULL;

const tI18nPhrase Phrases[] = {
	{	"VDR Streaming Server",	// English
		"VDR Streaming Server",	// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"VDR-suoratoistopalvelin",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika / Greek
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"VTP Streaming Client",	// English
		"VTP Streaming Client",	// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"VTP-suoratoistoasiakas ",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika / Greek
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Start VDR-to-VDR Server",// English
		"VDR-zu-VDR Server starten",// Deutsch
		"",											// Slovenski
		"Avvia il Server VDR-toVDR",// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Käynnistä VDR-palvelin",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika / Greek
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Start HTTP Server",		// English
		"HTTP Server starten",	// Deutsch
		"",											// Slovenski
		"Avvia il Server HTTP", // Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Käynnistä HTTP-palvelin",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"HTTP Streamtype",		  // English
		"HTTP Streamtyp",	      // Deutsch
		"",											// Slovenski
		"Tipo di Stream HTTP",  // Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"HTTP-lähetysmuoto",									// Suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Start Client",					// English
		"Client starten",				// Deutsch
		"",											// Slovenski
		"Avvia il Client",			// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Käynnistä VDR-asiakas",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"VDR-to-VDR Server Port",// English
		"Port des VDR-zu-VDR Servers",// Deutsch
		"",											// Slovenski
		"Porta del Server VDR-to-VDR",// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"VDR-palvelimen portti",								// Suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"HTTP Server Port",			// English
		"Port des HTTP Servers",// Deutsch
		"",											// Slovenski
		"Porta del Server HTTP",// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"HTTP-palvelimen portti",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Maximum Number of Clients",// English
		"Maximalanzahl an Clients",// Deutsch
		"",											// Slovenski
		"Numero Massimo di Client",// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Suurin sallittu asiakkaiden määrä",							// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Remote IP",						// English
		"IP der Gegenseite",		// Deutsch
		"",											// Slovenski
		"Indirizzo IP del Server",// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Etäkoneen IP-osoite",									// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Remote Port",					// English
		"Port der Gegenseite",	// Deutsch
		"",											// Slovenski
		"Porta del Server Remoto",// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Etäkoneen portti",									// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Remote Streamtype",		// English
		"Streamtyp von Gegenseite",// Deutsch
		"",											// Slovenski
		"Tipo di Stream",       // Italiano (oppure Flusso ?)
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Etäkoneen lähetysmuoto",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Common Settings",		  // English
		"Allgemeines",          // Deutsch
		"",											// Slovenski
		"Settaggi Comuni",			// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Yleiset asetukset",									// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"VDR-to-VDR Server",		// English
		"VDR-zu-VDR Server",    // Deutsch
		"",											// Slovenski
		"Server VDR-to-VDR",		// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"VDR-palvelin",										// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"HTTP Server",		      // English
		"HTTP Server",          // Deutsch
		"",											// Slovenski
		"Server HTTP",					// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"HTTP-palvelin",									// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"VDR-to-VDR Client",		// English
		"VDR-zu-VDR Client",    // Deutsch
		"",											// Slovenski
		"Client VDR-to-VDR",		// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"VDR-asiakas",										// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Please restart VDR to activate changes",// English
		"Bitte starten Sie für die Änderungen VDR neu",// Deutsch
		"",											// Slovenski
		"Riavviare VDR per attivare i cambiamenti",// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Aktivoi muutokset käynnistämällä VDR uudelleen",					// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Synchronize EPG",			// English
		"EPG synchronisieren",	// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Päivitä ohjelmaopas",									// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Suspend Live TV",			// English
		"Live-TV pausieren",		// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Pysäytä suora TV-lähetys",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Suspend behaviour",		// English
		"Pausierverhalten",			// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Pysäytystoiminto",									// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Offer suspend mode",		// English
		"Pausieren anbieten",		// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"tyrkytä",										// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Always suspended",			// English
		"Immer pausiert",				// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"aina",											// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Never suspended",			// English
		"Nie pausiert",					// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"ei koskaan",										// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Suspend Server",				// English
		"Server pausieren",			// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Pysäytä palvelin",									// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Server is suspended",	// English
		"Server ist pausiert",	// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Palvelin on pysäytetty",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Couldn't suspend Server!",// English
		"Konnte Server nicht pausieren!",// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Palvelinta ei onnistuttu pysäyttämään!",						// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
	{	"Client may suspend",		// English
		"Client darf pausieren",// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Asiakas saa pysäyttää palvelimen",							// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
		""                      // Russian
	},
        {       "Bind to IP",           // English
                "",// Deutsch
                "",                                                                                     // Slovenski
                "",                                                                                     // Italiano
                "",                                                                                     // Nederlands
                "",                                                                                     // Português
                "",                                                                                     // Français
                "",                                                                                     // Norsk
                "Sido osoitteeseen",                                                                    // suomi
                "",                                                                                     // Polski
                "",                                                                                     // Español
                "",                                                                                     // Ellinika
                "",                                                                                     // Svenska
                "",                                                                                     // Romaneste
                "",                                                                                     // Magyar
                "",                                                                                     // Catala
                ""                      // Russian
        },
        {       "Filter Streaming",     // English
                "",// Deutsch
                "",                                                                                     // Slovenski
                "",                                                                                     // Italiano
                "",                                                                                     // Nederlands
                "",                                                                                     // Português
                "",                                                                                     // Français
                "",                                                                                     // Norsk
                "Suodatetun tiedon suoratoisto",                                                        // suomi
                "",                                                                                     // Polski
                "",                                                                                     // Español
                "",                                                                                     // Ellinika
                "",                                                                                     // Svenska
                "",                                                                                     // Romaneste
                "",                                                                                     // Magyar
                "",                                                                                     // Catala
                ""                      // Russian
        },
        {       "Streaming active",     // English
                "Streamen im Gange",// Deutsch
                "",                                                                                     // Slovenski
                "",                                                                                     // Italiano
                "",                                                                                     // Nederlands
                "",                                                                                     // Português
                "",                                                                                     // Français
                "",                                                                                     // Norsk
                "Suoratoistopalvelin aktiivinen", // suomi
                "",                                                                                     // Polski
                "",                                                                                     // Español
                "",                                                                                     // Ellinika
                "",                                                                                     // Svenska
                "",                                                                                     // Romaneste
                "",                                                                                     // Magyar
                "",                                                                                     // Catala
                ""                      // Russian
        },
	{ NULL }
};
