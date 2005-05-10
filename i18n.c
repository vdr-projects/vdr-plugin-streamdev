/*
 *  $Id: i18n.c,v 1.3 2005/05/10 21:02:24 lordjaxom Exp $
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
	},
	{	"Streaming Control",		// English
		"Streamkontrolle",			// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Suoratoiston hallinta",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
	},
	{	"Fetching recordings...",// English
		"Hole Aufnahmen...",		// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Haetaan tallenteita...",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
	},
	{	"Remote Recordings",		// English
		"Entfernte Aufnahmen",	// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Etätallenteet",									// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
	},
	{	"Remote Timers",				// English
		"Entfernte Timer",			// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Etäajastimet",										// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
	},
	{	"Edit remote timer",		// English
		"Entfernten Timer editieren",// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Muokkaa etäajastinta",									// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
	},
	{	"Timers not in sync! Try again...",// Englisch
		"Timer nicht synchron! Bitte wiederholen...",//Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Ajastimet eivät täsmää! Yritä uudelleen...",						// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
	},
	{	"Couldn't save timer! Try again...",// English
		"Konnte Timer nicht speichern! Bitte wiederholen...",// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Ajastimen tallennus epäonnistui! Yritä uudelleen...",					// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
	},
	{	"Couldn't delete timer! Try again...",// English
		"Konnte Timer nicht löschen! Bitte wiederholen...",// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Ajastimen poistaminen epäonnistui! Yritä uudelleen...",				// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
	},
	{	"Server error! Try again...",// English
		"Serverfehler! Bitte wiederholen...",// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Palvelimessa virhe! Yritä uudelleen...",						// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
	},
	{	"MultiPID Streaming",		// English
		"Multiple PIDs streamen",// Deutsch
		"",											// Slovenski
		"",											// Italiano
		"",											// Nederlands
		"",											// Português
		"",											// Français
		"",											// Norsk
		"Usean PID:in suoratoisto",								// suomi
		"",											// Polski
		"",											// Español
		"",											// Ellinika
		"",											// Svenska
		"",											// Romaneste
		"",											// Magyar
		"",											// Catala
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
		""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
                ""                      // Russian
#endif
        },
        {       "Remote Schedule",      // English
                "",// Deutsch
                "",                                                                                     // Slovenski
                "",                                                                                     // Italiano
                "",                                                                                     // Nederlands
                "",                                                                                     // Português
                "",                                                                                     // Français
                "",                                                                                     // Norsk
                "Etäkoneen ohjelmaopas",                                                                // suomi
                "",                                                                                     // Polski
                "",                                                                                     // Español
                "",                                                                                     // Ellinika
                "",                                                                                     // Svenska
                "",                                                                                     // Romaneste
                "",                                                                                     // Magyar
                "",                                                                                     // Catala
#if VDRVERSNUM >= 10300
                ""                      // Russian
#endif
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
#if VDRVERSNUM >= 10300
                ""                      // Russian
#endif
        },
	{ NULL }
};
