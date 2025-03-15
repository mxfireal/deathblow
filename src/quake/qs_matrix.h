
#if defined(mat4)
//if something else defines mat4 and you decide to use this header dont cause weird issues
#error mat4 already defined??
#endif
#define mat4 float *
#define mat4_t(v) float v[4 * 4]

void MatrixIdentity(mat4 m);
void MatrixMultiply(mat4 out, const mat4 a, const mat4 b);
void MatrixTranslate(mat4 m, float x, float y, float z);
void MatrixRotate(mat4 m, float angle, float x, float y, float z);
void MatrixRotate_OnePass(mat4 m, float x, float y, float z);
void MatrixSetFrustum(mat4 m, float fovX, float fovY);
void MatrixSetFrustum_AspectFOV(mat4 m, float aspect,float fov);

void MatrixScale(mat4 m,float sx,float sy,float sz);