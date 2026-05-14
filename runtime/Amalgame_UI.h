/*
 * Amalgame Standard Library — Amalgame.UI.SDL
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Thin C binding over SDL2 (default) or SDL3 (opt-in via
 * -DAMALGAME_UI_USE_SDL3). Exposes Window / Event / Surface
 * primitives plus an OS-theme detector through a stable
 * `Amalgame_UI_*` C API that the facade.am layer wraps as
 * idiomatic Amalgame classes.
 *
 * v0.0.2-dev surface:
 *   - Amalgame_UI_Init / Quit
 *   - Amalgame_UI_Window_new / close / should_close /
 *     width / height / set_title
 *   - Amalgame_UI_pollEvent / waitEvent (flat
 *     AmalgameUIEvent struct out-param — no nested unions)
 *   - Amalgame_UI_clear / present / fillRect / drawRect /
 *     drawLine / drawPixel
 *   - Amalgame_UI_loadFont / destroyFont / drawText /
 *     measureText
 *   - Amalgame_UI_DetectOSTheme — returns "light" or "dark"
 *     via OS-native probes, cached after first call
 *
 * Linking:
 *   - SDL2 path: -lSDL2 -lSDL2_ttf
 *   - SDL3 path: -lSDL3 -lSDL3_ttf
 */

#ifndef AMALGAME_UI_H
#define AMALGAME_UI_H

#include "_runtime.h"

#ifdef AMALGAME_UI_USE_SDL3
  #include <SDL3/SDL.h>
  #include <SDL3_ttf/SDL_ttf.h>
#else
  #include <SDL2/SDL.h>
  #include <SDL2/SDL_ttf.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
  #include <windows.h>
#endif

/* ─── Lifecycle ────────────────────────────────────────── */

static int _amalgame_ui_inited = 0;

static inline int Amalgame_UI_Init(void) {
    if (_amalgame_ui_inited) return 1;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 0;
    if (TTF_Init() != 0) { SDL_Quit(); return 0; }
    _amalgame_ui_inited = 1;
    return 1;
}

static inline void Amalgame_UI_Quit(void) {
    if (!_amalgame_ui_inited) return;
    TTF_Quit();
    SDL_Quit();
    _amalgame_ui_inited = 0;
}

/* ─── Window struct + ops ──────────────────────────────── */

typedef struct AmalgameUIWindow {
    SDL_Window*   sdl_window;
    SDL_Renderer* sdl_renderer;
    int           width;
    int           height;
    int           should_close;
} AmalgameUIWindow;

