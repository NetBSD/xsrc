/*
 * KHR_vg_parent_image extension test
 *
 * This test aims to validate KHR_vg_parent_image extension which provides
 * a mechanism for creating EGLImage objects from OpenVG VGImage resources,
 * and then bind EGLImage with texture which will be used by OpenGL API.
 *
 * VGImage->EGLImage->Texture
 *
 * Cooper Yuan <cooperyuan@gmail.com>
 * 20 Aug 2011
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <VG/openvg.h>
#include <GL/glu.h>
#include <EGL/egl.h>

#include <EGL/eglext.h>

#define WINDOW_WIDTH   300
#define WINDOW_HEIGHT  300

static PFNEGLCREATEIMAGEKHRPROC    eglCreateImageKHR                    = NULL;
static PFNEGLDESTROYIMAGEKHRPROC   eglDestroyImageKHR                   = NULL;
static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = NULL;

typedef struct _egl_manager_t
{
    EGLNativeDisplayType xdpy;
    EGLNativeWindowType  xwin;

    EGLDisplay  dpy;
    EGLConfig   conf;

    // Rendering contexts
    EGLContext  vg_ctx;
    EGLContext  es_ctx;

    // Surfaces
    EGLSurface  win_surface;
    EGLSurface  pbuf_surface;

    VGImage     vg_image;
    EGLImageKHR egl_image;
    GLuint      texture;

    EGLint      major_ver, minor_ver;
}EGLmanager;


static EGLBoolean check_ext(EGLmanager *eglman)
{
    const char* egl_ext_str = NULL;
    egl_ext_str = eglQueryString(eglman->dpy, EGL_EXTENSIONS);

    // check extension KHR_vg_parent_image
    if (eglCreateImageKHR == NULL)
    {
        if (!strstr(egl_ext_str, "EGL_KHR_image"))
        {
            return EGL_FALSE;
        }
        else
        {
            eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
            eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
            if ((!eglCreateImageKHR) || (!eglDestroyImageKHR))
            {
                return EGL_FALSE;
            }
        }
    }

    // check extension GL_OES_EGL_image
    if (glEGLImageTargetTexture2DOES == NULL)
    {
        glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
        if (!glEGLImageTargetTexture2DOES)
        {
            return EGL_FALSE;
        }
    }

    return EGL_TRUE;
}

static EGLBoolean create_x_window(EGLmanager *eglman, const char *name)
{
    EGLint scrnum, num_conf, num_visuals;
    Window root;
    EGLint vid;
    XVisualInfo *visInfo, visTemplate;
    XSetWindowAttributes attr;
    unsigned long mask;

    EGLint config_attrib[] =
    {
        EGL_RED_SIZE,			1,
        EGL_GREEN_SIZE, 		1,
        EGL_BLUE_SIZE,			1,
        EGL_DEPTH_SIZE,         1,
        EGL_SURFACE_TYPE,       EGL_WINDOW_BIT|EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE,	EGL_OPENVG_BIT | EGL_OPENGL_BIT,
        EGL_NONE
    };

    scrnum = DefaultScreen(eglman->xdpy);
    root = RootWindow(eglman->xdpy, scrnum);

    if (!eglChooseConfig(eglman->dpy, config_attrib, &eglman->conf, 1, &num_conf) ||
        num_conf == 0 ||
        eglGetError() != EGL_SUCCESS)
    {
        printf("Error: couldn't get an EGL visual config\n");
        return EGL_FALSE;
    }

    if (!eglGetConfigAttrib(eglman->dpy, eglman->conf, EGL_NATIVE_VISUAL_ID, &vid) ||
        eglGetError() != EGL_SUCCESS)
    {
        printf("Error: eglGetConfigAttrib() failed\n");
        return EGL_FALSE;
    }

    /* The X window visual must match the EGL config */
    visTemplate.visualid = vid;
    visInfo = XGetVisualInfo(eglman->xdpy, VisualIDMask, &visTemplate, &num_visuals);
    if (!visInfo)
    {
        printf("Error: couldn't get X visual\n");
        return EGL_FALSE;
    }

    /* window attributes */
    attr.background_pixel = 0;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(eglman->xdpy, root, visInfo->visual, AllocNone);
    attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
    mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    eglman->xwin = XCreateWindow(eglman->xdpy, root, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                                 0, visInfo->depth, InputOutput,
                                 visInfo->visual, mask, &attr);

    if (!eglman->xwin)
    {
        printf("Error: couldn't create X Window\n");
        return EGL_FALSE;
    }

    /* set hints and properties */
    {
       XSizeHints sizehints;
       sizehints.x = 0;
       sizehints.y = 0;
       sizehints.width  = WINDOW_WIDTH;
       sizehints.height = WINDOW_HEIGHT;
       sizehints.flags = USSize | USPosition;
       XSetNormalHints(eglman->xdpy, eglman->xwin, &sizehints);
       XSetStandardProperties(eglman->xdpy, eglman->xwin, name, name,
                              None, (char **)NULL, 0, &sizehints);
    }

    XFree(visInfo);

    return EGL_TRUE;
}

