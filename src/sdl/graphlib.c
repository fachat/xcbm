/**
 * graphlib.c - SDL2-based framebuffer display library implementation
 */

#include "graphlib.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* -------------------------------------------------------------------------
 * SDL2 type definitions
 *
 * When the SDL2 development headers are available at build time
 * (HAVE_SDL2_HEADERS is defined, typically via pkg-config), the real SDL2
 * types and constants are used directly.  Otherwise, minimal binary-compatible
 * definitions are provided so the library can be compiled without any SDL2
 * development package installed.
 * ---------------------------------------------------------------------- */

#ifdef HAVE_SDL2_HEADERS

/* --- SDL2 headers available: use real types with SDL2_ aliases ---------- */
#include <SDL2/SDL.h>

typedef SDL_Window        SDL2_Window;
typedef SDL_Renderer      SDL2_Renderer;
typedef SDL_Texture       SDL2_Texture;
typedef SDL_Rect          SDL2_Rect;
typedef SDL_Keysym        SDL2_Keysym;
typedef SDL_WindowEvent   SDL2_WindowEvent;
typedef SDL_KeyboardEvent SDL2_KeyboardEvent;
typedef SDL_Event         SDL2_Event;
typedef SDL_bool          SDL2_bool;
typedef Uint32            SDL2_Uint32;
typedef Uint16            SDL2_Uint16;
typedef Uint8             SDL2_Uint8;
typedef Sint32            SDL2_Sint32;

#define SDL2_INIT_VIDEO               SDL_INIT_VIDEO
#define SDL2_WINDOWPOS_CENTERED       SDL_WINDOWPOS_CENTERED
#define SDL2_WINDOW_SHOWN             SDL_WINDOW_SHOWN
#define SDL2_WINDOW_RESIZABLE         SDL_WINDOW_RESIZABLE
#define SDL2_RENDERER_SOFTWARE        SDL_RENDERER_SOFTWARE
#define SDL2_RENDERER_ACCELERATED     SDL_RENDERER_ACCELERATED
#define SDL2_RENDERER_PRESENTVSYNC    SDL_RENDERER_PRESENTVSYNC
#define SDL2_PIXELFORMAT_RGB332       SDL_PIXELFORMAT_RGB332
#define SDL2_TEXTUREACCESS_STREAMING  SDL_TEXTUREACCESS_STREAMING
#define SDL2_FALSE                    SDL_FALSE
#define SDL2_QUIT                     SDL_QUIT
#define SDL2_WINDOWEVENT              SDL_WINDOWEVENT
#define SDL2_KEYDOWN                  SDL_KEYDOWN
#define SDL2_KEYUP                    SDL_KEYUP
#define SDL2_WINDOWEVENT_RESIZED      SDL_WINDOWEVENT_RESIZED
#define SDL2_WINDOWEVENT_SIZE_CHANGED SDL_WINDOWEVENT_SIZE_CHANGED

#else /* HAVE_SDL2_HEADERS */

/* --- Minimal SDL2 type definitions (no SDL2 headers required) ----------- *
 *
 * These replicate the binary layout of the corresponding SDL2 types so that
 * the library can be compiled without the SDL2 development headers installed.
 * The definitions match the SDL2 2.x ABI on 32- and 64-bit little-endian
 * systems (all fields use fixed-width integer types).
 */

typedef uint32_t SDL2_Uint32;
typedef uint16_t SDL2_Uint16;
typedef uint8_t  SDL2_Uint8;
typedef int32_t  SDL2_Sint32;
typedef int      SDL2_bool;

/* Opaque SDL2 object types */
typedef struct SDL2_Window   SDL2_Window;
typedef struct SDL2_Renderer SDL2_Renderer;
typedef struct SDL2_Texture  SDL2_Texture;
typedef struct SDL2_Rect     SDL2_Rect;

/* SDL_Init flags */
#define SDL2_INIT_VIDEO         0x00000020u

/* SDL_CreateWindow position */
#define SDL2_WINDOWPOS_CENTERED 0x2FFF0000u

/* SDL_CreateWindow flags */
#define SDL2_WINDOW_SHOWN       0x00000004u
#define SDL2_WINDOW_RESIZABLE   0x00000020u

/* SDL_CreateRenderer flags */
#define SDL2_RENDERER_SOFTWARE     0x00000001u
#define SDL2_RENDERER_ACCELERATED  0x00000002u
#define SDL2_RENDERER_PRESENTVSYNC 0x00000004u

/* SDL_CreateTexture pixel format (RGB332) and access */
#define SDL2_PIXELFORMAT_RGB332      0x14110801u
#define SDL2_TEXTUREACCESS_STREAMING 1

/* SDL_bool values */
#define SDL2_FALSE 0

