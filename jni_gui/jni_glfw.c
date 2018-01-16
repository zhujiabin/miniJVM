/* nuklear - 1.32.0 - public domain */
#define WIN32_LEAN_AND_MEAN

#include "GL/GL.h"
#include <stdio.h>
#include <string.h>
#include "deps/include/glad/glad.h"
#include "deps/include/GLFW/glfw3.h"
#include "deps/include/linmath.h"

#include "../mini_jvm/jvm/jvm.h"

typedef struct _GlobeRefer GlobeRefer;

struct _GlobeRefer {
    JniEnv *env;
    Runtime *runtime;
    Instance *glfw_callback;
    MethodInfo *_callback_error;
    MethodInfo *_callback_key;
    MethodInfo *_callback_character;
    MethodInfo *_callback_drop;
    MethodInfo *_button_callback_mouse;
    MethodInfo *_callback_cursor_pos;
    MethodInfo *_callback_cursor_enter;
    MethodInfo *_callback_window_close;
    MethodInfo *_callback_window_size;
    MethodInfo *_callback_window_pos;
    MethodInfo *_callback_window_focus;
    MethodInfo *_callback_window_iconify;
    MethodInfo *_callback_window_refresh;
    MethodInfo *_callback_framebuffer_size;
};
GlobeRefer refers;


/* ==============================   local tools  =================================*/

s64 getParaLong(Runtime *runtime, s32 index) {
    JniEnv *env = runtime->jnienv;
    Long2Double l2d;
    l2d.i2l.i1 = env->localvar_getInt(runtime, index);
    l2d.i2l.i0 = env->localvar_getInt(runtime, index + 1);
    return l2d.l;
}

c8 *jbytearr2c8arr(JniEnv *env, Instance *jbytearr) {
    c8 *arr = env->jvm_malloc(jbytearr->arr_length + 1);
    memcpy(arr, jbytearr->arr_body, jbytearr->arr_length);
    arr[jbytearr->arr_length] = 0;
    return arr;
}

Instance *createJavaString(Runtime *runtime, c8 *cstr) {
    JniEnv *env = runtime->jnienv;
    Utf8String *ustr = env->utf8_create_part_c(cstr, 0, strlen(cstr));
    Instance *jstr = env->jstring_create(ustr, runtime);
    env->utf8_destory(ustr);
    return jstr;
}

/* ==============================   jni callback =================================*/