static EGLBoolean egl_init(EGLmanager *eglman)
{
    EGLint pbuffer_attrib[] =
    {
        EGL_WIDTH,  128,
        EGL_HEIGHT, 128,
        EGL_NONE
    };

    // Check extension support
    if (check_ext(eglman) != EGL_TRUE)
    {
        return EGL_FALSE;
    }

    // Create GL context
    eglBindAPI(EGL_OPENGL_ES_API);
    eglman->es_ctx = eglCreateContext(eglman->dpy, eglman->conf, NULL, NULL);
    if (eglman->es_ctx == EGL_NO_CONTEXT ||
        eglGetError() != EGL_SUCCESS)
    {
        return EGL_FALSE;
    }

    // Create VG context
    eglBindAPI(EGL_OPENVG_API);
    eglman->vg_ctx = eglCreateContext(eglman->dpy, eglman->conf, NULL, NULL);
    if (eglman->vg_ctx == EGL_NO_CONTEXT ||
        eglGetError() != EGL_SUCCESS)
    {
        return EGL_FALSE;
    }

    // Create window surface
    eglman->win_surface = eglCreateWindowSurface(eglman->dpy, eglman->conf, eglman->xwin, NULL);
    if (eglman->win_surface == EGL_NO_SURFACE ||
        eglGetError() != EGL_SUCCESS)
    {
        return EGL_FALSE;
    }

    // Create pbuffer surface
    eglman->pbuf_surface = eglCreatePbufferSurface(eglman->dpy, eglman->conf, pbuffer_attrib);
    if (eglman->pbuf_surface == EGL_NO_SURFACE ||
        eglGetError() != EGL_SUCCESS)
    {

        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static void egl_deinit(EGLmanager *eglman)
{
    eglBindAPI(EGL_OPENGL_ES_API);
    eglMakeCurrent(eglman->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    eglBindAPI(EGL_OPENVG_API);
    eglMakeCurrent(eglman->dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    eglDestroySurface(eglman->dpy, eglman->win_surface);
    eglDestroySurface(eglman->dpy, eglman->pbuf_surface);

    eglDestroyContext(eglman->dpy, eglman->es_ctx);
    eglDestroyContext(eglman->dpy, eglman->vg_ctx);
}

static EGLBoolean vg_es_init(EGLmanager *eglman)
{
    // Initialize OpenVG
    eglBindAPI(EGL_OPENVG_API);
    eglMakeCurrent(eglman->dpy, eglman->pbuf_surface, eglman->pbuf_surface, eglman->vg_ctx);

    // Create VGImage
    eglman->vg_image = vgCreateImage(VG_sRGBA_8888, WINDOW_WIDTH, WINDOW_HEIGHT, VG_IMAGE_QUALITY_BETTER);

    // Create EGLImage from VGImage
    eglman->egl_image = (EGLImageKHR)eglCreateImageKHR(eglman->dpy, eglman->vg_ctx, EGL_VG_PARENT_IMAGE_KHR, (EGLClientBuffer)eglman->vg_image, NULL);

    // Initialize OpenGL ES
    eglBindAPI(EGL_OPENGL_ES_API);
    eglMakeCurrent(eglman->dpy, eglman->win_surface, eglman->win_surface, eglman->es_ctx);

    // Create Texture Target from EGLImage
    glGenTextures(1, &eglman->texture);
    glBindTexture(GL_TEXTURE_2D, eglman->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)eglman->egl_image);

    return EGL_TRUE;
}

static void vg_es_deinit(EGLmanager *eglman)
{
    // Destroy VG
    eglBindAPI(EGL_OPENVG_API);
    eglMakeCurrent(eglman->dpy, eglman->pbuf_surface, eglman->pbuf_surface, eglman->vg_ctx);
    eglDestroyImageKHR(eglman->dpy, eglman->egl_image);
    vgDestroyImage(eglman->vg_image);

    // Destroy GL
    eglBindAPI(EGL_OPENGL_ES_API);
    eglMakeCurrent(eglman->dpy, eglman->win_surface, eglman->win_surface, eglman->es_ctx);
    glDeleteTextures(1, &eglman->texture);
    glBindTexture(GL_TEXTURE_2D, 0);

}

static void draw(EGLmanager *eglman)
{
    static const GLfloat red[4] = {1.0, 0.2, 0.2, 1.0};
    static const GLfloat blue[4] = {0.2, 0.2, 1.0, 1.0};

    GLfloat pVertex[12] =
    {
        -1.f, -1.f, 0.f,
        1.f, -1.f, 0.f,
        -1.f, 1.f, 0.f,
        1.f, 1.f, 0.f
    };
    GLshort pTexCoord[8] =
    {
        0, 0,
        1, 0,
        0, 1,
        1, 1,
    };

    // Make current to VG content
    eglBindAPI(EGL_OPENVG_API);
    eglMakeCurrent(eglman->dpy, eglman->pbuf_surface, eglman->pbuf_surface, eglman->vg_ctx);

    vgSetfv(VG_CLEAR_COLOR, 4, red);
    vgClearImage(eglman->vg_image, 0, 0, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

    vgSetfv(VG_CLEAR_COLOR, 4, blue);
    vgClearImage(eglman->vg_image, WINDOW_WIDTH/2, WINDOW_HEIGHT/2, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

    // Make current to GL content
    eglBindAPI(EGL_OPENGL_ES_API);
    eglMakeCurrent(eglman->dpy, eglman->win_surface, eglman->win_surface, eglman->es_ctx);

    glViewport(WINDOW_WIDTH / 8, WINDOW_HEIGHT / 8, WINDOW_WIDTH * 3 / 4, WINDOW_HEIGHT * 3 / 4);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, eglman->texture);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState( GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, pVertex);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_SHORT, 0, pTexCoord);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Swap buffer
    eglSwapBuffers(eglman->dpy, eglman->win_surface);

    return;
}

int main(int argc, char **argv)
{
    const char *s;

    EGLmanager *eglman = calloc(1, sizeof(*eglman));

    // Open X Display
    Display *x_dpy = XOpenDisplay(NULL);
    if (!x_dpy)
    {
        printf("error: can't open default display\n");
        goto exit0;
    }
    eglman->xdpy = (EGLNativeDisplayType)x_dpy;

    // Get EGL Display
    eglman->dpy = eglGetDisplay(eglman->xdpy);
    if (!eglman->dpy || eglGetError() != EGL_SUCCESS)
    {
        printf("error: can't get EGL display\n");
        goto exit1;
    }

    // Initialize EGL
    eglInitialize(eglman->dpy, &eglman->major_ver, &eglman->minor_ver);
    if (eglGetError() != EGL_SUCCESS)
    {
        goto exit1;
    }

    // Query and print out information
    s = eglQueryString(eglman->dpy, EGL_VERSION);
    printf("EGL_VERSION = %s\n", s);

    s = eglQueryString(eglman->dpy, EGL_VENDOR);
    printf("EGL_VENDOR = %s\n", s);

    s = eglQueryString(eglman->dpy, EGL_EXTENSIONS);
    printf("EGL_EXTENSIONS = %s\n", s);

    s = eglQueryString(eglman->dpy, EGL_CLIENT_APIS);
    printf("EGL_CLIENT_APIS = %s\n", s);

    // Create an RGB, double-buffered X window
    if (create_x_window(eglman, "vgimage to texture") != EGL_TRUE)
    {
        goto exit2;
    }

    XMapWindow(eglman->xdpy, eglman->xwin);

    // Initialize EGL
    if (egl_init(eglman) != EGL_TRUE)
    {
        goto exit3;
    }

    // Initialize rendering API: OpenGL ES and OpenVG
    if (vg_es_init(eglman) != EGL_TRUE)
    {
        goto exit3;
    }

    // Rendering
    draw(eglman);

    // Deinitialize rendering API
    vg_es_deinit(eglman);

    // Deinitialize EGL
    egl_deinit(eglman);

exit3:
    XDestroyWindow(eglman->xdpy, eglman->xwin);
exit2:
    eglTerminate(eglman->dpy);
exit1:
    XCloseDisplay(eglman->xdpy);
exit0:
    free(eglman);

    return 0;
}
