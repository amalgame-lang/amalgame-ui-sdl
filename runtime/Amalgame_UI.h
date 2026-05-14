/*
 * Amalgame Standard Library — Amalgame.UI.SDL
 * Copyright (c) 2026 Bastien MOUGET
 * https://github.com/amalgame-lang/Amalgame
 *
 * Thin C binding over SDL2 (default) or SDL3 (when
 * AMALGAME_UI_USE_SDL3 is defined). Provides the helpers that
 * `facade.am`'s `@c { ... }` blocks call through:
 *
 *   - Amalgame_UI_Init / Amalgame_UI_Quit
 *   - Amalgame_UI_CreateWindow / DestroyWindow / ShowWindow
 *   - Amalgame_UI_PollEvent      (returns 0 if no event pending)
 *   - Amalgame_UI_WaitEvent      (blocking; used by Application.Run)
 *   - Amalgame_UI_GetRenderer    (SDL_Renderer* per window)
 *   - Amalgame_UI_Clear / FillRect / DrawLine / DrawText
 *   - Amalgame_UI_LoadFont / DestroyFont
 *   - Amalgame_UI_DetectOSTheme  (returns "light" / "dark")
 *
 * Status: v0.0.1-dev — header is currently a stub. Concrete
 * helpers land incrementally per the project roadmap. Defining
 * the symbols early (even if no-ops) lets `facade.am` reference
 * them stably while we iterate.
 *
 * Linking:
 *   - SDL2 path: -lSDL2 -lSDL2_ttf
 *   - SDL3 path: -lSDL3 -lSDL3_ttf
 *   Choice driven by `amalgame.toml`'s `[stdlib].libs` and the
 *   compile-time AMALGAME_UI_USE_SDL3 macro.
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

/* ─── Lifecycle ────────────────────────────────────────── */

static inline int Amalgame_UI_Init(void) {
    /* TODO: error reporting via _runtime.h conventions */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 0;
    if (TTF_Init() != 0) { SDL_Quit(); return 0; }
    return 1;
}

static inline void Amalgame_UI_Quit(void) {
    TTF_Quit();
    SDL_Quit();
}

/* ─── OS theme detection ───────────────────────────────── */

/* Returns "light" or "dark". Falls back to "light" on detection
 * failure. v0.0.1-dev: stub returns "light" unconditionally;
 * concrete probes (defaults / registry / gsettings) land in a
 * follow-up. */
static inline const char* Amalgame_UI_DetectOSTheme(void) {
    /* TODO platform probes:
     *   macOS:   defaults read -g AppleInterfaceStyle
     *   Windows: HKCU\…\Themes\Personalize\AppsUseLightTheme
     *   Linux:   gsettings get org.gnome.desktop.interface color-scheme
     *            or freedesktop appearance portal
     */
    return "light";
}

/* ─── Window / Renderer / Event / Drawing ──────────────── */
/* Helpers land in v0.0.2+. The struct typedefs below give
 * `facade.am` stable types to reference today. */

typedef SDL_Window*   Amalgame_UI_WindowHandle;
typedef SDL_Renderer* Amalgame_UI_RendererHandle;
typedef TTF_Font*     Amalgame_UI_FontHandle;

#endif /* AMALGAME_UI_H */
