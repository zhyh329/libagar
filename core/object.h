/*	Public domain	*/

#ifndef _AGAR_CORE_OBJECT_H_
#define _AGAR_CORE_OBJECT_H_

#ifndef AG_OBJECT_NAME_MAX
# define AG_OBJECT_NAME_MAX AG_MODEL
#endif
#ifndef AG_OBJECT_TYPE_MAX
# if AG_MODEL == AG_SMALL
#  define AG_OBJECT_TYPE_MAX 24
# else
#  define AG_OBJECT_TYPE_MAX 48
# endif
#endif
#ifndef AG_OBJECT_HIER_MAX
# if AG_MODEL == AG_SMALL
#  define AG_OBJECT_HIER_MAX 48
# elif AG_MODEL == AG_MEDIUM
#  define AG_OBJECT_HIER_MAX 96
# elif AG_MODEL == AG_LARGE
#  define AG_OBJECT_HIER_MAX 128
# endif
#endif
#ifndef AG_OBJECT_PATH_MAX
# if AG_MODEL == AG_SMALL
#  define AG_OBJECT_PATH_MAX 64
# elif AG_MODEL == AG_MEDIUM
#  define AG_OBJECT_PATH_MAX 128
# elif AG_MODEL == AG_LARGE
#  define AG_OBJECT_PATH_MAX 196
# endif
#endif
#ifndef AG_OBJECT_LIBS_MAX
# if AG_MODEL == AG_SMALL
#  define AG_OBJECT_LIBS_MAX 16
# else
#  define AG_OBJECT_LIBS_MAX 32
# endif
#endif
#ifndef AG_OBJECT_DEP_MAX
# define AG_OBJECT_DEP_MAX (0xffffffff-2)
#endif
#ifndef AG_OBJECT_MAX_VARIABLES
# define AG_OBJECT_MAX_VARIABLES 0xffff
#endif

#define AGOBJECT(ob)        ((struct ag_object *)(ob))
#define AGCLASS(obj)        ((struct ag_object_class *)(obj))
#define AGOBJECT_CLASS(obj) ((struct ag_object_class *)(AGOBJECT(obj)->cls))

struct ag_object;

#include <agar/core/begin.h>

#include <agar/core/variable.h>
#include <agar/core/event.h>
#include <agar/core/time.h>
#include <agar/core/class.h>

AG_TAILQ_HEAD(ag_objectq, ag_object);

/* Entry in dependency table. */
typedef struct ag_object_dep {
	int persistent;				/* Serialize this entry? */
	char *_Nullable path;			/* Unresolved object path */
	struct ag_object *_Nullable obj;	/* Resolved object */
	Uint32 count;				/* Reference count */
	AG_TAILQ_ENTRY(ag_object_dep) deps;
} AG_ObjectDep;

/* Object private data */
typedef struct ag_object_pvt {
	AG_TAILQ_ENTRY(ag_object) tobjs;	/* Entry in agTimerObjQ */
	/* TODO 1.6: store these as AG_Variables */
	AG_Event *_Nullable attachFn;		/* Attach hook */
	AG_Event *_Nullable detachFn;		/* Detach hook */
	_Nonnull_Mutex AG_Mutex lock;		/* General object lock */
} AG_ObjectPvt;

