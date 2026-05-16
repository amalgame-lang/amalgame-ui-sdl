# amalgame-ui-sdl

SDL2/SDL3 binding for [Amalgame](https://github.com/amalgame-lang/Amalgame).
Exposes a thin `Window` / `Surface` / `Event` / `Color` / `Rect`
surface that mirrors SDL's event loop, plus a runtime helper to
detect the OS color scheme.

Companion package: [`amalgame-ui-forms`](https://github.com/amalgame-lang/amalgame-ui-forms)
ships the retained-mode Forms toolkit on top of this binding.

> **Status: v0.0.1-dev — work in progress.** Public API is being designed.

## Prerequisites

SDL2 + SDL2_ttf development headers at **build time**, plus the
matching runtime libraries on the **deploy** target.

### Build time (`amc package add` machine)

| OS / distro | Command |
|---|---|
| Debian / Ubuntu | `sudo apt install libsdl2-dev libsdl2-ttf-dev` |
| Fedora / RHEL | `sudo dnf install SDL2-devel SDL2_ttf-devel` |
| Arch / Manjaro | `sudo pacman -S sdl2 sdl2_ttf` |
| Alpine | `apk add sdl2-dev sdl2_ttf-dev` |
| macOS (Homebrew) | `brew install sdl2 sdl2_ttf` |
| Windows (MSYS2) | `pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf` |

### Deploy time (target running the user binary)

| OS / distro | Command |
|---|---|
| Debian / Ubuntu | `apt install libsdl2-2.0-0 libsdl2-ttf-2.0-0` |
| Fedora / RHEL | usually pulled in as a dep of `SDL2-devel`; else `dnf install SDL2 SDL2_ttf` |
| macOS | bundled with Homebrew install above (same runtime) |
| Windows | ship `SDL2.dll` + `SDL2_ttf.dll` alongside the `.exe`, or use the static-link variant — see below |

### Optional: SDL3

Pass `-DAMALGAME_UI_USE_SDL3` to switch from the default SDL2 to
SDL3 (the binding header `#ifdef`-switches the function signatures
that differ). SDL3 packages: `libsdl3-dev` (apt), `sdl3` (brew /
pacman / pkg), `SDL3-devel` (dnf).

## Install

```bash
amc package add github.com/amalgame-lang/amalgame-ui-sdl@v0.0.1-dev
```

Requires **amc 0.8.0+**.

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