/* SDL_Event type values */
#define SDL2_QUIT        0x100u
#define SDL2_WINDOWEVENT 0x200u
#define SDL2_KEYDOWN     0x300u
#define SDL2_KEYUP       0x301u

/* SDL_WindowEventID values */
#define SDL2_WINDOWEVENT_RESIZED      5
#define SDL2_WINDOWEVENT_SIZE_CHANGED 6

/*
 * SDL_Keysym (16 bytes):
 *   scancode  Uint32  offset  0
 *   sym       Sint32  offset  4
 *   mod       Uint16  offset  8
 *   <2-byte padding>
 *   unused    Uint32  offset 12
 */
typedef struct {
    SDL2_Uint32 scancode;
    SDL2_Sint32 sym;
    SDL2_Uint16 mod;
    SDL2_Uint32 unused;   /* compiler inserts 2-byte pad before this */
} SDL2_Keysym;

/*
 * SDL_WindowEvent (24 bytes):
 *   type      Uint32  offset  0
 *   timestamp Uint32  offset  4
 *   windowID  Uint32  offset  8
 *   event     Uint8   offset 12
 *   padding1  Uint8   offset 13
 *   padding2  Uint8   offset 14
 *   padding3  Uint8   offset 15
 *   data1     Sint32  offset 16
 *   data2     Sint32  offset 20
 */
typedef struct {
    SDL2_Uint32 type;
    SDL2_Uint32 timestamp;
    SDL2_Uint32 windowID;
    SDL2_Uint8  event;
    SDL2_Uint8  padding1;
    SDL2_Uint8  padding2;
    SDL2_Uint8  padding3;
    SDL2_Sint32 data1;
    SDL2_Sint32 data2;
} SDL2_WindowEvent;

/*
 * SDL_KeyboardEvent (32 bytes):
 *   type      Uint32     offset  0
 *   timestamp Uint32     offset  4
 *   windowID  Uint32     offset  8
 *   state     Uint8      offset 12
 *   repeat    Uint8      offset 13
 *   padding2  Uint8      offset 14
 *   padding3  Uint8      offset 15
 *   keysym    SDL_Keysym offset 16  (16 bytes)
 */
typedef struct {
    SDL2_Uint32 type;
    SDL2_Uint32 timestamp;
    SDL2_Uint32 windowID;
    SDL2_Uint8  state;
    SDL2_Uint8  repeat;
    SDL2_Uint8  padding2;
    SDL2_Uint8  padding3;
    SDL2_Keysym keysym;
} SDL2_KeyboardEvent;

/*
 * SDL_Event union (56 bytes, matching SDL2 ABI).
 * The padding array forces the union to the correct size so that
 * SDL_PollEvent writes into a buffer that is large enough.
 */
typedef union {
    SDL2_Uint32        type;
    SDL2_WindowEvent   window;
    SDL2_KeyboardEvent key;
    uint8_t            padding[56];
} SDL2_Event;

#endif /* HAVE_SDL2_HEADERS */

/* -------------------------------------------------------------------------
 * SDL2 dynamic loading
 * ---------------------------------------------------------------------- */

static void *g_sdl_handle = NULL;

/* All SDL2 functions used by this library, loaded at runtime by load_sdl2() */
static struct {
    int            (*Init)(SDL2_Uint32 flags);
    void           (*Quit)(void);
    const char    *(*GetError)(void);
    SDL2_Window   *(*CreateWindow)(const char *title, int x, int y,
                                   int w, int h, SDL2_Uint32 flags);
    void           (*DestroyWindow)(SDL2_Window *window);
    SDL2_Renderer *(*CreateRenderer)(SDL2_Window *window, int index,
                                     SDL2_Uint32 flags);
    void           (*DestroyRenderer)(SDL2_Renderer *renderer);
    int            (*RenderSetLogicalSize)(SDL2_Renderer *renderer,
                                           int w, int h);
    int            (*RenderSetIntegerScale)(SDL2_Renderer *renderer,
                                            SDL2_bool enable);
    SDL2_Texture  *(*CreateTexture)(SDL2_Renderer *renderer,
                                    SDL2_Uint32 format, int access,
                                    int w, int h);
    void           (*DestroyTexture)(SDL2_Texture *texture);
    int            (*UpdateTexture)(SDL2_Texture *texture,
                                    const SDL2_Rect *rect,
                                    const void *pixels, int pitch);
    int            (*RenderClear)(SDL2_Renderer *renderer);
    int            (*RenderCopy)(SDL2_Renderer *renderer,
                                 SDL2_Texture *texture,
                                 const SDL2_Rect *srcrect,
                                 const SDL2_Rect *dstrect);
    void           (*RenderPresent)(SDL2_Renderer *renderer);
    int            (*PollEvent)(SDL2_Event *event);
} g_sdl;

/*
 * Attempt to open the SDL2 shared library and resolve all needed symbols.
 * Returns 0 on success, -1 on failure (error printed to stderr).
 */