static inline AmalgameUIWindow* Amalgame_UI_Window_new(code_string title, int width, int height) {
    if (!Amalgame_UI_Init()) return NULL;
    AmalgameUIWindow* w = (AmalgameUIWindow*) GC_MALLOC(sizeof(AmalgameUIWindow));
    if (!w) return NULL;
    w->width        = width;
    w->height       = height;
    w->should_close = 0;
    const char* t = title ? (const char*)title : "Amalgame";
#ifdef AMALGAME_UI_USE_SDL3
    w->sdl_window = SDL_CreateWindow(t, width, height, SDL_WINDOW_RESIZABLE);
    if (!w->sdl_window) return NULL;
    w->sdl_renderer = SDL_CreateRenderer(w->sdl_window, NULL);
#else
    w->sdl_window = SDL_CreateWindow(t,
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     width, height,
                                     SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!w->sdl_window) return NULL;
    w->sdl_renderer = SDL_CreateRenderer(w->sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (!w->sdl_renderer) {
        /* Fall back to software renderer (e.g. when SDL_VIDEODRIVER=dummy). */
        w->sdl_renderer = SDL_CreateRenderer(w->sdl_window, -1, SDL_RENDERER_SOFTWARE);
    }
#endif
    return w;
}

static inline void Amalgame_UI_Window_close(AmalgameUIWindow* w) {
    if (!w) return;
    if (w->sdl_renderer) { SDL_DestroyRenderer(w->sdl_renderer); w->sdl_renderer = NULL; }
    if (w->sdl_window)   { SDL_DestroyWindow(w->sdl_window);     w->sdl_window   = NULL; }
    w->should_close = 1;
}

static inline int Amalgame_UI_Window_should_close(AmalgameUIWindow* w) {
    return w ? w->should_close : 1;
}

static inline int Amalgame_UI_Window_width(AmalgameUIWindow* w)  { return w ? w->width  : 0; }
static inline int Amalgame_UI_Window_height(AmalgameUIWindow* w) { return w ? w->height : 0; }

static inline void Amalgame_UI_Window_set_title(AmalgameUIWindow* w, code_string title) {
    if (!w || !w->sdl_window) return;
    SDL_SetWindowTitle(w->sdl_window, title ? (const char*)title : "Amalgame");
}

/* ─── Event ────────────────────────────────────────────── */

/* Flat payload — the facade decodes `kind` into an Amalgame enum
 * and reads only the fields relevant for that kind. */
typedef struct AmalgameUIEvent {
    int kind;     /* 0=none, 1=quit, 2=mouse_down, 3=mouse_up,
                   * 4=mouse_move, 5=key_down, 6=key_up,
                   * 7=text_input, 8=window_resize */
    int x;
    int y;
    int button;   /* mouse button id or SDL keycode */
    int mods;     /* keyboard modifier bitmask */
    int w;        /* resize width  (kind=8) */
    int h;        /* resize height (kind=8) */
} AmalgameUIEvent;

static inline void _amalgame_ui_zero_event(AmalgameUIEvent* out) {
    out->kind = 0; out->x = 0; out->y = 0;
    out->button = 0; out->mods = 0; out->w = 0; out->h = 0;
}

static inline int _amalgame_ui_translate_event(AmalgameUIWindow* w, const SDL_Event* ev,
                                               AmalgameUIEvent* out) {
    _amalgame_ui_zero_event(out);
    switch (ev->type) {
        case SDL_QUIT:
            out->kind = 1;
            if (w) w->should_close = 1;
            return 1;
        case SDL_MOUSEBUTTONDOWN:
            out->kind = 2;
            out->x = ev->button.x; out->y = ev->button.y;
            out->button = ev->button.button;
            return 1;
        case SDL_MOUSEBUTTONUP:
            out->kind = 3;
            out->x = ev->button.x; out->y = ev->button.y;
            out->button = ev->button.button;
            return 1;
        case SDL_MOUSEMOTION:
            out->kind = 4;
            out->x = ev->motion.x; out->y = ev->motion.y;
            return 1;
        case SDL_KEYDOWN:
            out->kind = 5;
#ifdef AMALGAME_UI_USE_SDL3
            out->button = (int)ev->key.key;
            out->mods   = (int)ev->key.mod;
#else
            out->button = (int)ev->key.keysym.sym;
            out->mods   = (int)ev->key.keysym.mod;
#endif
            return 1;
        case SDL_KEYUP:
            out->kind = 6;
#ifdef AMALGAME_UI_USE_SDL3
            out->button = (int)ev->key.key;
            out->mods   = (int)ev->key.mod;
#else
            out->button = (int)ev->key.keysym.sym;
            out->mods   = (int)ev->key.keysym.mod;
#endif
            return 1;
#ifdef AMALGAME_UI_USE_SDL3
        case SDL_EVENT_WINDOW_RESIZED:
            out->kind = 8;
            out->w = ev->window.data1; out->h = ev->window.data2;
            if (w) { w->width = out->w; w->height = out->h; }
            return 1;
#else
        case SDL_WINDOWEVENT:
            if (ev->window.event == SDL_WINDOWEVENT_RESIZED ||
                ev->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                out->kind = 8;
                out->w = ev->window.data1; out->h = ev->window.data2;
                if (w) { w->width = out->w; w->height = out->h; }
                return 1;
            }
            return 0;
#endif
        default:
            return 0;
    }
}

/* Non-blocking. Returns 1 if `out` was populated with a real
 * event, 0 if the queue was empty (out->kind=0). */
static inline int Amalgame_UI_pollEvent(AmalgameUIWindow* w, AmalgameUIEvent* out) {
    if (!out) return 0;
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (_amalgame_ui_translate_event(w, &ev, out)) return 1;
    }
    _amalgame_ui_zero_event(out);
    return 0;
}

/* Blocking. Waits until at least one translatable event arrives.
 * Used by `Application.Run()` so the CPU stays idle between
 * interactions. */
static inline int Amalgame_UI_waitEvent(AmalgameUIWindow* w, AmalgameUIEvent* out) {
    if (!out) return 0;
    SDL_Event ev;
    while (SDL_WaitEvent(&ev)) {
        if (_amalgame_ui_translate_event(w, &ev, out)) return 1;
    }
    _amalgame_ui_zero_event(out);
    return 0;
}

/* ─── Drawing primitives ───────────────────────────────── */

