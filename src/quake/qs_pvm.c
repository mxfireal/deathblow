#include "qs_pvm.h"
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#include <math.h>
#include "qs_matrix.h"
extern client_state_t cl;    // in cl_main.c, for getting viewangles
extern refdef_t r_refdef;    // in gl_main.c, use instead of cl
extern float r_fovx, r_fovy; // in gl_main.c

// note: for a screen that is 4 / 3, the aspect ratio may not actually be 4 / 3,
// the status bar or other whatever i dont know messes things up sorry

typedef float *vec3_ptr;

int cv_setuped=0;
cvar_t cv_modelscale = {"sau_modelscale","1",0};
cvar_t cv_modelrot_x = {"sau_modelrot_x","0",0};
cvar_t cv_modelrot_y = {"sau_modelrot_y","0",0};
cvar_t cv_modelrot_z = {"sau_modelrot_z","0",0};
void checkcvars()
{
        if (cv_setuped) return;
        cv_setuped=1;
        Cvar_RegisterVariable(&cv_modelscale);
        Cvar_RegisterVariable(&cv_modelrot_x);
        Cvar_RegisterVariable(&cv_modelrot_y);
        Cvar_RegisterVariable(&cv_modelrot_z);
}

// name is a lie. only sets view matrix now
void SetModelViewMatrix(mat4 out, int is_worldspawn)
{
        checkcvars();
        mat4_t(m);
        MatrixIdentity(m);
        /*

                X, Y, Z
                X -> Yaw
                Y -> Pitch
                Z -> Roll

        */
        int fn = is_worldspawn ? 90 : 90;

#if 0
        MatrixRotate(m, -90, 1, 0, 0); 
        MatrixRotate(m, 90, 0, 0, 1);
#endif

        MatrixTranslate(m, -r_refdef.vieworg[0], -r_refdef.vieworg[1], -r_refdef.vieworg[2]); // gl has -z as forward
        MatrixTranslate(m, 0, cl.viewheight, 0);

        float pitchrot = r_refdef.viewangles[PITCH];
        float yawrot = -(r_refdef.viewangles[YAW] - fn);
        float rollrot = r_refdef.viewangles[ROLL];

#if 0
        MatrixRotate_OnePass(
            m,
            -r_refdef.viewangles[PITCH] + 90,
            r_refdef.viewangles[YAW]  - fn,
            -r_refdef.viewangles[ROLL]);
#else
        MatrixRotate(m, -90, 1, 0, 0);
        MatrixTranslate(m, 0, 0, 25); // manual touchup
        MatrixRotate(m, yawrot, 0, 1, 0);
        MatrixRotate(m, pitchrot, 1, 0, 0);
        MatrixRotate(m, rollrot, 0, 0, 1);
#endif

        memcpy(out, m, sizeof(float) * 16);
}

#define STANDARD_PERSPECTIVE_NAME "u_mProjection"
#define STANDARD_VIEW_NAME "u_mView"
#define STANDARD_MODEL_NAME "u_mModel"
#define gl_passm4_old(vMAT4, vUNIFORMNAME) glUniformMatrix4fv(glGetUniformLocation(prog, vUNIFORMNAME), 1, 0, vMAT4);
#define gl_passm4(vMAT4, vLOC) glUniformMatrix4fv(vLOC, 1, 0, vMAT4);

// todo: dont calculate stuff we dont need. check if the uniform exists b4 doing any math
void qspvm_apply(vec3_ptr model, vec3_ptr angles, int is_worldspawn,vec3_ptr model_scale)
{
        mat4_t(model_mat4);
        mat4_t(view_mat4);
        mat4_t(perspective_mat4);

        int do_perspective, do_view, do_model;
        GLint prog = 0;
        vec3_ptr view_angles = r_refdef.viewangles;
        vec3_ptr view_origin = r_refdef.vieworg;

        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        do_perspective = glGetUniformLocation(prog, STANDARD_PERSPECTIVE_NAME);
        do_view = glGetUniformLocation(prog, STANDARD_VIEW_NAME);
        do_model = glGetUniformLocation(prog, STANDARD_MODEL_NAME);

        if (do_perspective != -1)
        {
                // perspective matrix
                MatrixIdentity(perspective_mat4);
                MatrixSetFrustum(perspective_mat4, r_fovx, r_fovy);
                gl_passm4(perspective_mat4, do_perspective);
        }

        if (do_view != -1)
        {
                // view matrix (the camera)
                MatrixIdentity(view_mat4);
                SetModelViewMatrix(view_mat4, is_worldspawn);
                gl_passm4(view_mat4, do_view);
        }

        if (do_model != -1)
        { 
                // model matrix (if applicable)
                MatrixIdentity(model_mat4);

                if (model_scale && cv_modelscale.value)
                {
                        MatrixScale(model_mat4,model_scale[0],model_scale[1],model_scale[2]);
                }


                MatrixRotate(model_mat4, cv_modelrot_x.value, 1, 0, 0);
                MatrixRotate(model_mat4, cv_modelrot_y.value, 0, 1, 0);
                MatrixRotate(model_mat4, cv_modelrot_z.value, 0, 0, 1);

                float ryaw = -angles[YAW];
                float rpitch = angles[PITCH];
                float rroll = angles[ROLL];

                MatrixRotate(model_mat4, ryaw, 0, 1, 0);
                MatrixRotate(model_mat4, rpitch, 0, 0, 1);
                MatrixRotate(model_mat4, rroll, 1, 0, 0);

                MatrixTranslate(model_mat4, model[0], model[1], model[2]);

                gl_passm4(model_mat4, do_model);
        }
}

// this gets passed from WORLDSPAWN

void QSPVM_Apply_FromTextureChainsGLSL(qmodel_t *model, entity_t *ent, texchain_t chain)
{
        vec3_t empty_origin = {0, 0, 0};

        vec3_ptr v_model = empty_origin;
        vec3_ptr v_angles = empty_origin;

        qspvm_apply(v_model, v_angles, 1,0);
}

// this gets passed from entities
void QSPVM_Apply_FromDrawAliasFrameGLSL(aliashdr_t *paliashdr, lerpdata_t lerpdata, gltexture_t *tx, gltexture_t *fb)
{
        vec3_ptr v_model;
        vec3_ptr v_angles;
        #if 0
        vec3_t model_pos_blend;
        // have to recalculate this :P
        float blend;
        if (lerpdata.pose1 != lerpdata.pose2) // need to blend
        {
                v_model = model_pos_blend;
                blend = lerpdata.blend;
                // not implemented yet. sorry
                // im planning on getting around to it though dw dog
                v_model = lerpdata.origin;
        }
        else
        {
                blend = 0;
                v_model = lerpdata.origin;
        }
                #endif
        v_model = lerpdata.origin; //paliashdr->scale_origin;
        v_angles = lerpdata.angles;
        qspvm_apply(v_model, v_angles, 0,paliashdr->scale);
}