static int load_sdl2(void)
{
    /* Try the versioned SONAME first, then fall back to the unversioned name.
     * Library names are platform-specific:
     *   Linux:  libSDL2-2.0.so.0  (versioned) / libSDL2.so (unversioned)
     *   macOS:  libSDL2-2.0.0.dylib / libSDL2.dylib
     */
    g_sdl_handle = dlopen("libSDL2-2.0.so.0", RTLD_LAZY | RTLD_GLOBAL);
    if (!g_sdl_handle) {
        g_sdl_handle = dlopen("libSDL2.so", RTLD_LAZY | RTLD_GLOBAL);
    }
#if defined(__APPLE__)
    if (!g_sdl_handle) {
        g_sdl_handle = dlopen("libSDL2-2.0.0.dylib", RTLD_LAZY | RTLD_GLOBAL);
    }
    if (!g_sdl_handle) {
        g_sdl_handle = dlopen("libSDL2.dylib", RTLD_LAZY | RTLD_GLOBAL);
    }
#endif
    if (!g_sdl_handle) {
        fprintf(stderr,
                "graphlib: SDL2 shared library not found: %s\n"
                "graphlib: display unavailable; install libsdl2 to enable it\n",
                dlerror());
        return -1;
    }

#define LOAD_SDL2(fn)                                                   \
    do {                                                                \
        *(void **)(&g_sdl.fn) = dlsym(g_sdl_handle, "SDL_" #fn);       \
        if (!g_sdl.fn) {                                                \
            fprintf(stderr,                                             \
                    "graphlib: SDL2 symbol SDL_" #fn " not found\n");  \
            dlclose(g_sdl_handle);                                      \
            g_sdl_handle = NULL;                                        \
            return -1;                                                  \
        }                                                               \
    } while (0)

    LOAD_SDL2(Init);
    LOAD_SDL2(Quit);
    LOAD_SDL2(GetError);
    LOAD_SDL2(CreateWindow);
    LOAD_SDL2(DestroyWindow);
    LOAD_SDL2(CreateRenderer);
    LOAD_SDL2(DestroyRenderer);
    LOAD_SDL2(RenderSetLogicalSize);
    LOAD_SDL2(RenderSetIntegerScale);
    LOAD_SDL2(CreateTexture);
    LOAD_SDL2(DestroyTexture);
    LOAD_SDL2(UpdateTexture);
    LOAD_SDL2(RenderClear);
    LOAD_SDL2(RenderCopy);
    LOAD_SDL2(RenderPresent);
    LOAD_SDL2(PollEvent);

#undef LOAD_SDL2

    return 0;
}

/* -------------------------------------------------------------------------
 * Internal state
 * ---------------------------------------------------------------------- */

static SDL2_Window   *g_window   = NULL;
static SDL2_Renderer *g_renderer = NULL;
static SDL2_Texture  *g_texture  = NULL;

/* Framebuffer: GRAPHLIB_WIDTH * GRAPHLIB_HEIGHT pixels, RGB332 */
static graphlib_pixel_t g_framebuffer[GRAPHLIB_HEIGHT * GRAPHLIB_WIDTH];

/* Optional keyboard callback */
static graphlib_key_callback_t g_key_callback = NULL;

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

int graphlib_init(void)
{
    /* On Linux/Unix systems SDL2's video subsystem requires a display
     * server connection (X11 or Wayland).  Without one, SDL_Init can hang
     * indefinitely waiting for a connection that will never succeed.
     * Detect the missing display early and fail cleanly so that the caller
     * can fall back to a non-graphical output path.
     *
     * Note: this is a heuristic — a set but unreachable DISPLAY will still
     * cause SDL_Init to fail, but that failure is caught by the error check
     * below rather than resulting in a hang. */
#if !defined(__APPLE__) && !defined(_WIN32)
    if (!getenv("DISPLAY") && !getenv("WAYLAND_DISPLAY")) {
        fprintf(stderr,
                "graphlib: no display available "
                "(DISPLAY and WAYLAND_DISPLAY are not set)\n");
        return -1;
    }
#endif

    if (load_sdl2() != 0) {
        return -1;
    }

    if (g_sdl.Init(SDL2_INIT_VIDEO) != 0) {
        fprintf(stderr, "graphlib: SDL_Init failed: %s\n", g_sdl.GetError());
        return -1;
    }

    g_window = g_sdl.CreateWindow(
        "Graphlib Display",
        SDL2_WINDOWPOS_CENTERED, SDL2_WINDOWPOS_CENTERED,
        GRAPHLIB_WIDTH, GRAPHLIB_HEIGHT,
        SDL2_WINDOW_SHOWN | SDL2_WINDOW_RESIZABLE
    );
    if (!g_window) {
        fprintf(stderr, "graphlib: SDL_CreateWindow failed: %s\n",
                g_sdl.GetError());
        g_sdl.Quit();
        return -1;
    }

    g_renderer = g_sdl.CreateRenderer(
        g_window, -1,
        SDL2_RENDERER_ACCELERATED | SDL2_RENDERER_PRESENTVSYNC
    );
    if (!g_renderer) {
        /* Fall back to software renderer */
        g_renderer = g_sdl.CreateRenderer(g_window, -1,
                                           SDL2_RENDERER_SOFTWARE);
    }
    if (!g_renderer) {
        fprintf(stderr, "graphlib: SDL_CreateRenderer failed: %s\n",
                g_sdl.GetError());
        g_sdl.DestroyWindow(g_window);
        g_window = NULL;
        g_sdl.Quit();
        return -1;
    }

    /* Keep the 720x576 logical resolution independent of the window size */
    g_sdl.RenderSetLogicalSize(g_renderer, GRAPHLIB_WIDTH, GRAPHLIB_HEIGHT);

    /* Scale with integer or best-fit, keeping aspect ratio */
    g_sdl.RenderSetIntegerScale(g_renderer, SDL2_FALSE);

    /* Streaming texture in RGB332 format */
    g_texture = g_sdl.CreateTexture(
        g_renderer,
        SDL2_PIXELFORMAT_RGB332,
        SDL2_TEXTUREACCESS_STREAMING,
        GRAPHLIB_WIDTH, GRAPHLIB_HEIGHT
    );
    if (!g_texture) {
        fprintf(stderr, "graphlib: SDL_CreateTexture failed: %s\n",
                g_sdl.GetError());
        g_sdl.DestroyRenderer(g_renderer);
        g_sdl.DestroyWindow(g_window);
        g_renderer = NULL;
        g_window   = NULL;
        g_sdl.Quit();
        return -1;
    }

    /* Clear the framebuffer to black */
    memset(g_framebuffer, 0, sizeof(g_framebuffer));

    return 0;
}

graphlib_pixel_t *graphlib_get_framebuffer(void)
{
    if (!g_window) {
        return NULL;
    }
    return g_framebuffer;
}

void graphlib_update(void)
{
    if (!g_texture || !g_renderer) {
        return;
    }

    /* Upload framebuffer to the streaming texture */
    g_sdl.UpdateTexture(
        g_texture, NULL,
        g_framebuffer,
        GRAPHLIB_WIDTH * (int)sizeof(graphlib_pixel_t)
    );

    g_sdl.RenderClear(g_renderer);
    g_sdl.RenderCopy(g_renderer, g_texture, NULL, NULL);
    g_sdl.RenderPresent(g_renderer);
}

int graphlib_poll_events(void)
{
    /* If graphlib_init() failed (SDL2 not loaded), there are no events to
     * process.  Callers should not reach this path after a failed init, but
     * we return 0 (no quit) as a safe no-op rather than crashing. */
    if (!g_sdl_handle) {
        return 0;
    }

    SDL2_Event event;
    while (g_sdl.PollEvent(&event)) {
        switch (event.type) {
        case SDL2_QUIT:
            return 1;

        case SDL2_WINDOWEVENT:
            if (event.window.event == SDL2_WINDOWEVENT_RESIZED ||
                event.window.event == SDL2_WINDOWEVENT_SIZE_CHANGED) {
                /* Re-render the current framebuffer contents at the new size */
                graphlib_update();
            }
            break;

        case SDL2_KEYDOWN:
            if (g_key_callback) {
                g_key_callback(
                    (int)event.key.keysym.sym,
                    (int)event.key.keysym.scancode,
                    1
                );
            }
            break;

        case SDL2_KEYUP:
            if (g_key_callback) {
                g_key_callback(
                    (int)event.key.keysym.sym,
                    (int)event.key.keysym.scancode,
                    0
                );
            }
            break;

        default:
            break;
        }
    }
    return 0;
}

void graphlib_set_key_callback(graphlib_key_callback_t cb)
{
    g_key_callback = cb;
}

void graphlib_cleanup(void)
{
    if (g_texture) {
        g_sdl.DestroyTexture(g_texture);
        g_texture = NULL;
    }
    if (g_renderer) {
        g_sdl.DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }
    if (g_window) {
        g_sdl.DestroyWindow(g_window);
        g_window = NULL;
    }
    if (g_sdl_handle) {
        g_sdl.Quit();
        dlclose(g_sdl_handle);
        g_sdl_handle = NULL;
    }
    g_key_callback = NULL;
}

void graphlib_delay(unsigned int ms)
{
    struct timespec ts;
    ts.tv_sec  = (time_t)(ms / 1000u);
    ts.tv_nsec = (long)(ms % 1000u) * 1000000L;
    nanosleep(&ts, NULL);
}
