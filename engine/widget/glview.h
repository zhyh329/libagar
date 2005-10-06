/*	$Csoft: glview.h,v 1.1 2005/10/04 18:04:47 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_GLVIEW_H_
#define _AGAR_WIDGET_GLVIEW_H_

#include <config/have_opengl.h>

#ifdef HAVE_OPENGL

#include <engine/widget/widget.h>
#include "begin_code.h"

typedef struct ag_glview {
	struct ag_widget wid;
	u_int flags;
#define AG_GLVIEW_WFILL		0x01
#define AG_GLVIEW_HFILL		0x02
#define AG_GLVIEW_NOMODELVIEW	0x04	/* Don't preserve modelview matrix */
#define AG_GLVIEW_NOTEXTURE	0x08	/* Don't preserve texture matrix */
#define AG_GLVIEW_NOCOLOR	0x10	/* Don't preserve color matrix */

	AG_Event *draw_ev;			/* Draw callback */
	AG_Event *scale_ev;			/* Scaling/movement event */
	AG_Event *keydown_ev, *keyup_ev;	/* Keyboard events */
	AG_Event *btndown_ev, *btnup_ev;	/* Mouse button events */
	AG_Event *motion_ev;			/* Mouse motion event */

	GLdouble mProjection[16];		/* Projection matrix to load */
	GLdouble mModelview[16];		/* Modelview matrix to load */
	GLdouble mTexture[16];			/* Texture matrix to load */
	GLdouble mColor[16];			/* Color matrix to load */
} AG_GLView;

__BEGIN_DECLS
AG_GLView *AG_GLViewNew(void *, u_int);
void	   AG_GLViewInit(AG_GLView *, u_int);
void	   AG_GLViewDestroy(void *);
void	   AG_GLViewDraw(void *);
void	   AG_GLViewScale(void *, int, int);
void	   AG_GLViewReshape(AG_GLView *);
void	   AG_GLViewDrawFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewScaleFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewKeydownFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewKeyupFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewButtondownFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewButtonupFn(AG_GLView *, AG_EventFn, const char *, ...);
void	   AG_GLViewMotionFn(AG_GLView *, AG_EventFn, const char *, ...);
__END_DECLS

#endif /* HAVE_OPENGL */
#include "close_code.h"
#endif /* _AGAR_WIDGET_GLVIEW_H_ */
