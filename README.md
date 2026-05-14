# amalgame-ui-sdl

SDL2/SDL3 binding for [Amalgame](https://github.com/amalgame-lang/Amalgame).
Exposes a thin `Window` / `Surface` / `Event` / `Color` / `Rect`
surface that mirrors SDL's event loop, plus a runtime helper to
detect the OS color scheme.

Companion package: [`amalgame-ui-forms`](https://github.com/amalgame-lang/amalgame-ui-forms)
ships the retained-mode Forms toolkit on top of this binding.

> **Status: v0.0.1-dev — work in progress.** Public API is being designed.

## Install

```bash
amc package add github.com/amalgame-lang/amalgame-ui-sdl@v0.0.1-dev
```

Requires **amc 0.8.0+** and SDL2 development headers on the host
(`libsdl2-dev` on Debian/Ubuntu, `sdl2 sdl2_ttf` on Homebrew,
`mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf` on MSYS2).

## Backend selection

Default is SDL2 (universally packaged, LTS). Pass
`-DAMALGAME_UI_USE_SDL3` to the underlying gcc invocation to switch
to SDL3 (the binding header `#ifdef`-switches the headers and
function signatures that differ between the two majors).

## Linking strategy

Both static and dynamic linking are supported:

- **`--sdl-dynamic`** (default) — links against the system's
  `libSDL2.so` / `SDL2.dll` / `libSDL2.dylib`. Smallest binary,
  end users need SDL2 installed at runtime.
- **`--sdl-static`** — links a vendored or system-static
  `libSDL2.a`. Single-file portable binary, larger output.

(Wiring of these flags into `amc build` lands in v0.0.2.)

## Surface (planned for v0.1.0)

```amalgame
import Amalgame.UI.SDL

class Program {
    public static void Main() {
        let win: Window = Window.Create("Hello", 640, 480)
        while (win.IsOpen()) {
            let ev: Event = win.PollEvent()
            if (ev.Kind() == EventKind.Quit) { win.Close() }
        }
    }
}
```

## Scope (v0.1.0 target)

- `Window` — create / show / close / size / title
- `Event` — Quit, MouseDown/Up/Move, KeyDown/Up, Resize, TextInput
- `Surface` — drawing primitives (Clear, FillRect, DrawLine, DrawText)
- `Color` — RGBA
- `Rect` — X/Y/W/H + intersect / contains
- `Theme.DetectOS()` — returns `Light` or `Dark` from the host's
  appearance setting (macOS `AppleInterfaceStyle`, Windows registry
  `AppsUseLightTheme`, Linux `gsettings color-scheme`)

## Tests

```bash
./tests/run_tests.sh /path/to/amc
```

## License

Apache-2.0 — see `LICENSE`. SDL2 is shipped under the Zlib license
(linked, not vendored). See `NOTICE.md`.