/* Object instance */
typedef struct ag_object {
	char name[AG_OBJECT_NAME_MAX];	/* Object ID (unique in parent) */
	/*
	 * XXX TODO 1.6: we can store archivePath and save_pfx as AG_Variables.
	 */
	char *_Nullable archivePath;	/* Application-specific archive path */
	char *_Nullable save_pfx;	/* Prefix for default save paths */
	AG_ObjectClass *_Nonnull cls;	/* Class description */
	Uint flags;
#define AG_OBJECT_FLOATING_VARS	 0x00001  /* Clear variables before load */
#define AG_OBJECT_NON_PERSISTENT 0x00002  /* Never include in saves */
#define AG_OBJECT_INDESTRUCTIBLE 0x00004  /* Not destructible (advisory) */
#define AG_OBJECT_RESIDENT	 0x00008  /* Data part is resident */
#define AG_OBJECT_PRESERVE_DEPS	 0x00010  /* Preserve cnt=0 dependencies */
#define AG_OBJECT_STATIC	 0x00020  /* Don't free() after detach */
#define AG_OBJECT_READONLY	 0x00040  /* Disallow edition (advisory) */
#define AG_OBJECT_WAS_RESIDENT	 0x00080  /* Used internally by ObjectLoad() */
#define AG_OBJECT_REOPEN_ONLOAD	 0x00200  /* Recreate editor UI on ObjectLoad() */
#define AG_OBJECT_REMAIN_DATA	 0x00400  /* Keep user data resident */
#define AG_OBJECT_DEBUG		 0x00800  /* Enable debugging */
#define AG_OBJECT_NAME_ONATTACH	 0x01000  /* Generate name on attach */
#define AG_OBJECT_CHLD_AUTOSAVE	 0x02000  /* Include child obj data in archive */
#define AG_OBJECT_DEBUG_DATA	 0x04000  /* Datafiles contain debug info */
#define AG_OBJECT_INATTACH	 0x08000  /* In AG_ObjectAttach() */
#define AG_OBJECT_INDETACH	 0x10000  /* In AG_ObjectDetach() */
#define AG_OBJECT_BOUND_EVENTS	 0x20000  /* Generate "bound" events whenever
					     AG_Bind*() is invoked */
#define AG_OBJECT_SAVED_FLAGS	(AG_OBJECT_FLOATING_VARS|\
 				 AG_OBJECT_INDESTRUCTIBLE|\
				 AG_OBJECT_PRESERVE_DEPS|\
				 AG_OBJECT_READONLY|\
				 AG_OBJECT_REOPEN_ONLOAD|\
				 AG_OBJECT_REMAIN_DATA|\
				 AG_OBJECT_DEBUG|\
				 AG_OBJECT_BOUND_EVENTS)

	AG_TAILQ_HEAD_(ag_event) events;	/* Event handlers/virtual fns */
	AG_TAILQ_HEAD_(ag_timer) timers;	/* Running timers (read-only or
						   R/W under AG_LockTiming()) */
	AG_TAILQ_HEAD_(ag_variable) vars;	/* Named variables / bindings */
	/*
	 * TODO 1.6: represent deps as AG_Variables (of P_OBJECT type) and
	 * remove this list entirely.
	 */
	AG_TAILQ_HEAD_(ag_object_dep) deps;	/* Object dependencies */
	struct ag_objectq children;		/* Child objects */
	AG_TAILQ_ENTRY(ag_object) cobjs;	/* Entry in parent */

	void *_Nullable parent;			/* Parent object (or NULL = is VFS root) */
	void *_Nonnull root;			/* Pointer to VFS root (possibly self) */

	AG_ObjectPvt pvt;			/* Private data */
} AG_Object;

/* Object archive header information. */
typedef struct ag_object_header {
	AG_ObjectClassSpec cs;			/* Class specification */
	Uint32 dataOffs;			/* Dataset offset */
	AG_Version ver;				/* AG_Object version */
	Uint flags;				/* Object flags */
} AG_ObjectHeader;

/* Iterate over the direct child objects. */
#define AGOBJECT_FOREACH_CHILD(var, ob, t) \
	for((var) = (struct t *)AG_TAILQ_FIRST(&AGOBJECT(ob)->children); \
	    (var) != (struct t *)AG_TAILQ_END(&AGOBJECT(ob)->children); \
	    (var) = (struct t *)AG_TAILQ_NEXT(AGOBJECT(var), cobjs))

/* Return next entry in list of direct child objects. */
#define AGOBJECT_NEXT_CHILD(var,t) \
	((struct t *)AG_TAILQ_NEXT(AGOBJECT(var),cobjs))

/* Return last entry in list of direct child objects. */
#define AGOBJECT_LAST_CHILD(var,t) \
	((struct t *)AG_TAILQ_LAST(&AGOBJECT(var)->children,ag_objectq))
	
