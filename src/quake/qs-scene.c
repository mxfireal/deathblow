#include "qs-scene.h"
#include "quakedef.h"
#include "glquake.h"

extern GLuint gl_bmodel_vbo;

#define vertAttrIndex 0
#define texCoordsAttrIndex 1
#define LMCoordsAttrIndex 2

extern GLint texLoc;
extern GLint LMTexLoc;
extern GLint fullbrightTexLoc;
extern GLint useFullbrightTexLoc;
extern GLint useOverbrightLoc;
extern GLint useAlphaTestLoc;
extern GLint useLightmapWideLoc;
extern GLint useLightmapOnlyLoc;
extern GLint alphaLoc;

//use quake's effects for teleporters by default, and the new fx for liquids
cvar_t cv_warpstyle = {"sau_warpstyle","0",0};
cvar_t cv_warpstyle_liquid = {"sau_warpstyle_liquid","1",0};
cvar_t cv_alias_dotstyle = {"sau_dotstyle","0",0};
cvar_t cv_alias_lightingstyle = {"sau_lightstyle","0",0};
cvar_t cv_alias_lightingscale = {"sau_lightscale","1.0",0};
cvar_t cv_alias_specularthreshold = {"sau_specularthreshold","-1",0};

void rcv()
{
        static int sdv = 0;
        if (sdv) return;
        Cvar_RegisterVariable(&cv_warpstyle);
        Cvar_RegisterVariable(&cv_warpstyle_liquid);
        Cvar_RegisterVariable(&cv_alias_dotstyle);
        Cvar_RegisterVariable(&cv_alias_lightingstyle);
        Cvar_RegisterVariable(&cv_alias_lightingscale);
        Cvar_RegisterVariable(&cv_alias_specularthreshold);
        sdv=1;
}

float getscale()
{
        extern cvar_t r_scale_uncapped;
        extern cvar_t r_scale;

        if (r_scale_uncapped.value)
                return r_scale.value;
        return CLAMP(1, (int)r_scale.value, 4);
}
int qsrc_programs[kQSRSCProgram_Count];
int GetProgram(int id)
{
        return qsrc_programs[id];
}

#define r_world_program GetProgram(kQSRSCProgramWorld)
#define r_water_program GetProgram(kQSRSCProgramWarp)
#define r_sky_program GetProgram(kQSRSCProgramSky)


void R_DrawEntitiesOnList(qboolean is_alphapass);

void QSRSC_RenderScene()
{
        rcv();
        float scale = getscale();
        glViewport(glx + r_refdef.vrect.x,
                   gly + glheight - r_refdef.vrect.y - r_refdef.vrect.height,
                   r_refdef.vrect.width / scale,
                   r_refdef.vrect.height / scale);

#if 0
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
#else
        glDisable(GL_CULL_FACE);
#endif

        glDisable(GL_BLEND);
        glDisable(GL_ALPHA_TEST);
        glEnable(GL_DEPTH_TEST);
        //(draw world)
        qsrc_DrawTextureChains(cl.worldmodel, NULL, chain_world);

        //(draw entities)
        R_DrawEntitiesOnList(false);
        R_DrawEntitiesOnList(true);
}

int get_uniform(int program, char *name)
{
        int uni = GL_GetUniformLocationFunc(program, name);
        return uni;
}



