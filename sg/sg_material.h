/*	Public domain	*/

typedef struct sg_texture {
	SDL_Surface *su;			/* Source surface */
	SG_Real s, t;				/* Texture coordinates */
	TAILQ_ENTRY(sg_texture) textures;
} SG_Texture;
	
enum sg_blend_mode {
	SG_BLEND_ONE,
	SG_BLEND_ZERO,
	SG_BLEND_SRC_COLOR,
	SG_BLEND_DST_COLOR,
	SG_BLEND_ONE_MINUS_SRC_COLOR,
	SG_BLEND_ONE_MINUS_DST_COLOR,
	SG_BLEND_SRC_ALPHA,
	SG_BLEND_DST_ALPHA,
	SG_BLEND_ONE_MINUS_SRC_ALPHA,
	SG_BLEND_ONE_MINUS_DST_ALPHA
};
enum sg_blend_combination {
	SG_BLEND_OPAQUE,	/* src=ONE, dst=ZERO */
	SG_BLEND_ADDITIVE,	/* src=ONE, dst=ONE */
	SG_BLEND_MODULATED,	/* src=DST_COLOR, dst=ZERO */
	SG_BLEND_ALPHA_MASK	/* src=SRC_ALPHA, dst=ONE_MINUS_SRC_ALPHA */
};

typedef struct sg_material {
	struct ag_object obj;
	Uint flags;
#define SG_MATERIAL_NOLIGHT	0x01	/* Disable lighting for this material */

	/* For fixed lighting model */
	SG_Color emissive;		/* Light emitted by surface (Ke) */
	SG_Color ambient;		/* Ambient reflectance (Ka) */
	SG_Color diffuse;		/* Diffuse reflectance (Kd) */
	SG_Color specular;		/* Specular reflectance (Ks) */
	SG_Real shininess;		/* Specular exponent */
	enum sg_blend_mode blend_src;	/* Blending mode for source fragment */
	enum sg_blend_mode blend_dst;	/* Blending mode for target fragment */

	TAILQ_HEAD(,sg_texture) textures;	/* Textures to apply */
	TAILQ_HEAD(,sg_program) progs;		/* Vertex/fragment shaders */
} SG_Material;

__BEGIN_DECLS
SG_Material	*SG_MaterialNew(void *, const char *);
void		 SG_MaterialInit(void *, const char *);
void		 SG_MaterialReinit(void *);
void		 SG_MaterialDestroy(void *);
void		*SG_MaterialEdit(void *);
int		 SG_MaterialLoad(void *, AG_Netbuf *);
int		 SG_MaterialSave(void *, AG_Netbuf *);

void		 SG_MaterialBind(SG_Material *, SG_View *);
void		 SG_MaterialUnbind(SG_Material *, SG_View *);

SG_Texture	*SG_TextureFromSurface(SDL_Surface *);
__END_DECLS