/* Iterate over the direct child objects (reverse order). */
#define AGOBJECT_FOREACH_CHILD_REVERSE(var, ob, t) \
	for((var) = (struct t *)AG_TAILQ_LAST(&AGOBJECT(ob)->children, \
	    ag_objectq); \
	    (var) != (struct t *)AG_TAILQ_END(&AGOBJECT(ob)->children); \
	    (var) = (struct t *)AG_TAILQ_PREV(AGOBJECT(var), ag_objectq, \
	    cobjs))

/* Iterate over the direct child objects (matching a specified class). */
#define AGOBJECT_FOREACH_CLASS(var, ob, t, subclass) \
	AGOBJECT_FOREACH_CHILD(var,ob,t) \
		if (!AG_OfClass(var,(subclass))) { \
			continue; \
		} else

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_CORE)
# define OBJECT(ob)              AGOBJECT(ob)
# define OBJECT_CLASS(ob)        AGOBJECT_CLASS(ob)
# define CLASS(ob)               AGCLASS(ob)
# define OBJECT_RESIDENT(ob)    (AGOBJECT(ob)->flags & AG_OBJECT_RESIDENT)
# define OBJECT_PERSISTENT(ob) !(AGOBJECT(ob)->flags & AG_OBJECT_NON_PERSISTENT)
# define OBJECT_DEBUG(ob)       (AGOBJECT(ob)->flags & AG_OBJECT_DEBUG)

# define OBJECT_FOREACH_CHILD(var,ob,t)          AGOBJECT_FOREACH_CHILD((var),(ob),t)
# define OBJECT_FOREACH_CHILD_REVERSE(var,ob,t)  AGOBJECT_FOREACH_CHILD_REVERSE((var),(ob),t)
# define OBJECT_FOREACH_CLASS(var,ob,t,subclass) AGOBJECT_FOREACH_CLASS((var),(ob),t,(subclass))
# define OBJECT_NEXT_CHILD(var,t)                AGOBJECT_NEXT_CHILD((var),t)
# define OBJECT_LAST_CHILD(var,t)                AGOBJECT_LAST_CHILD((var),t)
#endif /* _AGAR_INTERNAL or _USE_AGAR_CORE */

struct ag_db;
struct ag_dbt;

__BEGIN_DECLS
extern AG_ObjectClass agObjectClass;		/* Generic Object class */

void *_Nullable AG_ObjectNew(void *_Nullable, const char *_Nullable,
                             AG_ObjectClass *_Nonnull);

void AG_ObjectAttach(void *_Nullable _Restrict, void *_Nonnull _Restrict);

int  AG_ObjectAttachToNamed(void *_Nonnull, const char *_Nonnull,
                            void *_Nonnull);

void AG_ObjectInit(void *_Nonnull _Restrict, void *_Nullable _Restrict);
void AG_ObjectInitStatic(void *_Nonnull _Restrict, void *_Nullable _Restrict);
void AG_ObjectInitNamed(void *_Nonnull _Restrict, void *_Nonnull _Restrict,
                        const char *_Nullable);

void AG_ObjectDetach(void *_Nonnull);
void AG_ObjectReset(void *_Nonnull);
void AG_ObjectRemain(void *_Nonnull, Uint);

char *_Nullable AG_ObjectGetName(void *_Nonnull);
int AG_ObjectCopyName(void *_Nonnull, char *_Nonnull, AG_Size);
int AG_ObjectCopyDirname(void *_Nonnull, char *_Nonnull, AG_Size);
int AG_ObjectCopyFilename(void *_Nonnull, char *_Nonnull, AG_Size);

int AG_ObjectChanged(void *_Nonnull);
int AG_ObjectChangedAll(void *_Nonnull);

void *_Nullable AG_ObjectFindS(void *_Nonnull, const char *_Nonnull)
                              _Pure_Attribute_If_Unthreaded;

void *_Nullable AG_ObjectFind(void *_Nonnull, const char *_Nonnull, ...)
                             FORMAT_ATTRIBUTE(printf,2,3)
			     _Pure_Attribute_If_Unthreaded;

