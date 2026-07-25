/*********************************************************************
 *
 *  owndraw.h - Owner-Drawn Listbox API
 *
 *  Public header for the Owner Drawn Listbox library (owndraw.dll).
 *  Include this file in any application that uses the column-aligned
 *  listbox functions.
 *
 *  Usage overview:
 *
 *    1. Create a listbox with the LS_OWNERDRAW style.
 *
 *    2. In your window procedure, handle WM_MEASUREITEM by calling
 *       ODInitLBWidthHeight.  Store the returned lCharWidth value; you
 *       will need it for every WM_DRAWITEM call.
 *
 *    3. Handle WM_DRAWITEM by calling ODDrawLBItems, passing the same
 *       lCharWidth value and your tab-stop array.
 *
 *    4. When inserting items into the listbox, separate columns with
 *       tab characters ('\t') or pre-pad strings to the column widths.
 *
 *  Copyright (c) 1992 ASH Software, Inc.
 *  Updates (c) 2026 Martin Iturbide / OS2World
 *
 *  Distributed under the BSD 3-Clause License.
 *
 *********************************************************************/

#ifndef OWNDRAW_H
#define OWNDRAW_H

/* -----------------------------------------------------------------------
 * Constants
 * ----------------------------------------------------------------------- */

/* sfCharWidth values for ODInitLBWidthHeight */
#define AVERAGE_WIDTH   0   /* Use lAveCharWidth from FONTMETRICS  */
#define MAXIMUM_WIDTH   1   /* Use lMaxCharInc from FONTMETRICS    */

/* Maximum number of characters in a single listbox item string.
 * Strings longer than this are silently truncated during drawing. */
#define MAX_TEXT_CHARS  255

/* -----------------------------------------------------------------------
 * Function prototypes
 * ----------------------------------------------------------------------- */

/*
 * ODInitLBWidthHeight - initialize column width/height for an owner-draw
 * listbox.  Call this from your WM_MEASUREITEM handler and return the
 * value it returns.
 *
 *   hListBox           - handle to the listbox window
 *   sfCharWidth        - AVERAGE_WIDTH or MAXIMUM_WIDTH
 *   sMaxNumberOfChars  - longest string (in characters) that will be inserted
 *   lCharWidth         - receives the per-character pixel width used for
 *                        column calculations; pass this to ODDrawLBItems
 *
 *   Returns MRESULT: HIUSHORT = maximum baseline extent (row height),
 *                    LOUSHORT = total item width in pixels
 */
MRESULT EXPENTRY ODInitLBWidthHeight(HWND   hListBox,
                                     SHORT  sfCharWidth,
                                     SHORT  sMaxNumberOfChars,
                                     LONG  *lCharWidth);

/*
 * ODDrawLBItems - draw one owner-draw listbox item with column alignment.
 * Call this from your WM_DRAWITEM handler and return the value it returns.
 *
 *   pOwnerItem  - pointer to the OWNERITEM structure passed in mp2
 *   lCharWidth  - per-character pixel width from ODInitLBWidthHeight
 *   psTabStops  - array of 1-based character-column numbers defining each
 *                 column boundary; the array must be terminated with 0
 *
 *   Returns (MRESULT)TRUE to indicate the item has been drawn.
 *
 * Notes:
 *   - Tab stop column numbers are 1-based (column 1 is the first character).
 *   - Items may embed '\t' characters to advance to the next tab stop, or
 *     may be pre-padded with spaces.
 *   - The tab-stop array and lCharWidth must remain valid for the lifetime
 *     of the listbox.  Declare them static or at file scope.
 *   - Highlighting is left to the default system behaviour.
 */
MRESULT EXPENTRY ODDrawLBItems(POWNERITEM pOwnerItem,
                               LONG       lCharWidth,
                               SHORT     *psTabStops);

#endif /* OWNDRAW_H */
