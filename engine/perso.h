/*	$Csoft: perso.h,v 1.16 2003/09/02 02:07:48 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_PERSO_H_
#define _AGAR_PERSO_H_

#include <engine/gobject.h>

#include "begin_code.h"

#define PERSO_NAME_MAX	256

struct perso {
	struct gobject obj;

	pthread_mutex_t	 lock;
	char		 name[PERSO_NAME_MAX];	/* Name set by user */
	Uint32		 flags;
	Uint32		 seed;			/* Random seed */
	Sint32		 level;			/* Current level */
	Uint32		 exp;			/* Experience */
	int		 age;			/* Age */
	int		 maxhp, maxmp;		/* Maximum HP/MP */
	int		 hp, mp;		/* Effective HP/MP */
	Uint32		 nzuars;		/* Money */
};

#define PERSO(ob)	((struct perso *)(ob))

__BEGIN_DECLS
struct perso	*perso_new(void *, const char *);
void		 perso_init(void *, const char *);
void		 perso_destroy(void *);
struct window	*perso_edit(void *);
int		 perso_load(void *, struct netbuf *);
int		 perso_save(void *, struct netbuf *);
int		 perso_specify(void *, void *, void *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_PERSO_H_ */