void *_Nullable AG_ObjectFindParent(void *_Nonnull, const char *_Nonnull,
				    const char *_Nonnull);

int AG_ObjectInUse(void *_Nonnull) _Pure_Attribute_If_Unthreaded;

void AG_ObjectSetNameS(void *_Nonnull, const char *_Nonnull);
void AG_ObjectSetName(void *_Nonnull, const char *_Nullable, ...)
                     FORMAT_ATTRIBUTE(printf,2,3);

void AG_ObjectSetArchivePath(void *_Nonnull, const char *_Nonnull);
void AG_ObjectGetArchivePath(void *_Nonnull, char *_Nonnull, AG_Size);

void AG_ObjectSetClass(void *_Nonnull, void *_Nonnull);

void AG_ObjectSetAttachFn(void *_Nonnull,
                          void (*_Nullable fn)(AG_Event *_Nonnull),
			  const char *_Nullable, ...);

void AG_ObjectSetDetachFn(void *_Nonnull,
                          void (*_Nullable fn)(AG_Event *_Nonnull),
			  const char *_Nullable, ...);

void AG_ObjectMoveUp(void *_Nonnull);
void AG_ObjectMoveDown(void *_Nonnull);
void AG_ObjectMoveToHead(void *_Nonnull);
void AG_ObjectMoveToTail(void *_Nonnull);

void AG_ObjectDestroy(void *_Nonnull);
void AG_ObjectUnlinkDatafiles(void *_Nonnull);
void AG_ObjectSetSavePfx(void *_Nonnull, char *_Nullable);

void AG_ObjectFreeVariables(void *_Nonnull);
void AG_ObjectFreeChildren(void *_Nonnull);
void AG_ObjectFreeEvents(AG_Object *_Nonnull);
void AG_ObjectFreeDeps(AG_Object *_Nonnull);
void AG_ObjectFreeDummyDeps(AG_Object *_Nonnull);

int AG_ObjectPageIn(void *_Nonnull);
int AG_ObjectPageOut(void *_Nonnull);

int AG_ObjectSerialize(void *_Nonnull, AG_DataSource *_Nonnull);
int AG_ObjectUnserialize(void *_Nonnull, AG_DataSource *_Nonnull);

int  AG_ObjectSave(void *_Nonnull);
int  AG_ObjectSaveToFile(void *_Nonnull, const char *_Nullable);
int  AG_ObjectSaveToDB(void *_Nonnull, struct ag_db *_Nonnull,
                       const struct ag_dbt *_Nonnull);
int  AG_ObjectSaveAll(void *_Nonnull);
void AG_ObjectSaveVariables(void *_Nonnull, AG_DataSource *_Nonnull);

int AG_ObjectLoad(void *_Nonnull);
int AG_ObjectLoadFromFile(void *_Nonnull, const char *_Nullable);
int AG_ObjectLoadFromDB(void *_Nonnull, struct ag_db *_Nonnull,
                        const struct ag_dbt *_Nonnull);
int AG_ObjectLoadData(void *_Nonnull, int *_Nonnull);
int AG_ObjectLoadDataFromFile(void *_Nonnull, int *_Nonnull, const char *_Nullable);
int AG_ObjectLoadGeneric(void *_Nonnull);
int AG_ObjectLoadGenericFromFile(void *_Nonnull, const char *_Nullable);
int AG_ObjectResolveDeps(void *_Nonnull);
int AG_ObjectReadHeader(AG_DataSource *_Nonnull, AG_ObjectHeader *_Nonnull);
int AG_ObjectLoadVariables(void *_Nonnull, AG_DataSource *_Nonnull);

AG_ObjectDep *_Nonnull AG_ObjectAddDep(void *_Nonnull, void *_Nonnull, int);

int     AG_ObjectFindDep(void *_Nonnull, Uint32, void *_Nonnull *_Nullable);
void    AG_ObjectDelDep(void *_Nonnull, const void *_Nonnull);
Uint32  AG_ObjectEncodeName(void *_Nonnull, const void *_Nullable)
                           _Pure_Attribute_If_Unthreaded;