void qsrc_DrawTextureChains(qmodel_t *model, entity_t *ent, texchain_t chain)
{
        const float entalpha = (ent != NULL) ? ENTALPHA_DECODE(ent->alpha) : 1.0f;

        int i;
        msurface_t *s;
        texture_t *t;
        qboolean bound;
        int lastlightmap;
        gltexture_t *fullbright = NULL;

        // enable blending / disable depth writes
        if (entalpha < 1)
        {
                glDepthMask(GL_FALSE);
                glEnable(GL_BLEND);
        }

        GL_UseProgramFunc(r_world_program);

// set uniforms
#if 0
        GL_Uniform1iFunc(texLoc, 0);
        GL_Uniform1iFunc(LMTexLoc, 1);
        GL_Uniform1iFunc(fullbrightTexLoc, 2);
        GL_Uniform1iFunc(useFullbrightTexLoc, 0);
        GL_Uniform1iFunc(useOverbrightLoc, 0);
        GL_Uniform1iFunc(useAlphaTestLoc, 0);
        GL_Uniform1iFunc(useLightmapWideLoc, 0);
        GL_Uniform1iFunc(useLightmapOnlyLoc, 0);
        GL_Uniform1fFunc(alphaLoc, entalpha);
#endif

        
        int is_warp = 0;
        for (i = 0; i < model->numtextures; i++)
        {
                t = model->textures[i];

                if (!t || !t->texturechains[chain]) //|| t->texturechains[chain]->flags & (SURF_DRAWTILED))
                        continue;

                // GL_Uniform1iFunc(useFullbrightTexLoc, 0);
                int designated_program = r_world_program;

                int flags = t->texturechains[chain]->flags;
                is_warp= 0;
                if (flags & SURF_DRAWTURB)
                {
                        designated_program = r_water_program;
                        is_warp=1;
                }
                else if (flags & SURF_DRAWSKY)
                {
                        designated_program = r_sky_program;
                }

                GL_UseProgramFunc(designated_program);

                // Bind the buffers
                GL_BindBuffer(GL_ARRAY_BUFFER, gl_bmodel_vbo);
                GL_BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // indices come from client memory!

                GL_EnableVertexAttribArrayFunc(vertAttrIndex);
                GL_EnableVertexAttribArrayFunc(texCoordsAttrIndex);
                GL_EnableVertexAttribArrayFunc(LMCoordsAttrIndex);

                GL_VertexAttribPointerFunc(vertAttrIndex, 3, GL_FLOAT, GL_FALSE, VERTEXSIZE * sizeof(float), ((float *)0));
                GL_VertexAttribPointerFunc(texCoordsAttrIndex, 2, GL_FLOAT, GL_FALSE, VERTEXSIZE * sizeof(float), ((float *)0) + 3);
                GL_VertexAttribPointerFunc(LMCoordsAttrIndex, 2, GL_FLOAT, GL_FALSE, VERTEXSIZE * sizeof(float), ((float *)0) + 5);

                //yeah.. i know this needs to be cached in some way
                GL_Uniform1iFunc(get_uniform(designated_program, "Tex"), 0);
                GL_Uniform1iFunc(get_uniform(designated_program, "LMTex"), 1);
                GL_Uniform1iFunc(get_uniform(designated_program, "fullbrightTexLoc"), 2);
                //causes issues with warp shader
                GL_Uniform1iFunc(get_uniform(designated_program, "UseAlphaTest"), 0);

                int fbtex_loc = get_uniform(designated_program,"UseFullbrightTex");

                if (is_warp)
                {
                        int style = cv_warpstyle.value;
                        int isliquid = (flags & SURF_DRAWTELE) == 0;
                        if (isliquid && cv_warpstyle_liquid.value != -1) style = cv_warpstyle_liquid.value;
                        GL_Uniform1iFunc(get_uniform(designated_program, "m_uWarpStyle"), style);
                }

                //GL_Uniform1iFunc(fbtex_loc, 0);
                // do this last
                QSPVM_Apply_FromTextureChainsGLSL(model, ent, chain);

                R_ClearBatch();

                bound = false;
                lastlightmap = 0; // avoid compiler warning
                for (s = t->texturechains[chain]; s; s = s->texturechain)
                {
                        if (!bound) // only bind once we are sure we need this texture
                        {
                                GL_SelectTexture(GL_TEXTURE0);
                                GL_Bind((R_TextureAnimation(t, ent != NULL ? ent->frame : 0))->gltexture);
                                // if (t->texturechains[chain]->flags & SURF_DRAWFENCE)
                                //         GL_Uniform1iFunc(useAlphaTestLoc, 1); // Flip alpha test back on

                                bound = true;
                                lastlightmap = s->lightmaptexturenum;
                        }
                        //im ignoring gl_fullbrights here because haha 
                        if ((fullbright = R_TextureAnimation(t, ent != NULL ? ent->frame : 0)->fullbright))
                        {
                                GL_SelectTexture(GL_TEXTURE2);
                                GL_Bind(fullbright);
                                GL_Uniform1iFunc(fbtex_loc, 1);
                        }
                        else
                                GL_Uniform1iFunc(fbtex_loc, 0);

                        if (s->lightmaptexturenum != lastlightmap)
                                R_FlushBatch();

                        GL_SelectTexture(GL_TEXTURE1);
                        GL_Bind(lightmaps[s->lightmaptexturenum].texture);
                        lastlightmap = s->lightmaptexturenum;
                        R_BatchSurface(s);

                        rs_brushpasses++;
                }

                R_FlushBatch();

                // if (bound && t->texturechains[chain]->flags & SURF_DRAWFENCE)
                //         GL_Uniform1iFunc(useAlphaTestLoc, 0); // Flip alpha test back off
        }

        // clean up
        GL_DisableVertexAttribArrayFunc(vertAttrIndex);
        GL_DisableVertexAttribArrayFunc(texCoordsAttrIndex);
        GL_DisableVertexAttribArrayFunc(LMCoordsAttrIndex);

        GL_UseProgramFunc(0);
        GL_SelectTexture(GL_TEXTURE0);

        if (entalpha < 1)
        {
                glDepthMask(GL_TRUE);
                glDisable(GL_BLEND);
        }
}

void QSRSC_FromRenderAlias()
{
        rcv();
        GLint program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &program);

        GL_Uniform1iFunc(get_uniform(program, "u_mDotStyle"), cv_alias_dotstyle.value);
        GL_Uniform1iFunc(get_uniform(program, "u_mLightingStyle"), cv_alias_lightingstyle.value);
        GL_Uniform1fFunc(get_uniform(program, "u_mLightingScale"), cv_alias_lightingscale.value);
        GL_Uniform1fFunc(get_uniform(program, "u_mSpecularThres"), cv_alias_specularthreshold.value);
}