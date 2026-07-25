# DEV-SAMPLES-C-LIB-PM-Owner_Drawn_Listbox

An OS/2 Presentation Manager library that enables **column-aligned text** in listboxes when using a proportional-spaced font.

The library (`owndraw.dll`) provides two functions that integrate with PM's owner-draw listbox mechanism. Applications link against `owndraw.lib` at build time and distribute `owndraw.dll` at run time.

## Background

Standard OS/2 PM listboxes draw text with the system's default font alignment, which means columns in proportional fonts will not line up across rows. The `LS_OWNERDRAW` listbox style lets an application take full control of item rendering. This library implements that rendering: it queries the font metrics once (at `WM_MEASUREITEM` time) and uses the resulting character width to map logical column numbers to exact pixel offsets at `WM_DRAWITEM` time.

Originally written in 1992 by ASH Software, Inc.

## API

### `ODInitLBWidthHeight`

Call from your `WM_MEASUREITEM` handler. Queries the listbox font metrics and computes item dimensions.

```c
MRESULT EXPENTRY ODInitLBWidthHeight(HWND   hListBox,
                                     SHORT  sfCharWidth,
                                     SHORT  sMaxNumberOfChars,
                                     LONG  *lCharWidth);
```

| Parameter           | Description |
|---------------------|-------------|
| `hListBox`          | Handle to the owner-draw listbox |
| `sfCharWidth`       | `AVERAGE_WIDTH` or `MAXIMUM_WIDTH` |
| `sMaxNumberOfChars` | Longest item string in characters |
| `lCharWidth`        | OUT: per-character pixel width for column math |

Returns a `MRESULT` whose high word is the row height and low word is the total item width. Return this value directly from `WM_MEASUREITEM`.

### `ODDrawLBItems`

Call from your `WM_DRAWITEM` handler. Draws one listbox item with columns aligned to pixel boundaries.

```c
MRESULT EXPENTRY ODDrawLBItems(POWNERITEM pOwnerItem,
                               LONG       lCharWidth,
                               SHORT     *psTabStops);
```

| Parameter     | Description |
|---------------|-------------|
| `pOwnerItem`  | `OWNERITEM` pointer from `mp2` |
| `lCharWidth`  | Value obtained from `ODInitLBWidthHeight` |
| `psTabStops`  | Zero-terminated array of 1-based column positions, e.g. `{10, 20, 35, 0}` |

Returns `(MRESULT)TRUE`. Return this value directly from `WM_DRAWITEM`.

## Usage example

```c
#include "owndraw.h"

static LONG  lCharWidth;
static SHORT asTabStops[] = { 10, 25, 40, 0 };  /* must be static/global */

MRESULT EXPENTRY MyWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    switch (msg)
    {
        case WM_MEASUREITEM:
            return ODInitLBWidthHeight(
                       WinWindowFromID(hwnd, ID_LISTBOX),
                       AVERAGE_WIDTH, 40, &lCharWidth);

        case WM_DRAWITEM:
            return ODDrawLBItems((POWNERITEM)mp2, lCharWidth, asTabStops);
    }
    return WinDefWindowProc(hwnd, msg, mp1, mp2);
}
```

Items inserted into the listbox can use `'\t'` to advance to the next tab stop:

```c
WinSendMsg(hwndList, LM_INSERTITEM,
           MPFROMSHORT(LIT_END),
           MPFROMP("FileName.txt\t42 KB\t2026-07-24"));
```

## Directory structure

```
src/            Source files (owndraw.c, owndraw.h, owndraw.def)
bin-gcc/        GCC build output (created by compile-gcc.cmd)
bin-ow/         OpenWatcom build output (created by compile-ow.cmd)
makefile-gcc    GNU make rules for GCC
makefile-ow     wmake rules for OpenWatcom
compile-gcc.cmd Build script for GCC
compile-ow.cmd  Build script for OpenWatcom
```

## Building

### GCC 9.2 (bitwiseworks)

Requires GCC 9.2 for OS/2, `emximp`, and GNU `make`.

```
compile-gcc.cmd
```

Output: `bin-gcc\owndraw.dll`, `bin-gcc\owndraw.lib`

### OpenWatcom 2.0

Requires `wcc386` and `wlink`.

```
compile-ow.cmd
```

Output: `bin-ow\owndraw.dll`, `bin-ow\owndraw.lib`

## Version

Current version: **1.1**

The bldlevel string is embedded in `src/owndraw.def` and reports at run time via:
```
bldlevel owndraw.dll
```

## License

BSD 3-Clause — see [LICENSE](LICENSE).

## Authors

- ASH Software, Inc. (original, 1992)
- Martin Iturbide / OS2World (2026 — build system, documentation)

## Links

- https://github.com/OS2World/DEV-SAMPLES-C-LIB-PM-Owner_Drawn_Listbox
- https://os2world.net
