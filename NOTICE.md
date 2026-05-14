# NOTICE — amalgame-ui-sdl

## Authorship

Copyright 2026 Bastien Mouget. The Amalgame binding code in this
repository is original work — see `runtime/Amalgame_UI.h`, the
`facade.am` facade, and the `amalgame.toml` manifest.

This package is part of the Amalgame ecosystem
([github.com/amalgame-lang/Amalgame](https://github.com/amalgame-lang/Amalgame)).
It backs the planned `amc new --template forms` scaffolder and the
companion `amalgame-ui-forms` widgets package.

## License

Licensed under the Apache License, Version 2.0 — see `LICENSE`.

## Third-party content

This package links against (but does not vendor) the following
third-party libraries at build time:

- **SDL2** — Simple DirectMedia Layer 2.x, by Sam Lantinga and
  contributors. Distributed under the Zlib license
  (https://www.libsdl.org/). End users obtain SDL2 from their
  platform package manager (`apt`, `brew`, MSYS2) or by vendoring
  a static build alongside their Amalgame project.

- **SDL2_ttf** — TrueType-font rendering extension for SDL2,
  distributed under the Zlib license. Same provenance as SDL2.

- **SDL3** *(optional)* — Simple DirectMedia Layer 3.x, Zlib
  license. Used when the compile-time flag `AMALGAME_UI_USE_SDL3`
  is set.

The runtime header `runtime/Amalgame_UI.h` contains only thin
C helpers that adapt SDL's API surface to Amalgame's value/struct
conventions — no SDL source code is copied into this repository.
