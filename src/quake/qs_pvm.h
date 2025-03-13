#include "quakedef.h"

#ifndef R_ALIAS_C
//redefining this, sorry
typedef struct {
	short pose1;
	short pose2;
	float blend;
	vec3_t origin;
	vec3_t angles;
} lerpdata_t;
#endif
void QSPVM_Apply_FromTextureChainsGLSL(qmodel_t *model, entity_t *ent, texchain_t chain);
void QSPVM_Apply_FromDrawAliasFrameGLSL(aliashdr_t *paliashdr, lerpdata_t lerpdata, gltexture_t *tx, gltexture_t *fb);