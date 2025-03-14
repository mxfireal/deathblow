#include "qs_pvm.h"
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#include <math.h>
#include "qs_matrix.h"
extern client_state_t cl;    // in cl_main.c, for getting viewangles
extern refdef_t r_refdef;    // in gl_main.c, use instead of cl
extern float r_fovx, r_fovy; // in gl_main.c

typedef float *vec3_ptr; // confusing

// name is a lie. only sets view matrix now
void SetModelViewMatrix(mat4 out, int is_worldspawn)
{
        mat4_t(m);
        MatrixIdentity(m);
        /*

                X, Y, Z
                X -> Yaw
                Y -> Pitch
                Z -> Roll

        */
        int fn = is_worldspawn ? 90 : 0;

        // MatrixRotate(m, -90, 1, 0, 0); //Not needed anymore
        // MatrixRotate(m, 90, 0, 0, 1); //Not sure what this does lol

        MatrixRotate_OnePass(
            m,
            r_refdef.viewangles[PITCH] + 90,
            r_refdef.viewangles[YAW]  - fn,
            -r_refdef.viewangles[ROLL]);
        MatrixTranslate(m, -r_refdef.vieworg[0], -r_refdef.vieworg[2], r_refdef.vieworg[1]); // gl has -z as forward
        // MatrixTranslate(m, 0,-cl.viewheight,0);

        memcpy(out, m, sizeof(float) * 16);
}

// todo: dont calculate stuff we dont need. check if the uniform exists b4 doing any math
void qspvm_apply(vec3_ptr model, vec3_ptr angles, int is_worldspawn)
{
        mat4_t(model_mat4);
        mat4_t(view_mat4);
        mat4_t(perspective_mat4);

        vec3_ptr view_angles = r_refdef.viewangles;
        vec3_ptr view_origin = r_refdef.vieworg;

        // placeholder
        float fov = 90.0f;
        float aspect = 4.0 / 3.0;

        MatrixIdentity(perspective_mat4);
        MatrixSetFrustum_AspectFOV(perspective_mat4, aspect, fov);

        MatrixIdentity(view_mat4);
        SetModelViewMatrix(view_mat4, is_worldspawn);

        MatrixIdentity(model_mat4);
        MatrixRotate_OnePass(
                model_mat4,
                angles[PITCH],
                -angles[YAW],
                angles[ROLL]);
        MatrixTranslate(model_mat4, model[0], model[2], -model[1]);

        GLint prog = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

#define gl_passm4(vMAT4, vUNIFORMNAME) glUniformMatrix4fv(glGetUniformLocation(prog, vUNIFORMNAME), 1, 0, vMAT4);
        // alright. pass to the shader and we're done
        gl_passm4(model_mat4, "u_mModel");
        gl_passm4(view_mat4, "u_mView");
        gl_passm4(perspective_mat4, "u_mProjection");
}

// this gets passed from WORLDSPAWN

void QSPVM_Apply_FromTextureChainsGLSL(qmodel_t *model, entity_t *ent, texchain_t chain)
{
        vec3_t empty_origin = {0, 0, 0};

        vec3_ptr v_model = empty_origin;
        vec3_ptr v_angles = empty_origin;

        qspvm_apply(v_model, v_angles, 1);
}

// this gets passed from entities
void QSPVM_Apply_FromDrawAliasFrameGLSL(aliashdr_t *paliashdr, lerpdata_t lerpdata, gltexture_t *tx, gltexture_t *fb)
{
        vec3_t model_pos_blend;
        vec3_ptr v_model;
        vec3_ptr v_angles;
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
        v_angles = lerpdata.angles;
        qspvm_apply(v_model, v_angles, 0);
}
