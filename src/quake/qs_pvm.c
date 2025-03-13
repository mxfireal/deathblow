#include "qs_pvm.h"
#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#include <math.h>

extern client_state_t cl;    // in cl_main.c, for getting viewangles
extern refdef_t r_refdef;    // in gl_main.c, use instead of cl
extern float r_fovx, r_fovy; // in gl_main.c

#define mat4 float *
#define mat4_t(v) float v[4 * 4]

void perspective_matrix(float fov, float aspect, float near, float far, mat4 matrix)
{
        float f = 1.0f / tanf(fov * 0.5f);

        matrix[0] = f / aspect;
        matrix[1] = 0.0f;
        matrix[2] = 0.0f;
        matrix[3] = 0.0f;

        matrix[4] = 0.0f;
        matrix[5] = f;
        matrix[6] = 0.0f;
        matrix[7] = 0.0f;

        matrix[8] = 0.0f;
        matrix[9] = 0.0f;
        matrix[10] = (far + near) / (near - far);
        matrix[11] = -1.0f;

        matrix[12] = 0.0f;
        matrix[13] = 0.0f;
        matrix[14] = (2.0f * far * near) / (near - far);
        matrix[15] = 0.0f;
}

// for converting degrees into radians
static const float pidiv180 = (M_PI / 180.0f);

#define quickcvar(vName, vDefaultValue)

/*
1 2 3
2 0 1
1 0 2
3 2 1
0 2 1
*/

void viewmatrix(float *matrix, float *viewangles, float *vieworg)
{
        float pitch = -viewangles[PITCH];
        float yaw = -viewangles[YAW];
        float roll = -viewangles[ROLL];
        float sx = sinf(pitch * (M_PI / 180.0f)); // Pitch (X)
        float cx = cosf(pitch * (M_PI / 180.0f));
        float sy = sinf(yaw * (M_PI / 180.0f)); // Yaw (Y)
        float cy = cosf(yaw * (M_PI / 180.0f));
        float sz = sinf(roll * (M_PI / 180.0f)); // Roll (Z)
        float cz = cosf(roll * (M_PI / 180.0f));

        // quake uses z for up, so align this to work with gl where -y is up
        float rotX_90[16] = {
            1, 0, 0, 0,
            0, 0, -1, 0,
            0, 1, 0, 0,
            0, 0, 0, 1};
            
        float rotZ_90[16] = {
            0, -1, 0, 0,
            1, 0, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1};

        // view rotation matrix
        float viewRot[16] = {
            cy * cz, -cy * sz, sy, 0,
            sx * sy * cz + cx * sz, -sx * sy * sz + cx * cz, -sx * cy, 0,
            -cx * sy * cz + sx * sz, cx * sy * sz + sx * cz, cx * cy, 0,
            0, 0, 0, 1};

        // view translation
        float translation[16] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            -vieworg[0], -vieworg[1], -vieworg[2], 1};

        // multiply: identity * rotX_90 * rotZ_90 * viewRot * translation
        float temp1[16], temp2[16];
        multiply_matrices(temp1, rotX_90, rotZ_90);
        multiply_matrices(temp2, temp1, viewRot);
        multiply_matrices(matrix, temp2, translation);
}

// Function to multiply two 4x4 matrices
void multiply_matrices(float *result, const float *a, const float *b)
{
        for (int i = 0; i < 4; i++)
        {
                for (int j = 0; j < 4; j++)
                {
                        result[i * 4 + j] =
                            a[i * 4 + 0] * b[0 * 4 + j] +
                            a[i * 4 + 1] * b[1 * 4 + j] +
                            a[i * 4 + 2] * b[2 * 4 + j] +
                            a[i * 4 + 3] * b[3 * 4 + j];
                }
        }
}

void mat4_makeidentity(float *matrix)
{
        memset(matrix, 0, 16 * sizeof(float));
        matrix[0] = 1.0f;
        matrix[5] = 1.0f;
        matrix[10] = 1.0f;
        matrix[15] = 1.0f;
}

typedef float *vec3_ptr; // confusing

// todo: dont calculate stuff we dont need. check if the uniform exists b4 doing any math
void qspvm_apply(vec3_ptr model)
{
        mat4_t(model_mat4);
        mat4_t(view_mat4);
        mat4_t(perspective_mat4);

        vec3_ptr view_angles = r_refdef.viewangles;
        vec3_ptr view_origin = r_refdef.vieworg;

        // havent gotten the actual values yet. sorry!
        float fov = 90.0f;
        float aspect = 4.0 / 3.0;
        float near = 0.1;
        float far = 20000;
        perspective_matrix(fov, aspect, near, far, perspective_mat4);

        viewmatrix(view_mat4, view_angles, view_origin);

        mat4_makeidentity(model_mat4);
        model_mat4[12] = model[0]; // x
        model_mat4[13] = model[2]; // z (up)
        model_mat4[14] = model[1]; // y

        GLint prog = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

#define gl_passm4(vMAT4, vUNIFORMNAME) glUniformMatrix4fv(glGetUniformLocation(prog, vUNIFORMNAME), 1, 0, vMAT4);
        // alright. pass to the shader and we're done
        gl_passm4(model_mat4, "u_mModel");
        gl_passm4(view_mat4, "u_mView");
        gl_passm4(perspective_mat4, "u_mProjection");
}

void QSPVM_Apply_FromTextureChainsGLSL(qmodel_t *model, entity_t *ent, texchain_t chain)
{
        entity_t *player = &cl_entities[cl.viewentity];
        if (!player)
                return;

        vec3_t empty_origin = {0, 0, 0};

        // note: for worldspawn, ent will be null
        //(model)
        vec3_ptr v_model = ent ? ent->origin : empty_origin;

        qspvm_apply(v_model);
}

void QSPVM_Apply_FromDrawAliasFrameGLSL(aliashdr_t *paliashdr, lerpdata_t lerpdata, gltexture_t *tx, gltexture_t *fb)
{
        vec3_t model_pos_blend;
        vec3_ptr v_model;
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
        qspvm_apply(v_model);
}