void AG_ObjectGenName(void *_Nonnull, AG_ObjectClass *_Nonnull, char *_Nonnull,
                      AG_Size);
void AG_ObjectGenNamePfx(void *_Nonnull, const char *_Nonnull, char *_Nonnull,
                         AG_Size);

#define AG_OfClass(obj,cspec) AG_ClassIsNamed(AGOBJECT(obj)->cls,(cspec))
#define AG_ObjectRoot(ob)   (AGOBJECT(ob)->root)
#define AG_ObjectParent(ob) (AGOBJECT(ob)->parent)

#ifdef AG_THREADS
# define AG_ObjectLock(ob)   AG_MutexLock(&AGOBJECT(ob)->pvt.lock)
# define AG_ObjectUnlock(ob) AG_MutexUnlock(&AGOBJECT(ob)->pvt.lock)
# define AG_LockVFS(ob)      AG_ObjectLock(AGOBJECT(ob)->root)
# define AG_UnlockVFS(ob)    AG_ObjectUnlock(AGOBJECT(ob)->root)
#else /* !AG_THREADS */
# define AG_ObjectLock(ob)
# define AG_ObjectUnlock(ob)
# define AG_LockVFS(ob)
# define AG_UnlockVFS(ob)
#endif /* AG_THREADS */

/*
 * Inlinables
 */
void                      ag_object_delete(void *_Nonnull);
void *_Nullable           ag_object_find_child(void *_Nonnull, const char *_Nonnull)
                                              _Pure_Attribute_If_Unthreaded;
AG_ObjectClass *_Nullable ag_object_superclass(void *_Nonnull)
                                              _Pure_Attribute;
void                      ag_lock_timers(void *_Nullable);
void                      ag_unlock_timers(void *_Nullable);
int                       ag_defined(void *_Nonnull, const char *_Nonnull)
                                    _Pure_Attribute
                                    _Warn_Unused_Result;
AG_Variable *_Nonnull     ag_fetch_variable(void *_Nonnull, const char *_Nonnull,
                                            enum ag_variable_type)
                                           _Warn_Unused_Result;
AG_Variable *_Nonnull     ag_fetch_variable_of_type(void *_Nonnull,
                                                    const char *_Nonnull,
                                                    enum ag_variable_type)
                                                   _Warn_Unused_Result;
AG_Variable *_Nullable    ag_access_variable(void *_Nonnull, const char *_Nonnull)
                                            _Warn_Unused_Result;
void *_Nonnull            ag_get_named_object(AG_Event *_Nonnull,
                                              const char *_Nonnull,
                                              const char *_Nonnull)
				             _Pure_Attribute
				             _Warn_Unused_Result;
#ifdef AG_INLINE_OBJECT
# define AG_INLINE_HEADER
# include <agar/core/inline_object.h>
#else
# define AG_ObjectDelete(o)            ag_object_delete(o)
# define AG_ObjectFindChild(o,n)       ag_object_find_child((o),(n))
# define AG_ObjectSuperclass(o)        ag_object_superclass(o)
# define AG_LockTimers(o)              ag_lock_timers(o)
# define AG_UnlockTimers(o)            ag_unlock_timers(o)
# define AG_Defined(o,n)               ag_defined((o),(n))
# define AG_FetchVariable(o,n,t)       ag_fetch_variable((o),(n),(t))
# define AG_FetchVariableOfType(o,n,t) ag_fetch_variable_of_type((o),(n),(t))
# define AG_AccessVariable(o,n)        ag_access_variable((o),(n))
# define AG_GetNamedObject(e,k,s)      ag_get_named_object((e),(k),(s))
#endif /* !AG_INLINE_OBJECT */
__END_DECLS
#ifdef AG_LEGACY
#include <agar/core/legacy_object.h>
#endif
#include <agar/core/close.h>
#endif /* _AGAR_CORE_OBJECT_H_ */
