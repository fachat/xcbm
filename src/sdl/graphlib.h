/**
 * graphlib.h - SDL2-based framebuffer display library
 *
 * Provides a 720x576 pixel framebuffer with 8-bit colour depth (RGB332).
 * The caller writes pixel data to the framebuffer and calls graphlib_update()
 * to display it.  The window is resizable and scales content dynamically.
 * Keyboard input is delivered via a registered callback.
 *
 * The library loads SDL2 at runtime via dlopen.  If SDL2 is not installed,
 * graphlib_init() prints a warning to stderr and returns -1 without aborting
 * the process, allowing the caller to fall back to an alternative display.
 */

#ifndef GRAPHLIB_H
#define GRAPHLIB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Framebuffer dimensions */
#define GRAPHLIB_WIDTH  720
#define GRAPHLIB_HEIGHT 576

/**
 * Pixel format: RGB332 (8 bits per pixel)
 *   bits 7-5 : Red   (3 bits)
 *   bits 4-2 : Green (3 bits)
 *   bits 1-0 : Blue  (2 bits)
 */
typedef uint8_t graphlib_pixel_t;

/** Convenience macro to pack an RGB332 pixel */
#define GRAPHLIB_RGB(r, g, b) \
    ((graphlib_pixel_t)(((r) & 0xE0u) | (((g) & 0xE0u) >> 3) | (((b) & 0xC0u) >> 6)))

/**
 * Keyboard callback type.
 *
 * @param keycode  SDL virtual key code (SDL_Keycode)
 * @param scancode SDL physical scan code (SDL_Scancode)
 * @param pressed  1 when the key is pressed, 0 when released
 */
typedef void (*graphlib_key_callback_t)(int keycode, int scancode, int pressed);

/**
 * Commonly used key codes passed to the keyboard callback.
 * These match the corresponding SDL_Keycode values.
 */
/* SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_ESCAPE): scancode 41 | SDLK_SCANCODE_MASK */
#define GRAPHLIB_KEY_ESCAPE  0x40000029  /**< Escape key (= SDLK_ESCAPE) */

/**
 * Initialise the library and open the display window.
 *
 * Must be called before any other graphlib function.
 *
 * @return 0 on success, -1 on failure (error is printed to stderr).
 */
int graphlib_init(void);

/**
 * Return a pointer to the framebuffer.
 *
 * The framebuffer is laid out as GRAPHLIB_HEIGHT rows of GRAPHLIB_WIDTH
 * pixels in row-major order.  Pixel (x, y) is at index y*GRAPHLIB_WIDTH + x.
 *
 * The returned pointer is valid until graphlib_cleanup() is called.
 *
 * @return pointer to the framebuffer, or NULL if the library is not
 *         initialised.
 */
graphlib_pixel_t *graphlib_get_framebuffer(void);

/**
 * Push the current framebuffer contents to the display window.
 *
 * Call this after writing pixel data to make the changes visible.
 */
void graphlib_update(void);

/**
 * Process pending window and keyboard events.
 *
 * Must be called regularly (e.g. once per frame) from the application's
 * main loop.  Keyboard events are forwarded to the registered callback.
 *
 * @return 0 to continue, 1 when the user has requested to quit (close
 *         button or Alt+F4).
 */
int graphlib_poll_events(void);

/**
 * Register a callback to receive keyboard events.
 *
 * Pass NULL to unregister the current callback.
 *
 * @param cb  callback function, or NULL.
 */
void graphlib_set_key_callback(graphlib_key_callback_t cb);

/**
 * Clean up all resources and close the display window.
 *
 * After this call the framebuffer pointer obtained via
 * graphlib_get_framebuffer() is no longer valid.
 */
void graphlib_cleanup(void);

/**
 * Sleep for the specified number of milliseconds.
 *
 * Provides a portable delay without requiring the caller to depend on
 * SDL2 directly.
 *
 * @param ms  Duration in milliseconds.
 */
void graphlib_delay(unsigned int ms);

#ifdef __cplusplus
}
#endif

#endif /* GRAPHLIB_H */