static inline void Amalgame_UI_clear(AmalgameUIWindow* w, int r, int g, int b) {
    if (!w || !w->sdl_renderer) return;
    SDL_SetRenderDrawColor(w->sdl_renderer, (Uint8)r, (Uint8)g, (Uint8)b, 255);
    SDL_RenderClear(w->sdl_renderer);
}

static inline void Amalgame_UI_present(AmalgameUIWindow* w) {
    if (!w || !w->sdl_renderer) return;
    SDL_RenderPresent(w->sdl_renderer);
}

static inline void Amalgame_UI_fillRect(AmalgameUIWindow* w,
                                        int x, int y, int width, int height,
                                        int r, int g, int b, int a) {
    if (!w || !w->sdl_renderer) return;
    SDL_SetRenderDrawColor(w->sdl_renderer, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a);
    SDL_SetRenderDrawBlendMode(w->sdl_renderer, SDL_BLENDMODE_BLEND);
#ifdef AMALGAME_UI_USE_SDL3
    SDL_FRect rect = { (float)x, (float)y, (float)width, (float)height };
    SDL_RenderFillRect(w->sdl_renderer, &rect);
#else
    SDL_Rect rect = { x, y, width, height };
    SDL_RenderFillRect(w->sdl_renderer, &rect);
#endif
}

static inline void Amalgame_UI_drawRect(AmalgameUIWindow* w,
                                        int x, int y, int width, int height,
                                        int r, int g, int b, int a) {
    if (!w || !w->sdl_renderer) return;
    SDL_SetRenderDrawColor(w->sdl_renderer, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a);
    SDL_SetRenderDrawBlendMode(w->sdl_renderer, SDL_BLENDMODE_BLEND);
#ifdef AMALGAME_UI_USE_SDL3
    SDL_FRect rect = { (float)x, (float)y, (float)width, (float)height };
    SDL_RenderRect(w->sdl_renderer, &rect);
#else
    SDL_Rect rect = { x, y, width, height };
    SDL_RenderDrawRect(w->sdl_renderer, &rect);
#endif
}

static inline void Amalgame_UI_drawLine(AmalgameUIWindow* w,
                                        int x1, int y1, int x2, int y2,
                                        int r, int g, int b, int a) {
    if (!w || !w->sdl_renderer) return;
    SDL_SetRenderDrawColor(w->sdl_renderer, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a);
    SDL_SetRenderDrawBlendMode(w->sdl_renderer, SDL_BLENDMODE_BLEND);
#ifdef AMALGAME_UI_USE_SDL3
    SDL_RenderLine(w->sdl_renderer, (float)x1, (float)y1, (float)x2, (float)y2);
#else
    SDL_RenderDrawLine(w->sdl_renderer, x1, y1, x2, y2);
#endif
}

static inline void Amalgame_UI_drawPixel(AmalgameUIWindow* w, int x, int y,
                                         int r, int g, int b, int a) {
    if (!w || !w->sdl_renderer) return;
    SDL_SetRenderDrawColor(w->sdl_renderer, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a);
#ifdef AMALGAME_UI_USE_SDL3
    SDL_RenderPoint(w->sdl_renderer, (float)x, (float)y);
#else
    SDL_RenderDrawPoint(w->sdl_renderer, x, y);
#endif
}

/* ─── Fonts + text ─────────────────────────────────────── */

typedef struct AmalgameUIFont {
    TTF_Font* font;
    int       size;
} AmalgameUIFont;

static inline AmalgameUIFont* Amalgame_UI_loadFont(code_string path, int size) {
    if (!Amalgame_UI_Init()) return NULL;
    if (!path) return NULL;
    TTF_Font* f = TTF_OpenFont((const char*)path, size);
    if (!f) return NULL;
    AmalgameUIFont* af = (AmalgameUIFont*) GC_MALLOC(sizeof(AmalgameUIFont));
    if (!af) { TTF_CloseFont(f); return NULL; }
    af->font = f;
    af->size = size;
    return af;
}

static inline void Amalgame_UI_destroyFont(AmalgameUIFont* f) {
    if (!f) return;
    if (f->font) TTF_CloseFont(f->font);
    f->font = NULL;
}

