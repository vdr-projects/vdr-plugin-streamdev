/*
 *  $Id: i18n.h,v 1.1.1.1 2004/12/30 22:43:58 lordjaxom Exp $
 */
 
#ifndef VDR_STREAMDEV_I18N_H
#define VDR_STREAMDEV_I18N_H

#include <vdr/i18n.h>

extern const char *i18n_name;
extern const tI18nPhrase Phrases[];

#undef tr
#define tr(s) I18nTranslate(s, i18n_name)

#endif // VDR_STREAMDEV_I18N_H