static void _callback_error(int error, const char *description) {
    if (refers._callback_error) {
        JniEnv *env = refers.env;
        Utf8String *ustr = env->utf8_create_part_c((c8 *) description, 0, strlen(description));
        Instance *jstr = refers.env->jstring_create(ustr, refers.runtime);
        env->utf8_destory(ustr);
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_int(refers.runtime->stack, error);
        env->push_ref(refers.runtime->stack, jstr);
        env->execute_method(refers._callback_error, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

static void _callback_key(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (refers._callback_key) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->push_int(refers.runtime->stack, key);
        env->push_int(refers.runtime->stack, scancode);
        env->push_int(refers.runtime->stack, action);
        env->push_int(refers.runtime->stack, mods);
        env->execute_method(refers._callback_key, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

static void _callback_character(GLFWwindow *window, u32 ch) {
    if (refers._callback_character) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->push_int(refers.runtime->stack, ch);
        env->execute_method(refers._callback_character, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

static void _callback_drop(GLFWwindow *window, s32 count, const c8 **cstrs) {
    if (refers._callback_drop) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->push_int(refers.runtime->stack, count);
        Utf8String *cls = env->utf8_create_part_c(STR_CLASS_JAVA_LANG_STRING, 0, strlen(STR_CLASS_JAVA_LANG_STRING));
        Instance *jstrs = env->jarray_create(count, 0, cls);
        env->utf8_destory(cls);
        s32 i;
        Long2Double l2d;
        for (i = 0; i < count; i++) {
            l2d.r = createJavaString(refers.runtime, (c8 *) cstrs[i]);
            env->jarray_set_field(jstrs, i, &l2d);
        }
        env->push_ref(refers.runtime->stack, jstrs);
        env->execute_method(refers._callback_drop, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

void _button_callback_mouse(GLFWwindow *window, int button, int action, int mods) {
    if (refers._button_callback_mouse) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->push_int(refers.runtime->stack, button);
        env->push_int(refers.runtime->stack, action == GLFW_PRESS);
        env->execute_method(refers._button_callback_mouse, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

void _callback_cursor_pos(GLFWwindow *window, f64 x, f64 y) {
    if (refers._callback_cursor_pos) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->push_int(refers.runtime->stack, x);
        env->push_int(refers.runtime->stack, y);
        env->execute_method(refers._callback_cursor_pos, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

void _callback_cursor_enter(GLFWwindow *window, s32 enter) {
    if (refers._callback_cursor_enter) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->push_int(refers.runtime->stack, enter);
        env->execute_method(refers._callback_cursor_enter, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

void _callback_window_size(GLFWwindow *window, s32 w, s32 h) {
    if (refers._callback_window_size) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->push_int(refers.runtime->stack, w);
        env->push_int(refers.runtime->stack, h);
        env->execute_method(refers._callback_window_size, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

void _callback_window_pos(GLFWwindow *window, s32 w, s32 h) {
    if (refers._callback_window_pos) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->push_int(refers.runtime->stack, w);
        env->push_int(refers.runtime->stack, h);
        env->execute_method(refers._callback_window_pos, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

void _callback_window_close(GLFWwindow *window) {
    if (refers._callback_window_close) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->execute_method(refers._callback_window_close, refers.runtime, refers.glfw_callback->mb.clazz);
        env->pop_empty(refers.runtime->stack);
    }
}

void _callback_window_focus(GLFWwindow *window, s32 focus) {
    if (refers._callback_window_focus) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->push_int(refers.runtime->stack, focus);
        env->execute_method(refers._callback_window_focus, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

void _callback_window_iconify(GLFWwindow *window, s32 iconified) {
    if (refers._callback_window_iconify) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->push_int(refers.runtime->stack, iconified);
        env->execute_method(refers._callback_window_iconify, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

void _callback_window_refresh(GLFWwindow *window) {
    if (refers._callback_window_refresh) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->execute_method(refers._callback_window_refresh, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

void _callback_framebuffer_size(GLFWwindow *window, s32 w, s32 h) {
    if (refers._callback_framebuffer_size) {
        JniEnv *env = refers.env;
        env->push_ref(refers.runtime->stack, refers.glfw_callback);
        env->push_long(refers.runtime->stack, (s64) (intptr_t) window);
        env->push_int(refers.runtime->stack, w);
        env->push_int(refers.runtime->stack, h);
        env->execute_method(refers._callback_framebuffer_size, refers.runtime, refers.glfw_callback->mb.clazz);
    }
}

int org_mini_glfw_utils_Gutil_f2b(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *farr = env->localvar_getRefer(runtime, pos++);
    Instance *barr = env->localvar_getRefer(runtime, pos++);
    if (farr->arr_length == barr->arr_length * 4) {
        memcpy(barr->arr_body, farr->arr_body, barr->arr_length);
    }
    env->push_ref(runtime->stack, barr);
    return 0;
}

void vec_add(Instance *ra, Instance *aa, Instance *ba) {
    GLfloat *r = (GLfloat *) ra->arr_body;
    GLfloat *a = (GLfloat *) aa->arr_body;
    GLfloat *b = (GLfloat *) ba->arr_body;
    int i;
    for (i = 0; i < ra->arr_length; ++i)
        r[i] = a[i] + b[i];
}

int org_mini_glfw_utils_Gutil_vec_add(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *ra = env->localvar_getRefer(runtime, pos++);
    Instance *aa = env->localvar_getRefer(runtime, pos++);
    Instance *ba = env->localvar_getRefer(runtime, pos++);
    vec_add(ra, aa, ba);
    env->push_ref(runtime->stack, ra);
    return 0;
}

void vec_sub(Instance *ra, Instance *aa, Instance *ba) {
    GLfloat *r = (GLfloat *) ra->arr_body;
    GLfloat *a = (GLfloat *) aa->arr_body;
    GLfloat *b = (GLfloat *) ba->arr_body;
    int i;
    for (i = 0; i < ra->arr_length; ++i)
        r[i] = a[i] - b[i];
}

int org_mini_glfw_utils_Gutil_vec_sub(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *ra = env->localvar_getRefer(runtime, pos++);
    Instance *aa = env->localvar_getRefer(runtime, pos++);
    Instance *ba = env->localvar_getRefer(runtime, pos++);
    vec_sub(ra, aa, ba);
    env->push_ref(runtime->stack, ra);
    return 0;
}

float vec_mul_inner(Instance *aa, Instance *ba) {
    int i;
    float r;
    for (i = 0; i < aa->arr_length; ++i)
        r += aa->arr_body[i] * ba->arr_body[i];
    return r;
}

int org_mini_glfw_utils_Gutil_vec_mul_inner(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *aa = env->localvar_getRefer(runtime, pos++);
    Instance *ba = env->localvar_getRefer(runtime, pos++);
    float r = vec_mul_inner(aa, ba);
    env->push_float(runtime->stack, r);
    return 0;
}

void vec_scale(Instance *ra, Instance *aa, float f) {
    GLfloat *r = (GLfloat *) ra->arr_body;
    GLfloat *a = (GLfloat *) aa->arr_body;
    int i;
    for (i = 0; i < ra->arr_length; ++i)
        r[i] = a[i] * f;
}

int org_mini_glfw_utils_Gutil_vec_scale(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *ra = env->localvar_getRefer(runtime, pos++);
    Instance *aa = env->localvar_getRefer(runtime, pos++);
    Int2Float f;
    f.i = env->localvar_getInt(runtime, pos++);
    vec_scale(ra, aa, f.f);
    env->push_ref(runtime->stack, ra);
    return 0;
}

float vec_len(Instance *ra) {
    return (float) sqrt(vec_mul_inner(ra, ra));
}

int org_mini_glfw_utils_Gutil_vec_len(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *ra = env->localvar_getRefer(runtime, pos++);
    float f = vec_len(ra);
    env->push_float(runtime->stack, f);
    return 0;
}

int org_mini_glfw_utils_Gutil_vec_normal(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *ra = env->localvar_getRefer(runtime, pos++);
    Instance *aa = env->localvar_getRefer(runtime, pos++);
    float k = 1.f / vec_len(aa);
    vec_scale(ra, aa, k);
    env->push_ref(runtime->stack, ra);
    return 0;
}

int org_mini_glfw_utils_Gutil_vec_reflect(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *ra = env->localvar_getRefer(runtime, pos++);
    Instance *aa = env->localvar_getRefer(runtime, pos++);
    Instance *ba = env->localvar_getRefer(runtime, pos++);
    GLfloat *r = (GLfloat *) ra->arr_body;
    GLfloat *a = (GLfloat *) aa->arr_body;
    GLfloat *b = (GLfloat *) ba->arr_body;
    float p = 2.f * vec_mul_inner(aa, ba);
    int i;
    for (i = 0; i < 4; ++i)
        r[i] = a[i] - p * b[i];
    env->push_ref(runtime->stack, ra);
    return 0;
}

int org_mini_glfw_utils_Gutil_vec_mul_cross(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *ra = env->localvar_getRefer(runtime, pos++);
    Instance *aa = env->localvar_getRefer(runtime, pos++);
    Instance *ba = env->localvar_getRefer(runtime, pos++);
    GLfloat *r = (GLfloat *) ra->arr_body;
    GLfloat *a = (GLfloat *) aa->arr_body;
    GLfloat *b = (GLfloat *) ba->arr_body;
    r[0] = a[1] * b[2] - a[2] * b[1];
    r[1] = a[2] * b[0] - a[0] * b[2];
    r[2] = a[0] * b[1] - a[1] * b[0];
    if (ra->arr_length > 3)r[3] = 1.f;
    env->push_ref(runtime->stack, ra);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_identity(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    mat4x4_identity((vec4 *) r->arr_body);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_dup(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    mat4x4_dup((vec4 *) r->arr_body, (vec4 *) m1->arr_body);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_row(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    int row = env->localvar_getInt(runtime, pos++);
    mat4x4_row((GLfloat *) r->arr_body, (vec4 *) m1->arr_body, row);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_col(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    int col = env->localvar_getInt(runtime, pos++);
    mat4x4_col((GLfloat *) r->arr_body, (vec4 *) m1->arr_body, col);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_transpose(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    mat4x4_transpose((vec4 *) r->arr_body, (vec4 *) m1->arr_body);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_add(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    Instance *m2 = env->localvar_getRefer(runtime, pos++);
    mat4x4_add((vec4 *) r->arr_body, (vec4 *) m1->arr_body, (vec4 *) m2->arr_body);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_sub(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    Instance *m2 = env->localvar_getRefer(runtime, pos++);
    mat4x4_sub((vec4 *) r->arr_body, (vec4 *) m1->arr_body, (vec4 *) m2->arr_body);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_mul(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    Instance *m2 = env->localvar_getRefer(runtime, pos++);
    mat4x4_mul((vec4 *) r->arr_body, (vec4 *) m1->arr_body, (vec4 *) m2->arr_body);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_mul_vec4(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    Instance *m2 = env->localvar_getRefer(runtime, pos++);
    mat4x4_mul_vec4((GLfloat *) r->arr_body, (vec4 *) m1->arr_body, (GLfloat *) m2->arr_body);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_from_vec3_mul_outer(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    Instance *m2 = env->localvar_getRefer(runtime, pos++);
    mat4x4_from_vec3_mul_outer((vec4 *) r->arr_body, (GLfloat *) m1->arr_body, (GLfloat *) m2->arr_body);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_translate(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Int2Float x, y, z;
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    z.i = env->localvar_getInt(runtime, pos++);
    mat4x4_translate((vec4 *) r->arr_body, x.f, y.f, z.f);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_translate_in_place(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Int2Float x, y, z;
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    z.i = env->localvar_getInt(runtime, pos++);
    mat4x4_translate_in_place((vec4 *) r->arr_body, x.f, y.f, z.f);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_scale(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    Int2Float f;
    f.i = env->localvar_getInt(runtime, pos++);
    mat4x4_scale((vec4 *) r->arr_body, (vec4 *) m1->arr_body, f.f);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_scale_aniso(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    Int2Float x, y, z;
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    z.i = env->localvar_getInt(runtime, pos++);
    mat4x4_scale_aniso((vec4 *) r->arr_body, (vec4 *) m1->arr_body, x.f, y.f, z.f);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_rotate(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    Int2Float x, y, z, a;
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    z.i = env->localvar_getInt(runtime, pos++);
    a.i = env->localvar_getInt(runtime, pos++);
    mat4x4_rotate((vec4 *) r->arr_body, (vec4 *) m1->arr_body, x.f, y.f, z.f, a.f);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_rotateX(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    Int2Float f;
    f.i = env->localvar_getInt(runtime, pos++);
    mat4x4_rotate_X((vec4 *) r->arr_body, (vec4 *) m1->arr_body, f.f);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_rotateY(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    Int2Float f;
    f.i = env->localvar_getInt(runtime, pos++);
    mat4x4_rotate_Y((vec4 *) r->arr_body, (vec4 *) m1->arr_body, f.f);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_rotateZ(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    Int2Float f;
    f.i = env->localvar_getInt(runtime, pos++);
    mat4x4_rotate_Z((vec4 *) r->arr_body, (vec4 *) m1->arr_body, f.f);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_invert(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    mat4x4_invert((vec4 *) r->arr_body, (vec4 *) m1->arr_body);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_orthonormalize(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *m1 = env->localvar_getRefer(runtime, pos++);
    mat4x4_orthonormalize((vec4 *) r->arr_body, (vec4 *) m1->arr_body);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_ortho(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *ra = env->localvar_getRefer(runtime, pos++);
    Int2Float l, r, b, t, n, f;
    l.i = env->localvar_getInt(runtime, pos++);
    r.i = env->localvar_getInt(runtime, pos++);
    b.i = env->localvar_getInt(runtime, pos++);
    t.i = env->localvar_getInt(runtime, pos++);
    n.i = env->localvar_getInt(runtime, pos++);
    f.i = env->localvar_getInt(runtime, pos++);
    mat4x4_ortho((vec4 *) ra->arr_body, l.f, r.f, b.f, t.f, n.f, f.f);
    env->push_ref(runtime->stack, ra);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_frustum(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *ra = env->localvar_getRefer(runtime, pos++);
    Int2Float l, r, b, t, n, f;
    l.i = env->localvar_getInt(runtime, pos++);
    r.i = env->localvar_getInt(runtime, pos++);
    b.i = env->localvar_getInt(runtime, pos++);
    t.i = env->localvar_getInt(runtime, pos++);
    n.i = env->localvar_getInt(runtime, pos++);
    f.i = env->localvar_getInt(runtime, pos++);
    mat4x4_frustum((vec4 *) ra->arr_body, l.f, r.f, b.f, t.f, n.f, f.f);
    env->push_ref(runtime->stack, ra);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_perspective(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Int2Float y_fov, aspect, n, f;
    y_fov.i = env->localvar_getInt(runtime, pos++);
    aspect.i = env->localvar_getInt(runtime, pos++);
    n.i = env->localvar_getInt(runtime, pos++);
    f.i = env->localvar_getInt(runtime, pos++);
    mat4x4_perspective((vec4 *) r->arr_body, y_fov.f, aspect.f, n.f, f.f);
    env->push_ref(runtime->stack, r);
    return 0;
}

int org_mini_glfw_utils_Gutil_mat4x4_look_at(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    int pos = 0;
    Instance *r = env->localvar_getRefer(runtime, pos++);
    Instance *vec3_eye = env->localvar_getRefer(runtime, pos++);
    Instance *vec3_center = env->localvar_getRefer(runtime, pos++);
    Instance *vec3_up = env->localvar_getRefer(runtime, pos++);
    mat4x4_look_at((vec4 *) r->arr_body, (float *) vec3_eye->arr_body,
                   (float *) vec3_center->arr_body,
                   (float *) vec3_up->arr_body);
    env->push_ref(runtime->stack, r);
    return 0;
}

/* ==============================   jni glfw =================================*/

int org_mini_glfw_Glfw_glfwSetCallbackJni(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    GLFWwindow *window = (__refer) (intptr_t) getParaLong(runtime, pos);
    pos += 2;
    refers.glfw_callback = env->localvar_getRefer(runtime, pos++);

    //this object not refered by jvm , so needs to hold by jni manaul
    if (refers.glfw_callback) env->instance_release_from_thread(refers.glfw_callback, runtime);
    env->instance_hold_to_thread(refers.glfw_callback, runtime);

    glfwSetErrorCallback(_callback_error);
    glfwSetKeyCallback(window, _callback_key);
    glfwSetCharCallback(window, _callback_character);
    glfwSetDropCallback(window, _callback_drop);
    glfwSetMouseButtonCallback(window, _button_callback_mouse);
    glfwSetCursorPosCallback(window, _callback_cursor_pos);
    glfwSetCursorEnterCallback(window, _callback_cursor_enter);
    glfwSetWindowCloseCallback(window, _callback_window_close);
    glfwSetWindowSizeCallback(window, _callback_window_size);
    glfwSetWindowPosCallback(window, _callback_window_pos);
    glfwSetWindowFocusCallback(window, _callback_window_focus);
    glfwSetWindowIconifyCallback(window, _callback_window_iconify);
    glfwSetWindowRefreshCallback(window, _callback_window_refresh);
    glfwSetFramebufferSizeCallback(window, _callback_framebuffer_size);

    c8 *name_s, *type_s;
    {
        name_s = "error";
        type_s = "(ILjava/lang/String;)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_error =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "key";
        type_s = "(JIIII)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_key =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "character";
        type_s = "(JC)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_character =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "drop";
        type_s = "(JI[Ljava/lang/String;)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_drop =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "mouseButton";
        type_s = "(JIZ)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._button_callback_mouse =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "cursorPos";
        type_s = "(JII)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_cursor_pos =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "cursorEnter";
        type_s = "(JZ)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_cursor_enter =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "windowPos";
        type_s = "(JII)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_window_pos =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "windowSize";
        type_s = "(JII)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_window_size =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "windowClose";
        type_s = "(J)Z";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_window_close =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "windowRefresh";
        type_s = "(J)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_window_refresh =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "windowFocus";
        type_s = "(JZ)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_window_focus =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "windowIconify";
        type_s = "(JZ)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_window_iconify =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    {
        name_s = "framebufferSize";
        type_s = "(JII)V";
        Utf8String *name = env->utf8_create_part_c(name_s, 0, strlen(name_s));
        Utf8String *type = env->utf8_create_part_c(type_s, 0, strlen(type_s));
        refers._callback_framebuffer_size =
                env->find_methodInfo_by_name(refers.glfw_callback->mb.clazz->name, name, type);
        env->utf8_destory(name);
        env->utf8_destory(type);
    }
    return 0;
}

int org_mini_glfw_Glfw_glfwGetTime(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 list = env->localvar_getInt(runtime, pos++);
    env->push_double(runtime->stack, glfwGetTime());
    return 0;
}

int org_mini_glfw_Glfw_glfwSetTime(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double t;
    t.l = getParaLong(runtime, pos);
    pos += 2;
    glfwSetTime(t.d);
    return 0;
}

int org_mini_glfw_Glfw_glfwCreateWindowJni(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 width = env->localvar_getInt(runtime, pos++);
    s32 height = env->localvar_getInt(runtime, pos++);
    Instance *title_arr = env->localvar_getRefer(runtime, pos++);
    c8 *title = jbytearr2c8arr(env, title_arr);
    GLFWmonitor *monitor = (__refer) (intptr_t) getParaLong(runtime, pos);
    pos += 2;
    GLFWwindow *share = (__refer) (intptr_t) getParaLong(runtime, pos);
    pos += 2;

    GLFWwindow *window = glfwCreateWindow(width, height, title, monitor, share);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
    }
    //
    env->jvm_free(title);
    env->push_long(runtime->stack, (s64) (intptr_t) window);
    return 0;
}

int org_mini_glfw_Glfw_glfwDestroyWindow(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    GLFWwindow *window = (__refer) (intptr_t) getParaLong(runtime, pos);
    glfwDestroyWindow(window);
    return 0;
}

int org_mini_glfw_Glfw_glfwWindowShouldClose(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    GLFWwindow *window = (__refer) (intptr_t) getParaLong(runtime, pos);
    env->push_int(runtime->stack, GL_TRUE == glfwWindowShouldClose((GLFWwindow *) window));
    return 0;
}

int org_mini_glfw_Glfw_glfwInitJni(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    env->push_int(runtime->stack, glfwInit() == GLFW_TRUE);
    return 0;
}

int org_mini_glfw_Glfw_glfwTerminate(Runtime *runtime, Class *clazz) {
    glfwTerminate();
    return 0;
}

int org_mini_glfw_Glfw_glfwWindowHint(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 hint = env->localvar_getInt(runtime, pos++);
    s32 value = env->localvar_getInt(runtime, pos++);
    glfwWindowHint(hint, value);
    return 0;
}

int org_mini_glfw_Glfw_glfwPollEvents(Runtime *runtime, Class *clazz) {
    refers.runtime = runtime;
    glfwPollEvents();
    refers.runtime = NULL;
    return 0;
}

int org_mini_glfw_Glfw_glfwSetWindowShouldClose(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    GLFWwindow *window = (__refer) (intptr_t) getParaLong(runtime, pos);
    pos += 2;
    s32 value = env->localvar_getInt(runtime, pos++);
    glfwSetWindowShouldClose(window, value);
    return 0;
}

int org_mini_glfw_Glfw_glfwMakeContextCurrentJni(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    GLFWwindow *window = (__refer) (intptr_t) getParaLong(runtime, pos);
    pos += 2;
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    return 0;
}

int org_mini_glfw_Glfw_glfwSwapInterval(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 interval = env->localvar_getInt(runtime, pos++);
    glfwSwapInterval(interval);
    return 0;
}

int org_mini_glfw_Glfw_glfwSwapBuffers(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    GLFWwindow *window = (__refer) (intptr_t) getParaLong(runtime, pos);
    pos += 2;
    glfwSwapBuffers(window);
    return 0;
}

int org_mini_glfw_Glfw_glfwGetFramebufferSizeW(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    GLFWwindow *window = (__refer) (intptr_t) getParaLong(runtime, pos);
    s32 w, h;
    glfwGetFramebufferSize(window, &w, &h);
    env->push_int(runtime->stack, w);
    return 0;
}

int org_mini_glfw_Glfw_glfwGetFramebufferSizeH(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    GLFWwindow *window = (__refer) (intptr_t) getParaLong(runtime, pos);
    s32 w, h;
    glfwGetFramebufferSize(window, &w, &h);
    env->push_int(runtime->stack, h);
    return 0;
}

int org_mini_glfw_Glfw_glfwSetWindowAspectRatio(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    GLFWwindow *window = (__refer) (intptr_t) getParaLong(runtime, pos);
    pos += 2;
    s32 numer = env->localvar_getInt(runtime, pos++);
    s32 denom = env->localvar_getInt(runtime, pos++);
    glfwSetWindowAspectRatio(window, numer, denom);
    return 0;
}

int org_mini_glfw_Glfw_glfwGetClipboardString(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    GLFWwindow *window = (__refer) (intptr_t) getParaLong(runtime, pos);
    c8 *cstr = (c8 *) glfwGetClipboardString(window);
    if (cstr) {
        Utf8String *ustr = env->utf8_create_part_c(cstr, 0, strlen(cstr));
        Instance *jstr = env->jstring_create(ustr, runtime);
        env->utf8_destory(ustr);
        env->push_ref(runtime->stack, jstr);
    } else {
        env->push_ref(runtime->stack, NULL);
    }
    return 0;
}

/* ==============================   jni gl =================================*/


int org_mini_gl_GL_init(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;

    return 0;
}

int org_mini_gl_GL_glViewport(Runtime *runtime, Class *clazz) {
    //glViewport(GLint x,GLint y,GLsizei width,GLsizei height);
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 x = env->localvar_getInt(runtime, pos++);
    s32 y = env->localvar_getInt(runtime, pos++);
    s32 w = env->localvar_getInt(runtime, pos++);
    s32 h = env->localvar_getInt(runtime, pos++);
    glViewport(x, y, (GLsizei) w, (GLsizei) h);
    return 0;
}

int org_mini_gl_GL_glVertexAttribPointer(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 index = env->localvar_getInt(runtime, pos++);
    s32 size = env->localvar_getInt(runtime, pos++);
    s32 type = env->localvar_getInt(runtime, pos++);
    s32 normalized = env->localvar_getInt(runtime, pos++);
    s32 stride = env->localvar_getInt(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    glVertexAttribPointer((GLuint) index, (GLint) size, (GLenum) type, (GLboolean) normalized, (GLsizei) stride,
                          (const GLvoid *) (NULL + offset));
    return 0;
}


int org_mini_gl_GL_glMatrixMode(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 mode = env->localvar_getInt(runtime, pos++);
    glMatrixMode((GLenum) mode);
    return 0;
}

int org_mini_gl_GL_glLoadMatrixf(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glLoadMatrixf((const GLfloat *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glPushMatrix(Runtime *runtime, Class *clazz) {
    glPushMatrix();
    return 0;
}

int org_mini_gl_GL_glPopMatrix(Runtime *runtime, Class *clazz) {
    glPopMatrix();
    return 0;
}

int org_mini_gl_GL_glEnable(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 cap = env->localvar_getInt(runtime, pos++);
    glEnable((GLenum) cap);
    return 0;
}

int org_mini_gl_GL_glDisable(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 cap = env->localvar_getInt(runtime, pos++);
    glDisable((GLenum) cap);
    return 0;
}

int org_mini_gl_GL_glShadeModel(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 mode = env->localvar_getInt(runtime, pos++);
    glShadeModel((GLenum) mode);
    return 0;
}

int org_mini_gl_GL_glClear(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 mask = env->localvar_getInt(runtime, pos++);
    glClear((GLbitfield) mask);
    return 0;
}

int org_mini_gl_GL_glHint(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 target = env->localvar_getInt(runtime, pos++);
    s32 mode = env->localvar_getInt(runtime, pos++);
    glHint((GLenum) target, (GLenum) mode);
    return 0;
}

int org_mini_gl_GL_glClearDepth(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double depth;
    depth.l = getParaLong(runtime, pos);
    glClearDepth((GLclampd) depth.d);
    return 0;
}

int org_mini_gl_GL_glDepthFunc(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 func = env->localvar_getInt(runtime, pos++);
    glDepthFunc((GLenum) func);
    return 0;
}

int org_mini_gl_GL_glDepthMask(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 flag = env->localvar_getInt(runtime, pos++);
    glDepthMask((GLboolean) flag);
    return 0;
}

int org_mini_gl_GL_glDepthRange(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double near_val, far_val;
    near_val.l = getParaLong(runtime, pos);
    far_val.l = getParaLong(runtime, pos);
    glDepthRange((GLclampd) near_val.d, (GLclampd) far_val.d);
    return 0;
}

int org_mini_gl_GL_glDrawBuffer(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 mode = env->localvar_getInt(runtime, pos++);
    glDrawBuffer((GLenum) mode);
    return 0;
}

int org_mini_gl_GL_glReadBuffer(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 mode = env->localvar_getInt(runtime, pos++);
    glReadBuffer((GLenum) mode);
    return 0;
}

int org_mini_gl_GL_glTranslatef(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float x, y, z;
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    z.i = env->localvar_getInt(runtime, pos++);
    glTranslatef((GLfloat) x.f, (GLfloat) y.f, (GLfloat) z.f);
    return 0;
}

int org_mini_gl_GL_glTranslated(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double x, y, z;
    x.l = env->localvar_getInt(runtime, pos++);
    y.l = env->localvar_getInt(runtime, pos++);
    z.l = env->localvar_getInt(runtime, pos++);
    glTranslated((GLdouble) x.d, (GLdouble) y.d, (GLdouble) z.d);
    return 0;
}

int org_mini_gl_GL_glRotatef(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float a, x, y, z;
    a.i = env->localvar_getInt(runtime, pos++);
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    z.i = env->localvar_getInt(runtime, pos++);
    glRotatef((GLfloat) a.f, (GLfloat) x.f, (GLfloat) y.f, (GLfloat) z.f);
    return 0;
}

int org_mini_gl_GL_glRotated(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double a, x, y, z;
    a.l = env->localvar_getInt(runtime, pos++);
    x.l = env->localvar_getInt(runtime, pos++);
    y.l = env->localvar_getInt(runtime, pos++);
    z.l = env->localvar_getInt(runtime, pos++);
    glRotated((GLdouble) a.d, (GLdouble) x.d, (GLdouble) y.d, (GLdouble) z.d);
    return 0;
}

int org_mini_gl_GL_glScalef(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float x, y, z;
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    z.i = env->localvar_getInt(runtime, pos++);
    glScalef((GLfloat) x.f, (GLfloat) y.f, (GLfloat) z.f);
    return 0;
}


int org_mini_gl_GL_glScaled(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double x, y, z;
    x.l = env->localvar_getInt(runtime, pos++);
    y.l = env->localvar_getInt(runtime, pos++);
    z.l = env->localvar_getInt(runtime, pos++);
    glScaled((GLdouble) x.d, (GLdouble) y.d, (GLdouble) z.d);
    return 0;
}


int org_mini_gl_GL_glCullFace(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 mode = env->localvar_getInt(runtime, pos++);
    glCullFace((GLenum) mode);
    return 0;
}

int org_mini_gl_GL_glIndexMask(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 mask = env->localvar_getInt(runtime, pos++);
    glIndexMask((GLuint) mask);
    return 0;
}

int org_mini_gl_GL_glLogicOp(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 opcode = env->localvar_getInt(runtime, pos++);
    glLogicOp((GLenum) opcode);
    return 0;
}

int org_mini_gl_GL_glBlendFunc(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 sfactor = env->localvar_getInt(runtime, pos++);
    s32 dfactor = env->localvar_getInt(runtime, pos++);
    glBlendFunc((GLenum) sfactor, (GLenum) dfactor);
    return 0;
}

int org_mini_gl_GL_glColorMask(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 red = env->localvar_getInt(runtime, pos++);
    s32 green = env->localvar_getInt(runtime, pos++);
    s32 blue = env->localvar_getInt(runtime, pos++);
    s32 alpha = env->localvar_getInt(runtime, pos++);
    glColorMask((GLboolean) red, (GLboolean) green, (GLboolean) blue, (GLboolean) alpha);
    return 0;
}

int org_mini_gl_GL_glAlphaFunc(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 func = env->localvar_getInt(runtime, pos++);
    Int2Float ref;
    ref.i = env->localvar_getInt(runtime, pos++);
    glAlphaFunc((GLenum) func, (GLclampf) ref.f);
    return 0;
}

int org_mini_gl_GL_glFrontFace(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 mode = env->localvar_getInt(runtime, pos++);
    glFrontFace((GLenum) mode);
    return 0;
}

int org_mini_gl_GL_glBegin(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 mode = env->localvar_getInt(runtime, pos++);
    glBegin((GLenum) mode);
    return 0;
}

int org_mini_gl_GL_glEnd(Runtime *runtime, Class *clazz) {
    glEnd();
    return 0;
}

int org_mini_gl_GL_glFlush(Runtime *runtime, Class *clazz) {
    glFlush();
    return 0;
}

int org_mini_gl_GL_glFinish(Runtime *runtime, Class *clazz) {
    glFinish();
    return 0;
}

int org_mini_gl_GL_glColor3f(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float r, g, b;
    r.i = env->localvar_getInt(runtime, pos++);
    g.i = env->localvar_getInt(runtime, pos++);
    b.i = env->localvar_getInt(runtime, pos++);
    glColor3f((GLfloat) r.f, (GLfloat) g.f, (GLfloat) b.f);
    return 0;
}

int org_mini_gl_GL_glColor4f(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float r, g, b, a;
    r.i = env->localvar_getInt(runtime, pos++);
    g.i = env->localvar_getInt(runtime, pos++);
    b.i = env->localvar_getInt(runtime, pos++);
    a.i = env->localvar_getInt(runtime, pos++);
    glColor4f((GLfloat) r.f, (GLfloat) g.f, (GLfloat) b.f, (GLfloat) a.f);
    return 0;
}

int org_mini_gl_GL_glColor3d(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double r, g, b;
    r.l = env->localvar_getInt(runtime, pos++);
    g.l = env->localvar_getInt(runtime, pos++);
    b.l = env->localvar_getInt(runtime, pos++);
    glColor3d((GLdouble) r.d, (GLdouble) g.d, (GLdouble) b.d);
    return 0;
}

int org_mini_gl_GL_glColor4d(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double r, g, b, a;
    r.l = env->localvar_getInt(runtime, pos++);
    g.l = env->localvar_getInt(runtime, pos++);
    b.l = env->localvar_getInt(runtime, pos++);
    a.l = env->localvar_getInt(runtime, pos++);
    glColor4d((GLdouble) r.d, (GLdouble) g.d, (GLdouble) b.d, (GLdouble) a.d);
    return 0;
}

int org_mini_gl_GL_glColor3b(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    glColor3b((GLbyte) r, (GLbyte) g, (GLbyte) b);
    return 0;
}

int org_mini_gl_GL_glColor4b(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b, a;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    a = env->localvar_getInt(runtime, pos++);
    glColor4b((GLbyte) r, (GLbyte) g, (GLbyte) b, (GLbyte) a);
    return 0;
}

int org_mini_gl_GL_glColor3ub(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    glColor3ub((GLubyte) r, (GLubyte) g, (GLubyte) b);
    return 0;
}

int org_mini_gl_GL_glColor4ub(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b, a;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    a = env->localvar_getInt(runtime, pos++);
    glColor4ub((GLubyte) r, (GLubyte) g, (GLubyte) b, (GLubyte) a);
    return 0;
}

int org_mini_gl_GL_glColor3i(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    glColor3i((GLint) r, (GLint) g, (GLint) b);
    return 0;
}

int org_mini_gl_GL_glColor4i(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b, a;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    a = env->localvar_getInt(runtime, pos++);
    glColor4i((GLint) r, (GLint) g, (GLint) b, (GLint) a);
    return 0;
}

int org_mini_gl_GL_glColor3ui(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    glColor3ui((GLuint) r, (GLuint) g, (GLuint) b);
    return 0;
}

int org_mini_gl_GL_glColor4ui(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b, a;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    a = env->localvar_getInt(runtime, pos++);
    glColor4ui((GLuint) r, (GLuint) g, (GLuint) b, (GLuint) a);
    return 0;
}

int org_mini_gl_GL_glColor3s(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    glColor3s((GLshort) r, (GLshort) g, (GLshort) b);
    return 0;
}

int org_mini_gl_GL_glColor4s(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b, a;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    a = env->localvar_getInt(runtime, pos++);
    glColor4s((GLshort) r, (GLshort) g, (GLshort) b, (GLshort) a);
    return 0;
}

int org_mini_gl_GL_glColor3us(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    glColor3us((GLushort) r, (GLushort) g, (GLushort) b);
    return 0;
}

int org_mini_gl_GL_glColor4us(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 r, g, b, a;
    r = env->localvar_getInt(runtime, pos++);
    g = env->localvar_getInt(runtime, pos++);
    b = env->localvar_getInt(runtime, pos++);
    a = env->localvar_getInt(runtime, pos++);
    glColor4us((GLushort) r, (GLushort) g, (GLushort) b, (GLushort) a);
    return 0;
}

int org_mini_gl_GL_glColor3dv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor3dv((const GLdouble *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor4dv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor4dv((const GLdouble *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor3fv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor3fv((const GLfloat *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor4fv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor4fv((const GLfloat *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor3bv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor3bv((const GLbyte *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor4bv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor4bv((const GLbyte *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor3ubv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor3ubv((const GLubyte *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor4ubv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor4ubv((const GLubyte *) (arr->arr_body + offset));
    return 0;
}

//int org_mini_gl_GL_glColor3iv(Runtime *runtime, Class *clazz) {
//    JniEnv *env = runtime->jnienv;
//    s32 pos = 0;
//    Instance *arr = env->localvar_getRefer(runtime, pos++);
//    s32 offset = env->localvar_getInt(runtime, pos++);
//    offset *= env->data_type_bytes[arr->mb.arr_type_index];
//    glColor3iv((const GLint *) (arr->arr_body + offset));
//    return 0;
//}
int org_mini_gl_GL_glColor3iv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;

    Instance *v = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];

    glColor3iv((const GLvoid *) (v->arr_body + offset));

    return 0;
}

int org_mini_gl_GL_glColor4iv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor4iv((const GLint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor3uiv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor3uiv((const GLuint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor4uiv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor4uiv((const GLuint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor3sv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor3sv((const GLshort *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor4sv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor4sv((const GLshort *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor3usv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor3usv((const GLshort *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColor4usv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glColor4usv((const GLshort *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex2s(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    int x, y;
    x = env->localvar_getInt(runtime, pos++);
    y = env->localvar_getInt(runtime, pos++);
    glVertex2s((GLshort) x, (GLshort) y);
    return 0;
}

int org_mini_gl_GL_glVertex3s(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    int x, y, z, w;
    x = env->localvar_getInt(runtime, pos++);
    y = env->localvar_getInt(runtime, pos++);
    z = env->localvar_getInt(runtime, pos++);
    glVertex3s((GLshort) x, (GLshort) y, (GLshort) z);
    return 0;
}

int org_mini_gl_GL_glVertex4s(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    int x, y, z, w;
    x = env->localvar_getInt(runtime, pos++);
    y = env->localvar_getInt(runtime, pos++);
    z = env->localvar_getInt(runtime, pos++);
    w = env->localvar_getInt(runtime, pos++);
    glVertex4s((GLshort) x, (GLshort) y, (GLshort) z, (GLshort) w);
    return 0;
}

int org_mini_gl_GL_glVertex2i(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    int x, y;
    x = env->localvar_getInt(runtime, pos++);
    y = env->localvar_getInt(runtime, pos++);
    glVertex2i((GLint) x, (GLint) y);
    return 0;
}

int org_mini_gl_GL_glVertex3i(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    int x, y, z;
    x = env->localvar_getInt(runtime, pos++);
    y = env->localvar_getInt(runtime, pos++);
    z = env->localvar_getInt(runtime, pos++);
    glVertex3i((GLint) x, (GLint) y, (GLint) z);
    return 0;
}

int org_mini_gl_GL_glVertex4i(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    int x, y, z, w;
    x = env->localvar_getInt(runtime, pos++);
    y = env->localvar_getInt(runtime, pos++);
    z = env->localvar_getInt(runtime, pos++);
    w = env->localvar_getInt(runtime, pos++);
    glVertex4i((GLint) x, (GLint) y, (GLint) z, (GLint) w);
    return 0;
}

int org_mini_gl_GL_glVertex2f(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float x, y;
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    glVertex2f((GLfloat) x.f, (GLfloat) y.f);
    return 0;
}

int org_mini_gl_GL_glVertex3f(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float x, y, z;
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    z.i = env->localvar_getInt(runtime, pos++);
    glVertex3f((GLfloat) x.f, (GLfloat) y.f, (GLfloat) z.f);
    return 0;
}

int org_mini_gl_GL_glVertex4f(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float x, y, z, w;
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    z.i = env->localvar_getInt(runtime, pos++);
    w.i = env->localvar_getInt(runtime, pos++);
    glVertex4f((GLfloat) x.f, (GLfloat) y.f, (GLfloat) z.f, (GLfloat) w.f);
    return 0;
}

int org_mini_gl_GL_glVertex2d(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double x, y, z;
    x.l = env->localvar_getInt(runtime, pos++);
    y.l = env->localvar_getInt(runtime, pos++);
    glVertex2d((GLdouble) x.d, (GLdouble) y.d);
    return 0;
}

int org_mini_gl_GL_glVertex3d(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double x, y, z;
    x.l = env->localvar_getInt(runtime, pos++);
    y.l = env->localvar_getInt(runtime, pos++);
    z.l = env->localvar_getInt(runtime, pos++);
    glVertex3d((GLdouble) x.d, (GLdouble) y.d, (GLdouble) z.d);
    return 0;
}

int org_mini_gl_GL_glVertex4d(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double x, y, z, w;
    x.l = env->localvar_getInt(runtime, pos++);
    y.l = env->localvar_getInt(runtime, pos++);
    z.l = env->localvar_getInt(runtime, pos++);
    w.l = env->localvar_getInt(runtime, pos++);
    glVertex4d((GLdouble) x.d, (GLdouble) y.d, (GLdouble) z.d, (GLdouble) w.d);
    return 0;
}

int org_mini_gl_GL_glVertex2sv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex2sv((const GLshort *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex3sv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex3sv((const GLshort *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex4sv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex4sv((const GLshort *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex2iv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex2iv((const GLint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex3iv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex3iv((const GLint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex4iv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex4iv((const GLint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex2fv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex2fv((const GLfloat *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex3fv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex3fv((const GLfloat *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex4fv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex4fv((const GLfloat *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex2dv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex2dv((const GLdouble *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex3dv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex3dv((const GLdouble *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glVertex4dv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glVertex4dv((const GLdouble *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glNormal3b(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    int x, y, z;
    x = env->localvar_getInt(runtime, pos++);
    y = env->localvar_getInt(runtime, pos++);
    z = env->localvar_getInt(runtime, pos++);
    glNormal3b((GLbyte) x, (GLbyte) y, (GLbyte) z);
    return 0;
}

int org_mini_gl_GL_glNormal3s(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    int x, y, z;
    x = env->localvar_getInt(runtime, pos++);
    y = env->localvar_getInt(runtime, pos++);
    z = env->localvar_getInt(runtime, pos++);
    glNormal3s((GLshort) x, (GLshort) y, (GLshort) z);
    return 0;
}

int org_mini_gl_GL_glNormal3i(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    int x, y, z;
    x = env->localvar_getInt(runtime, pos++);
    y = env->localvar_getInt(runtime, pos++);
    z = env->localvar_getInt(runtime, pos++);
    glNormal3i((GLint) x, (GLint) y, (GLint) z);
    return 0;
}

int org_mini_gl_GL_glNormal3f(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float x, y, z;
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    z.i = env->localvar_getInt(runtime, pos++);
    glNormal3f((GLfloat) x.f, (GLfloat) y.f, (GLfloat) z.f);
    return 0;
}

int org_mini_gl_GL_glNormal3d(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double x, y, z;
    x.l = env->localvar_getInt(runtime, pos++);
    y.l = env->localvar_getInt(runtime, pos++);
    z.l = env->localvar_getInt(runtime, pos++);
    glNormal3d((GLdouble) x.d, (GLdouble) y.d, (GLdouble) z.d);
    return 0;
}

int org_mini_gl_GL_glNormal3bv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glNormal3bv((const GLbyte *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glNormal3sv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glNormal3sv((const GLshort *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glNormal3iv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glNormal3iv((const GLint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glNormal3fv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glNormal3fv((const GLfloat *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glNormal3dv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glNormal3dv((const GLdouble *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glClearColor(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float x, y, z, a;
    x.i = env->localvar_getInt(runtime, pos++);
    y.i = env->localvar_getInt(runtime, pos++);
    z.i = env->localvar_getInt(runtime, pos++);
    a.i = env->localvar_getInt(runtime, pos++);
    glClearColor((GLfloat) x.f, (GLfloat) y.f, (GLfloat) z.f, (GLfloat) a.f);
    return 0;
}

int org_mini_gl_GL_glRectf(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float x1, y1, x2, y2;
    x1.i = env->localvar_getInt(runtime, pos++);
    y1.i = env->localvar_getInt(runtime, pos++);
    x2.i = env->localvar_getInt(runtime, pos++);
    y2.i = env->localvar_getInt(runtime, pos++);
    glRectf((GLfloat) x1.f, (GLfloat) y1.f, (GLfloat) x2.f, (GLfloat) y2.f);
    return 0;
}

int org_mini_gl_GL_glLightModelf(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 pname = env->localvar_getInt(runtime, pos++);
    Int2Float param;
    param.i = env->localvar_getInt(runtime, pos++);
    glLightModelf((GLenum) pname, (GLfloat) param.f);
    return 0;
}

int org_mini_gl_GL_glLightModeli(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 pname = env->localvar_getInt(runtime, pos++);
    s32 param = env->localvar_getInt(runtime, pos++);
    glLightModeli((GLenum) pname, (GLint) param);
    return 0;
}

int org_mini_gl_GL_glLightfv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 light = env->localvar_getInt(runtime, pos++);
    s32 pname = env->localvar_getInt(runtime, pos++);
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glLightfv((GLenum) light, (GLenum) pname, (const GLfloat *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glLightiv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 light = env->localvar_getInt(runtime, pos++);
    s32 pname = env->localvar_getInt(runtime, pos++);
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glLightiv((GLenum) light, (GLenum) pname, (const GLint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glMaterialf(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 face = env->localvar_getInt(runtime, pos++);
    s32 pname = env->localvar_getInt(runtime, pos++);
    Int2Float param;
    param.i = env->localvar_getInt(runtime, pos++);
    glMaterialf((GLenum) face, (GLenum) pname, (GLfloat) param.f);
    return 0;
}

int org_mini_gl_GL_glMateriali(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 face = env->localvar_getInt(runtime, pos++);
    s32 pname = env->localvar_getInt(runtime, pos++);
    s32 param = env->localvar_getInt(runtime, pos++);
    glMateriali((GLenum) face, (GLenum) pname, (GLint) param);
    return 0;
}

int org_mini_gl_GL_glMaterialfv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 face = env->localvar_getInt(runtime, pos++);
    s32 pname = env->localvar_getInt(runtime, pos++);
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glMaterialfv((GLenum) face, (GLenum) pname, (const GLfloat *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glMaterialiv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 face = env->localvar_getInt(runtime, pos++);
    s32 pname = env->localvar_getInt(runtime, pos++);
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glMaterialiv((GLenum) face, (GLenum) pname, (const GLint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glGetMaterialfv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 face = env->localvar_getInt(runtime, pos++);
    s32 pname = env->localvar_getInt(runtime, pos++);
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glGetMaterialfv((GLenum) face, (GLenum) pname, (GLfloat *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glGetMaterialiv(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 face = env->localvar_getInt(runtime, pos++);
    s32 pname = env->localvar_getInt(runtime, pos++);
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glGetMaterialiv((GLenum) face, (GLenum) pname, (GLint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glColorMaterial(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 face = env->localvar_getInt(runtime, pos++);
    s32 mode = env->localvar_getInt(runtime, pos++);
    glColorMaterial((GLenum) face, (GLenum) mode);
    return 0;
}

int org_mini_gl_GL_glNewList(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 list = env->localvar_getInt(runtime, pos++);
    s32 mode = env->localvar_getInt(runtime, pos++);
    glNewList((GLuint) list, (GLenum) mode);
    return 0;
}

int org_mini_gl_GL_glGenLists(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 range = env->localvar_getInt(runtime, pos++);
    env->push_int(runtime->stack, glGenLists((GLsizei) range));
    return 0;
}

int org_mini_gl_GL_glDeleteLists(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 list = env->localvar_getInt(runtime, pos++);
    s32 range = env->localvar_getInt(runtime, pos++);
    glDeleteLists((GLuint) list, (GLsizei) range);
    return 0;
}

int org_mini_gl_GL_glEndList(Runtime *runtime, Class *clazz) {
    glEndList();
    return 0;
}

int org_mini_gl_GL_glCallList(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 list = env->localvar_getInt(runtime, pos++);
    glCallList((GLuint) list);
    return 0;
}

int org_mini_gl_GL_glIsList(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 list = env->localvar_getInt(runtime, pos++);
    env->push_int(runtime->stack, glIsList((GLuint) list) == GL_TRUE);
    return 0;
}

int org_mini_gl_GL_glListBase(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 base = env->localvar_getInt(runtime, pos++);
    glListBase((GLuint) base);
    return 0;
}

int org_mini_gl_GL_glCallLists(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 n = env->localvar_getInt(runtime, pos++);
    s32 type = env->localvar_getInt(runtime, pos++);
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glCallLists((GLsizei) n, (GLenum) type, (const GLvoid *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glDrawArrays(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 mode = env->localvar_getInt(runtime, pos++);
    s32 first = env->localvar_getInt(runtime, pos++);
    s32 count = env->localvar_getInt(runtime, pos++);
    glDrawArrays((GLenum) mode, (GLint) first, (GLsizei) count);
    return 0;
}

int org_mini_gl_GL_glBindVertexArray(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 array = env->localvar_getInt(runtime, pos++);
    glBindVertexArray((GLuint) array);
    return 0;
}

int org_mini_gl_GL_glEnableVertexAttribArray(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 index = env->localvar_getInt(runtime, pos++);
    glEnableVertexAttribArray((GLuint) index);
    return 0;
}

int org_mini_gl_GL_glUseProgram(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 program = env->localvar_getInt(runtime, pos++);
    glUseProgram((GLuint) program);
    return 0;
}

int org_mini_gl_GL_glLinkProgram(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 program = env->localvar_getInt(runtime, pos++);
    glLinkProgram((GLuint) program);
    return 0;
}

int org_mini_gl_GL_glCreateShader(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 type = env->localvar_getInt(runtime, pos++);
    env->push_int(runtime->stack, glCreateShader((GLenum) type));
    return 0;
}

int org_mini_gl_GL_glCreateProgram(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    env->push_int(runtime->stack, glCreateProgram());
    return 0;
}

int org_mini_gl_GL_glCompileShader(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 shader = env->localvar_getInt(runtime, pos++);
    glCompileShader((GLuint) shader);

    GLint compileResult = GL_TRUE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);
    if (compileResult == GL_FALSE) {
        char szLog[1024] = {0};
        GLsizei logLen = 0;
        glGetShaderInfoLog(shader, 1024, &logLen, szLog);
        fprintf(stderr, "Compile Shader fail error log: %s \nshader code:\n", szLog);
        glDeleteShader(shader);
        shader = 0;
    }
    return 0;
}

int org_mini_gl_GL_glShaderSource(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 shader = env->localvar_getInt(runtime, pos++);
    Instance *barr = env->localvar_getRefer(runtime, pos++);
    const GLchar *src = barr->arr_body;

    glShaderSource((GLuint) shader, 1, &src, NULL);
    return 0;
}

int org_mini_gl_GL_glAttachShader(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 program = env->localvar_getInt(runtime, pos++);
    s32 shader = env->localvar_getInt(runtime, pos++);
    glAttachShader((GLuint) program, (GLuint) shader);
    return 0;
}

int org_mini_gl_GL_glLoadIdentity(Runtime *runtime, Class *clazz) {
    glLoadIdentity();
    return 0;
}

int org_mini_gl_GL_glFrustum(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Long2Double left, right, bottom, top, near_val, far_val;
    left.l = getParaLong(runtime, pos);
    pos += 2;
    right.l = getParaLong(runtime, pos);
    pos += 2;
    bottom.l = getParaLong(runtime, pos);
    pos += 2;
    top.l = getParaLong(runtime, pos);
    pos += 2;
    near_val.l = getParaLong(runtime, pos);
    pos += 2;
    far_val.l = getParaLong(runtime, pos);
    pos += 2;
    glFrustum((GLdouble) left.d, (GLdouble) right.d, (GLdouble) bottom.d, (GLdouble) top.d, (GLdouble) near_val.d,
              (GLdouble) far_val.d);
    return 0;
}

int org_mini_gl_GL_glBufferData(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 target = env->localvar_getInt(runtime, pos++);
    s64 size = getParaLong(runtime, pos);
    pos += 2;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    s32 usage = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glBufferData((GLenum) target, (GLsizeiptr) size, (const GLvoid *) (arr->arr_body + offset), (GLenum) usage);
    return 0;
}

int org_mini_gl_GL_glGenBuffers(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 n = env->localvar_getInt(runtime, pos++);
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glGenBuffers((GLsizei) n, (GLuint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glGenVertexArrays(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 n = env->localvar_getInt(runtime, pos++);
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glGenVertexArrays((GLsizei) n, (GLuint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glDeleteVertexArrays(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 n = env->localvar_getInt(runtime, pos++);
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glDeleteVertexArrays((GLsizei) n, (GLuint *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glBindBuffer(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 target = env->localvar_getInt(runtime, pos++);
    s32 buffer = env->localvar_getInt(runtime, pos++);
    glBindBuffer((GLenum) target, (GLuint) buffer);
    return 0;
}

int org_mini_gl_GL_glPointSize(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float i2f;
    i2f.i = env->localvar_getInt(runtime, pos++);
    glPointSize((GLfloat) i2f.f);
    return 0;
}

int org_mini_gl_GL_glLineWidth(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Int2Float i2f;
    i2f.i = env->localvar_getInt(runtime, pos++);
    glLineWidth((GLfloat) i2f.f);
    return 0;
}


int org_mini_gl_GL_glPolygonStipple(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    Instance *arr = env->localvar_getRefer(runtime, pos++);
    s32 offset = env->localvar_getInt(runtime, pos++);
    offset *= env->data_type_bytes[arr->mb.arr_type_index];
    glPolygonStipple((const GLbyte *) (arr->arr_body + offset));
    return 0;
}

int org_mini_gl_GL_glGetError(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    env->push_int(runtime->stack, glGetError());
    return 0;
}

int org_mini_gl_GL_glGetString(Runtime *runtime, Class *clazz) {
    JniEnv *env = runtime->jnienv;
    s32 pos = 0;
    s32 name = env->localvar_getInt(runtime, pos++);
    c8 *cstr = (c8 *) glGetString((GLenum) name);
    if (cstr) {
        Instance *jstr = createJavaString(runtime, cstr);
        env->push_ref(runtime->stack, jstr);
    } else {
        env->push_ref(runtime->stack, NULL);
    }
    return 0;
}

static java_native_method method_test2_table[] = {
        {"org/mini/glfw/utils/Gutil", "f2b",                        "([F[B)[B",                         org_mini_glfw_utils_Gutil_f2b},
        {"org/mini/glfw/utils/Gutil", "vec_add",                    "([F[F[F)[F",                       org_mini_glfw_utils_Gutil_vec_add},
        {"org/mini/glfw/utils/Gutil", "vec_sub",                    "([F[F[F)[F",                       org_mini_glfw_utils_Gutil_vec_sub},
        {"org/mini/glfw/utils/Gutil", "vec_scale",                  "([F[FF)[F",                        org_mini_glfw_utils_Gutil_vec_scale},
        {"org/mini/glfw/utils/Gutil", "vec_mul_inner",              "([F[F)[F",                         org_mini_glfw_utils_Gutil_vec_mul_inner},
        {"org/mini/glfw/utils/Gutil", "vec_len",                    "([F)F",                            org_mini_glfw_utils_Gutil_vec_len},
        {"org/mini/glfw/utils/Gutil", "vec_normal",                 "([F[F)[F",                         org_mini_glfw_utils_Gutil_vec_normal},
        {"org/mini/glfw/utils/Gutil", "vec_mul_cross",              "([F[F[F)[F",                       org_mini_glfw_utils_Gutil_vec_mul_cross},
        {"org/mini/glfw/utils/Gutil", "vec_reflect",                "([F[F[F)[F",                       org_mini_glfw_utils_Gutil_vec_reflect},
        {"org/mini/glfw/utils/Gutil", "mat4x4_identity",            "([F)[F",                           org_mini_glfw_utils_Gutil_mat4x4_identity},
        {"org/mini/glfw/utils/Gutil", "mat4x4_dup",                 "([F[F)[F",                         org_mini_glfw_utils_Gutil_mat4x4_dup},
        {"org/mini/glfw/utils/Gutil", "mat4x4_row",                 "([F[FI)[F",                        org_mini_glfw_utils_Gutil_mat4x4_row},
        {"org/mini/glfw/utils/Gutil", "mat4x4_col",                 "([F[FI)[F",                        org_mini_glfw_utils_Gutil_mat4x4_col},
        {"org/mini/glfw/utils/Gutil", "mat4x4_transpose",           "([F[F)[F",                         org_mini_glfw_utils_Gutil_mat4x4_transpose},
        {"org/mini/glfw/utils/Gutil", "mat4x4_add",                 "([F[F[F)[F",                       org_mini_glfw_utils_Gutil_mat4x4_add},
        {"org/mini/glfw/utils/Gutil", "mat4x4_sub",                 "([F[F[F)[F",                       org_mini_glfw_utils_Gutil_mat4x4_sub},
        {"org/mini/glfw/utils/Gutil", "mat4x4_mul",                 "([F[F[F)[F",                       org_mini_glfw_utils_Gutil_mat4x4_mul},
        {"org/mini/glfw/utils/Gutil", "mat4x4_mul_vec4",            "([F[F[F)[F",                       org_mini_glfw_utils_Gutil_mat4x4_mul_vec4},
        {"org/mini/glfw/utils/Gutil", "mat4x4_from_vec3_mul_outer", "([F[F[F)[F",                       org_mini_glfw_utils_Gutil_mat4x4_from_vec3_mul_outer},
        {"org/mini/glfw/utils/Gutil", "mat4x4_translate",           "([FFFF)[F",                        org_mini_glfw_utils_Gutil_mat4x4_translate},
        {"org/mini/glfw/utils/Gutil", "mat4x4_translate_in_place",  "([FFFF)[F",                        org_mini_glfw_utils_Gutil_mat4x4_translate_in_place},
        {"org/mini/glfw/utils/Gutil", "mat4x4_scale",               "([F[FF)[F",                        org_mini_glfw_utils_Gutil_mat4x4_scale},
        {"org/mini/glfw/utils/Gutil", "mat4x4_scale_aniso",         "([F[FFFF)[F",                      org_mini_glfw_utils_Gutil_mat4x4_scale_aniso},
        {"org/mini/glfw/utils/Gutil", "mat4x4_rotate",              "([F[FFFFF)[F",                     org_mini_glfw_utils_Gutil_mat4x4_rotate},
        {"org/mini/glfw/utils/Gutil", "mat4x4_rotateX",             "([F[FF)[F",                        org_mini_glfw_utils_Gutil_mat4x4_rotateX},
        {"org/mini/glfw/utils/Gutil", "mat4x4_rotateY",             "([F[FF)[F",                        org_mini_glfw_utils_Gutil_mat4x4_rotateY},
        {"org/mini/glfw/utils/Gutil", "mat4x4_rotateZ",             "([F[FF)[F",                        org_mini_glfw_utils_Gutil_mat4x4_rotateZ},
        {"org/mini/glfw/utils/Gutil", "mat4x4_invert",              "([F[F)[F",                         org_mini_glfw_utils_Gutil_mat4x4_invert},
        {"org/mini/glfw/utils/Gutil", "mat4x4_orthonormalize",      "([F[F)[F",                         org_mini_glfw_utils_Gutil_mat4x4_orthonormalize},
        {"org/mini/glfw/utils/Gutil", "mat4x4_ortho",               "([FFFFFFF)[F",                     org_mini_glfw_utils_Gutil_mat4x4_ortho},
        {"org/mini/glfw/utils/Gutil", "mat4x4_frustum",             "([FFFFFFF)[F",                     org_mini_glfw_utils_Gutil_mat4x4_frustum},
        {"org/mini/glfw/utils/Gutil", "mat4x4_perspective",         "([FFFFF)[F",                       org_mini_glfw_utils_Gutil_mat4x4_perspective},
        {"org/mini/glfw/utils/Gutil", "mat4x4_look_at",             "([F[F[F[F)[F",                     org_mini_glfw_utils_Gutil_mat4x4_look_at},
        {"org/mini/glfw/Glfw",        "glfwGetTime",                "()D",                              org_mini_glfw_Glfw_glfwGetTime},
        {"org/mini/glfw/Glfw",        "glfwSetTime",                "(D)V",                             org_mini_glfw_Glfw_glfwSetTime},
        {"org/mini/glfw/Glfw",        "glfwCreateWindowJni",        "(II[BJJ)J",                        org_mini_glfw_Glfw_glfwCreateWindowJni},
        {"org/mini/glfw/Glfw",        "glfwDestroyWindow",          "(J)V",                             org_mini_glfw_Glfw_glfwDestroyWindow},
        {"org/mini/glfw/Glfw",        "glfwWindowShouldClose",      "(J)Z",                             org_mini_glfw_Glfw_glfwWindowShouldClose},
        {"org/mini/glfw/Glfw",        "glfwSetCallbackJni",         "(JLorg/mini/glfw/GlfwCallback;)V", org_mini_glfw_Glfw_glfwSetCallbackJni},
        {"org/mini/glfw/Glfw",        "glfwInitJni",                "()Z",                              org_mini_glfw_Glfw_glfwInitJni},
        {"org/mini/glfw/Glfw",        "glfwTerminate",              "()V",                              org_mini_glfw_Glfw_glfwTerminate},
        {"org/mini/glfw/Glfw",        "glfwWindowHint",             "(II)V",                            org_mini_glfw_Glfw_glfwWindowHint},
        {"org/mini/glfw/Glfw",        "glfwPollEvents",             "()V",                              org_mini_glfw_Glfw_glfwPollEvents},
        {"org/mini/glfw/Glfw",        "glfwSetWindowShouldClose",   "(JI)V",                            org_mini_glfw_Glfw_glfwSetWindowShouldClose},
        {"org/mini/glfw/Glfw",        "glfwMakeContextCurrentJni",  "(J)V",                             org_mini_glfw_Glfw_glfwMakeContextCurrentJni},
        {"org/mini/glfw/Glfw",        "glfwSwapInterval",           "(I)V",                             org_mini_glfw_Glfw_glfwSwapInterval},
        {"org/mini/glfw/Glfw",        "glfwSwapBuffers",            "(J)V",                             org_mini_glfw_Glfw_glfwSwapBuffers},
        {"org/mini/glfw/Glfw",        "glfwGetFramebufferSizeW",    "(J)I",                             org_mini_glfw_Glfw_glfwGetFramebufferSizeW},
        {"org/mini/glfw/Glfw",        "glfwGetFramebufferSizeH",    "(J)I",                             org_mini_glfw_Glfw_glfwGetFramebufferSizeH},
        {"org/mini/glfw/Glfw",        "glfwSetWindowAspectRatio",   "(JII)V",                           org_mini_glfw_Glfw_glfwSetWindowAspectRatio},
        {"org/mini/glfw/Glfw",        "glfwGetClipboardString",     "(J)Ljava/lang/String;",            org_mini_glfw_Glfw_glfwGetClipboardString},
        {"org/mini/gl/GL",            "init",                       "()V",                              org_mini_gl_GL_init},
        {"org/mini/gl/GL",            "glViewport",                 "(IIII)V",                          org_mini_gl_GL_glViewport},
        {"org/mini/gl/GL",            "glMatrixMode",               "(I)V",                             org_mini_gl_GL_glMatrixMode},
        {"org/mini/gl/GL",            "glLoadMatrixf",              "([FI)V",                           org_mini_gl_GL_glLoadMatrixf},
        {"org/mini/gl/GL",            "glPushMatrix",               "()V",                              org_mini_gl_GL_glPushMatrix},
        {"org/mini/gl/GL",            "glPopMatrix",                "()V",                              org_mini_gl_GL_glPopMatrix},
        {"org/mini/gl/GL",            "glEnable",                   "(I)V",                             org_mini_gl_GL_glEnable},
        {"org/mini/gl/GL",            "glDisable",                  "(I)V",                             org_mini_gl_GL_glDisable},
        {"org/mini/gl/GL",            "glShadeModel",               "(I)V",                             org_mini_gl_GL_glShadeModel},
        {"org/mini/gl/GL",            "glClear",                    "(I)V",                             org_mini_gl_GL_glClear},
        {"org/mini/gl/GL",            "glHint",                     "(II)V",                            org_mini_gl_GL_glHint},
        {"org/mini/gl/GL",            "glClearDepth",               "(D)V",                             org_mini_gl_GL_glClearDepth},
        {"org/mini/gl/GL",            "glDepthRange",               "(DD)V",                            org_mini_gl_GL_glDepthRange},
        {"org/mini/gl/GL",            "glDepthFunc",                "(I)V",                             org_mini_gl_GL_glDepthFunc},
        {"org/mini/gl/GL",            "glDepthMask",                "(I)V",                             org_mini_gl_GL_glDepthMask},
        {"org/mini/gl/GL",            "glDrawBuffer",               "(I)V",                             org_mini_gl_GL_glDrawBuffer},
        {"org/mini/gl/GL",            "glReadBuffer",               "(I)V",                             org_mini_gl_GL_glReadBuffer},
        {"org/mini/gl/GL",            "glTranslatef",               "(FFF)V",                           org_mini_gl_GL_glTranslatef},
        {"org/mini/gl/GL",            "glTranslated",               "(DDD)V",                           org_mini_gl_GL_glTranslated},
        {"org/mini/gl/GL",            "glScalef",                   "(FFF)V",                           org_mini_gl_GL_glScalef},
        {"org/mini/gl/GL",            "glScaled",                   "(DDD)V",                           org_mini_gl_GL_glScaled},
        {"org/mini/gl/GL",            "glRotatef",                  "(FFFF)V",                          org_mini_gl_GL_glRotatef},
        {"org/mini/gl/GL",            "glRotated",                  "(DDDD)V",                          org_mini_gl_GL_glRotated},
        {"org/mini/gl/GL",            "glCullFace",                 "(I)V",                             org_mini_gl_GL_glCullFace},
        {"org/mini/gl/GL",            "glFrontFace",                "(I)V",                             org_mini_gl_GL_glFrontFace},
        {"org/mini/gl/GL",            "glBlendFunc",                "(II)V",                            org_mini_gl_GL_glBlendFunc},
        {"org/mini/gl/GL",            "glAlphaFunc",                "(IF)V",                            org_mini_gl_GL_glAlphaFunc},
        {"org/mini/gl/GL",            "glIndexMask",                "(I)V",                             org_mini_gl_GL_glIndexMask},
        {"org/mini/gl/GL",            "glLogicOp",                  "(I)V",                             org_mini_gl_GL_glLogicOp},
        {"org/mini/gl/GL",            "glColorMask",                "(ZZZZ)V",                          org_mini_gl_GL_glColorMask},
        {"org/mini/gl/GL",            "glBegin",                    "(I)V",                             org_mini_gl_GL_glBegin},
        {"org/mini/gl/GL",            "glEnd",                      "()V",                              org_mini_gl_GL_glEnd},
        {"org/mini/gl/GL",            "glFlush",                    "()V",                              org_mini_gl_GL_glFlush},
        {"org/mini/gl/GL",            "glFinish",                   "()V",                              org_mini_gl_GL_glFinish},
        {"org/mini/gl/GL",            "glColor3f",                  "(FFF)V",                           org_mini_gl_GL_glColor3f},
        {"org/mini/gl/GL",            "glColor4f",                  "(FFFF)V",                          org_mini_gl_GL_glColor4f},
        {"org/mini/gl/GL",            "glColor3d",                  "(DDD)V",                           org_mini_gl_GL_glColor3d},
        {"org/mini/gl/GL",            "glColor4d",                  "(DDDD)V",                          org_mini_gl_GL_glColor4d},
        {"org/mini/gl/GL",            "glColor3i",                  "(III)V",                           org_mini_gl_GL_glColor3i},
        {"org/mini/gl/GL",            "glColor4i",                  "(IIII)V",                          org_mini_gl_GL_glColor4i},
        {"org/mini/gl/GL",            "glColor3ui",                 "(III)V",                           org_mini_gl_GL_glColor3ui},
        {"org/mini/gl/GL",            "glColor4ui",                 "(IIII)V",                          org_mini_gl_GL_glColor4ui},
        {"org/mini/gl/GL",            "glColor3b",                  "(BBB)V",                           org_mini_gl_GL_glColor3b},
        {"org/mini/gl/GL",            "glColor4b",                  "(BBBB)V",                          org_mini_gl_GL_glColor4b},
        {"org/mini/gl/GL",            "glColor3ub",                 "(BBB)V",                           org_mini_gl_GL_glColor3ub},
        {"org/mini/gl/GL",            "glColor4ub",                 "(BBBB)V",                          org_mini_gl_GL_glColor4ub},
        {"org/mini/gl/GL",            "glColor3s",                  "(SSS)V",                           org_mini_gl_GL_glColor3s},
        {"org/mini/gl/GL",            "glColor4s",                  "(SSSS)V",                          org_mini_gl_GL_glColor4s},
        {"org/mini/gl/GL",            "glColor3us",                 "(SSS)V",                           org_mini_gl_GL_glColor3us},
        {"org/mini/gl/GL",            "glColor4us",                 "(SSSS)V",                          org_mini_gl_GL_glColor4us},
        {"org/mini/gl/GL",            "glColor3fv",                 "([FI)V",                           org_mini_gl_GL_glColor3fv},
        {"org/mini/gl/GL",            "glColor3dv",                 "([DI)V",                           org_mini_gl_GL_glColor3dv},
        {"org/mini/gl/GL",            "glColor3iv",                 "([II)V",                           org_mini_gl_GL_glColor3iv},
        {"org/mini/gl/GL",            "glColor3bv",                 "([BI)V",                           org_mini_gl_GL_glColor3bv},
        {"org/mini/gl/GL",            "glColor3sv",                 "([SI)V",                           org_mini_gl_GL_glColor3sv},
        {"org/mini/gl/GL",            "glColor3uiv",                "([II)V",                           org_mini_gl_GL_glColor3uiv},
        {"org/mini/gl/GL",            "glColor3ubv",                "([BI)V",                           org_mini_gl_GL_glColor3ubv},
        {"org/mini/gl/GL",            "glColor3usv",                "([SI)V",                           org_mini_gl_GL_glColor3usv},
        {"org/mini/gl/GL",            "glColor4fv",                 "([FI)V",                           org_mini_gl_GL_glColor4fv},
        {"org/mini/gl/GL",            "glColor4dv",                 "([DI)V",                           org_mini_gl_GL_glColor4dv},
        {"org/mini/gl/GL",            "glColor4iv",                 "([II)V",                           org_mini_gl_GL_glColor4iv},
        {"org/mini/gl/GL",            "glColor4bv",                 "([BI)V",                           org_mini_gl_GL_glColor4bv},
        {"org/mini/gl/GL",            "glColor4sv",                 "([SI)V",                           org_mini_gl_GL_glColor4sv},
        {"org/mini/gl/GL",            "glColor4uiv",                "([II)V",                           org_mini_gl_GL_glColor4uiv},
        {"org/mini/gl/GL",            "glColor4ubv",                "([BI)V",                           org_mini_gl_GL_glColor4ubv},
        {"org/mini/gl/GL",            "glColor4usv",                "([SI)V",                           org_mini_gl_GL_glColor4usv},
        {"org/mini/gl/GL",            "glClearColor",               "(FFFF)V",                          org_mini_gl_GL_glClearColor},
        {"org/mini/gl/GL",            "glVertex2s",                 "(SS)V",                            org_mini_gl_GL_glVertex2s},
        {"org/mini/gl/GL",            "glVertex3s",                 "(SSS)V",                           org_mini_gl_GL_glVertex3s},
        {"org/mini/gl/GL",            "glVertex4s",                 "(SSSS)V",                          org_mini_gl_GL_glVertex4s},
        {"org/mini/gl/GL",            "glVertex2i",                 "(II)V",                            org_mini_gl_GL_glVertex2i},
        {"org/mini/gl/GL",            "glVertex3i",                 "(III)V",                           org_mini_gl_GL_glVertex3i},
        {"org/mini/gl/GL",            "glVertex4i",                 "(IIII)V",                          org_mini_gl_GL_glVertex4i},
        {"org/mini/gl/GL",            "glVertex2f",                 "(FF)V",                            org_mini_gl_GL_glVertex2f},
        {"org/mini/gl/GL",            "glVertex3f",                 "(FFF)V",                           org_mini_gl_GL_glVertex3f},
        {"org/mini/gl/GL",            "glVertex4f",                 "(FFFF)V",                          org_mini_gl_GL_glVertex4f},
        {"org/mini/gl/GL",            "glVertex2d",                 "(DD)V",                            org_mini_gl_GL_glVertex2d},
        {"org/mini/gl/GL",            "glVertex3d",                 "(DDD)V",                           org_mini_gl_GL_glVertex3d},
        {"org/mini/gl/GL",            "glVertex4d",                 "(DDDD)V",                          org_mini_gl_GL_glVertex4d},
        {"org/mini/gl/GL",            "glVertex2sv",                "([SI)V",                           org_mini_gl_GL_glVertex2sv},
        {"org/mini/gl/GL",            "glVertex3sv",                "([SI)V",                           org_mini_gl_GL_glVertex3sv},
        {"org/mini/gl/GL",            "glVertex4sv",                "([SI)V",                           org_mini_gl_GL_glVertex4sv},
        {"org/mini/gl/GL",            "glVertex2iv",                "([II)V",                           org_mini_gl_GL_glVertex2iv},
        {"org/mini/gl/GL",            "glVertex3iv",                "([II)V",                           org_mini_gl_GL_glVertex3iv},
        {"org/mini/gl/GL",            "glVertex4iv",                "([II)V",                           org_mini_gl_GL_glVertex4iv},
        {"org/mini/gl/GL",            "glVertex2fv",                "([FI)V",                           org_mini_gl_GL_glVertex2fv},
        {"org/mini/gl/GL",            "glVertex3fv",                "([FI)V",                           org_mini_gl_GL_glVertex3fv},
        {"org/mini/gl/GL",            "glVertex4fv",                "([FI)V",                           org_mini_gl_GL_glVertex4fv},
        {"org/mini/gl/GL",            "glVertex2dv",                "([DI)V",                           org_mini_gl_GL_glVertex2dv},
        {"org/mini/gl/GL",            "glVertex3dv",                "([DI)V",                           org_mini_gl_GL_glVertex3dv},
        {"org/mini/gl/GL",            "glVertex4dv",                "([DI)V",                           org_mini_gl_GL_glVertex4dv},
        {"org/mini/gl/GL",            "glNormal3b",                 "(BBB)V",                           org_mini_gl_GL_glNormal3b},
        {"org/mini/gl/GL",            "glNormal3s",                 "(SSS)V",                           org_mini_gl_GL_glNormal3s},
        {"org/mini/gl/GL",            "glNormal3i",                 "(III)V",                           org_mini_gl_GL_glNormal3i},
        {"org/mini/gl/GL",            "glNormal3f",                 "(FFF)V",                           org_mini_gl_GL_glNormal3f},
        {"org/mini/gl/GL",            "glNormal3d",                 "(DDD)V",                           org_mini_gl_GL_glNormal3d},
        {"org/mini/gl/GL",            "glNormal3bv",                "([BI)V",                           org_mini_gl_GL_glNormal3bv},
        {"org/mini/gl/GL",            "glNormal3sv",                "([SI)V",                           org_mini_gl_GL_glNormal3sv},
        {"org/mini/gl/GL",            "glNormal3iv",                "([II)V",                           org_mini_gl_GL_glNormal3iv},
        {"org/mini/gl/GL",            "glNormal3fv",                "([FI)V",                           org_mini_gl_GL_glNormal3fv},
        {"org/mini/gl/GL",            "glNormal3dv",                "([DI)V",                           org_mini_gl_GL_glNormal3dv},
        {"org/mini/gl/GL",            "glRectf",                    "(FFFF)V",                          org_mini_gl_GL_glRectf},
        {"org/mini/gl/GL",            "glLightfv",                  "(II[FI)V",                         org_mini_gl_GL_glLightfv},
        {"org/mini/gl/GL",            "glLightiv",                  "(II[II)V",                         org_mini_gl_GL_glLightiv},
        {"org/mini/gl/GL",            "glMaterialf",                "(IIF)V",                           org_mini_gl_GL_glMaterialf},
        {"org/mini/gl/GL",            "glMateriali",                "(III)V",                           org_mini_gl_GL_glMateriali},
        {"org/mini/gl/GL",            "glMaterialfv",               "(II[FI)V",                         org_mini_gl_GL_glMaterialfv},
        {"org/mini/gl/GL",            "glMaterialiv",               "(II[II)V",                         org_mini_gl_GL_glMaterialiv},
        {"org/mini/gl/GL",            "glGetMaterialfv",            "(II[FI)V",                         org_mini_gl_GL_glGetMaterialfv},
        {"org/mini/gl/GL",            "glGetMaterialiv",            "(II[II)V",                         org_mini_gl_GL_glGetMaterialiv},
        {"org/mini/gl/GL",            "glColorMaterial",            "(II)V",                            org_mini_gl_GL_glColorMaterial},
        {"org/mini/gl/GL",            "glNewList",                  "(II)V",                            org_mini_gl_GL_glNewList},
        {"org/mini/gl/GL",            "glDeleteLists",              "(II)V",                            org_mini_gl_GL_glDeleteLists},
        {"org/mini/gl/GL",            "glGenLists",                 "(I)I",                             org_mini_gl_GL_glGenLists},
        {"org/mini/gl/GL",            "glIsList",                   "(I)Z",                             org_mini_gl_GL_glIsList},
        {"org/mini/gl/GL",            "glListBase",                 "(I)V",                             org_mini_gl_GL_glListBase},
        {"org/mini/gl/GL",            "glEndList",                  "()V",                              org_mini_gl_GL_glEndList},
        {"org/mini/gl/GL",            "glCallList",                 "(I)V",                             org_mini_gl_GL_glCallList},
        {"org/mini/gl/GL",            "glCallLists",                "(I)V",                             org_mini_gl_GL_glCallLists},
        {"org/mini/gl/GL",            "glDrawArrays",               "(III)V",                           org_mini_gl_GL_glDrawArrays},
        {"org/mini/gl/GL",            "glGenVertexArrays",          "(I[II)V",                          org_mini_gl_GL_glGenVertexArrays},
        {"org/mini/gl/GL",            "glDeleteVertexArrays",       "(I[II)V",                          org_mini_gl_GL_glDeleteVertexArrays},
        {"org/mini/gl/GL",            "glBindVertexArray",          "(I)V",                             org_mini_gl_GL_glBindVertexArray},
        {"org/mini/gl/GL",            "glEnableVertexAttribArray",  "(I)V",                             org_mini_gl_GL_glEnableVertexAttribArray},
        {"org/mini/gl/GL",            "glVertexAttribPointer",      "(IIIIII)V",                        org_mini_gl_GL_glVertexAttribPointer},
        {"org/mini/gl/GL",            "glLoadIdentity",             "()V",                              org_mini_gl_GL_glLoadIdentity},
        {"org/mini/gl/GL",            "glFrustum",                  "(DDDDDD)V",                        org_mini_gl_GL_glFrustum},
        {"org/mini/gl/GL",            "glShaderSource",             "(I[B)V",                           org_mini_gl_GL_glShaderSource},
        {"org/mini/gl/GL",            "glCompileShader",            "(I)V",                             org_mini_gl_GL_glCompileShader},
        {"org/mini/gl/GL",            "glAttachShader",             "(II)V",                            org_mini_gl_GL_glAttachShader},
        {"org/mini/gl/GL",            "glUseProgram",               "(I)V",                             org_mini_gl_GL_glUseProgram},
        {"org/mini/gl/GL",            "glLinkProgram",              "(I)V",                             org_mini_gl_GL_glLinkProgram},
        {"org/mini/gl/GL",            "glCreateShader",             "(I)I",                             org_mini_gl_GL_glCreateShader},
        {"org/mini/gl/GL",            "glCreateProgram",            "()I",                              org_mini_gl_GL_glCreateProgram},
        {"org/mini/gl/GL",            "glBufferData",               "(IJLjava/lang/Object;II)V",        org_mini_gl_GL_glBufferData},
        {"org/mini/gl/GL",            "glGenBuffers",               "(I[II)V",                          org_mini_gl_GL_glGenBuffers},
        {"org/mini/gl/GL",            "glBindBuffer",               "(II)V",                            org_mini_gl_GL_glBindBuffer},
        {"org/mini/gl/GL",            "glPointSize",                "(F)V",                             org_mini_gl_GL_glPointSize},
        {"org/mini/gl/GL",            "glLineWidth",                "(F)V",                             org_mini_gl_GL_glLineWidth},
        {"org/mini/gl/GL",            "glPolygonStipple",           "([BI)V",                           org_mini_gl_GL_glPolygonStipple},
        {"org/mini/gl/GL",            "glGetError",                 "()I",                              org_mini_gl_GL_glGetError},
        {"org/mini/gl/GL",            "glGetString",                "(I)Ljava/lang/String;",            org_mini_gl_GL_glGetString},
};

void JNI_OnLoad(JniEnv *env) {
    memset(&refers, 0, sizeof(GlobeRefer));
    refers.env = env;
    env->native_reg_lib(&(method_test2_table[0]), sizeof(method_test2_table) / sizeof(java_native_method));
}

void JNI_OnUnload(JniEnv *env) {
    env->native_remove_lib((__refer) &method_test2_table[0]);
}


/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/

int main(void) {

    return 0;
}