static inline void Amalgame_UI_drawText(AmalgameUIWindow* w, AmalgameUIFont* f,
                                        code_string text, int x, int y,
                                        int r, int g, int b, int a) {
    if (!w || !w->sdl_renderer || !f || !f->font || !text) return;
    SDL_Color col = { (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a };
#ifdef AMALGAME_UI_USE_SDL3
    SDL_Surface* surf = TTF_RenderText_Blended(f->font, (const char*)text, 0, col);
#else
    SDL_Surface* surf = TTF_RenderUTF8_Blended(f->font, (const char*)text, col);
#endif
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(w->sdl_renderer, surf);
    if (tex) {
#ifdef AMALGAME_UI_USE_SDL3
        SDL_FRect dst = { (float)x, (float)y, (float)surf->w, (float)surf->h };
        SDL_RenderTexture(w->sdl_renderer, tex, NULL, &dst);
#else
        SDL_Rect dst = { x, y, surf->w, surf->h };
        SDL_RenderCopy(w->sdl_renderer, tex, NULL, &dst);
#endif
        SDL_DestroyTexture(tex);
    }
#ifdef AMALGAME_UI_USE_SDL3
    SDL_DestroySurface(surf);
#else
    SDL_FreeSurface(surf);
#endif
}

/* Writes the rendered width/height for `text` at `f` into the
 * out-params. Either pointer may be NULL. Returns 1 on success. */
static inline int Amalgame_UI_measureText(AmalgameUIFont* f, code_string text,
                                          int* outW, int* outH) {
    if (!f || !f->font || !text) {
        if (outW) *outW = 0; if (outH) *outH = 0;
        return 0;
    }
    int w = 0, h = 0;
#ifdef AMALGAME_UI_USE_SDL3
    size_t len = strlen((const char*)text);
    if (TTF_GetStringSize(f->font, (const char*)text, len, &w, &h) < 0) return 0;
#else
    if (TTF_SizeUTF8(f->font, (const char*)text, &w, &h) != 0) return 0;
#endif
    if (outW) *outW = w;
    if (outH) *outH = h;
    return 1;
}

/* ─── OS theme detection ───────────────────────────────── *
 * Returns "light" or "dark". Falls back to "light" on probe
 * failure. Cached after first call.                          */

static int _amalgame_ui_theme_cached = -1;  /* -1=unknown, 0=light, 1=dark */

static inline code_string Amalgame_UI_DetectOSTheme(void) {
    if (_amalgame_ui_theme_cached >= 0) {
        return _amalgame_ui_theme_cached == 1 ? (code_string)"dark" : (code_string)"light";
    }
#if defined(__APPLE__)
    FILE* p = popen("defaults read -g AppleInterfaceStyle 2>/dev/null", "r");
    if (p) {
        char buf[16] = {0};
        size_t n = fread(buf, 1, sizeof(buf)-1, p);
        pclose(p);
        _amalgame_ui_theme_cached = (n > 0 && buf[0] == 'D') ? 1 : 0;
    } else {
        _amalgame_ui_theme_cached = 0;
    }
#elif defined(_WIN32)
    HKEY key;
    DWORD val = 1, sz = sizeof(val), type = REG_DWORD;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &key) == ERROR_SUCCESS) {
        RegQueryValueExA(key, "AppsUseLightTheme", NULL, &type, (LPBYTE)&val, &sz);
        RegCloseKey(key);
        _amalgame_ui_theme_cached = (val == 0) ? 1 : 0;
    } else {
        _amalgame_ui_theme_cached = 0;
    }
#else
    FILE* p = popen("gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null", "r");
    if (p) {
        char buf[64] = {0};
        size_t n = fread(buf, 1, sizeof(buf)-1, p);
        pclose(p);
        if (n > 0 && strstr(buf, "dark")) {
            _amalgame_ui_theme_cached = 1;
        } else if (n > 0) {
            _amalgame_ui_theme_cached = 0;
        } else {
            /* gsettings unavailable or no DE — fall back to env vars
             * (KDE / freedesktop appearance portals set these). */
            const char* env = getenv("GTK_THEME");
            if (env && strstr(env, "dark")) _amalgame_ui_theme_cached = 1;
            else _amalgame_ui_theme_cached = 0;
        }
    } else {
        _amalgame_ui_theme_cached = 0;
    }
#endif
    return _amalgame_ui_theme_cached == 1 ? (code_string)"dark" : (code_string)"light";
}

/* ─── Convenience handle typedefs ──────────────────────── *
 * Kept for backwards compatibility with v0.0.1-dev facade
 * stubs that referenced them. The Amalgame facade carries
 * AmalgameUIWindow* and AmalgameUIFont* directly as i64
 * handles. */

typedef SDL_Window*   Amalgame_UI_WindowHandle;
typedef SDL_Renderer* Amalgame_UI_RendererHandle;
typedef TTF_Font*     Amalgame_UI_FontHandle;

#endif /* AMALGAME_UI_H */
