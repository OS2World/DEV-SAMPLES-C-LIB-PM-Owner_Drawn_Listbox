/*********************************************************************
 *
 *  owndraw.c - Owner-Drawn Listbox implementation
 *
 *  Implements two functions that extend the OS/2 PM listbox control to
 *  align text in fixed columns when using a proportional-spaced font.
 *  The pair is intended to be built as a DLL (owndraw.dll) and used by
 *  applications that require tabular data in listboxes.
 *
 *  How it works:
 *
 *    OS/2 PM listboxes with the LS_OWNERDRAW style send WM_MEASUREITEM
 *    once per item and WM_DRAWITEM each time an item needs to be painted.
 *    These two messages are handled by ODInitLBWidthHeight and
 *    ODDrawLBItems respectively.
 *
 *    Column widths are expressed as character-count offsets from the left
 *    edge.  A caller supplies a tab-stop array such as {10, 20, 35, 0};
 *    the library translates those character positions to pixel offsets
 *    using the font metric obtained at measure time.
 *
 *  Original author: ASH Software, Inc., 1992
 *  Updates: Martin Iturbide / OS2World, 2026
 *
 *  Copyright (c) 1992 ASH Software, Inc.
 *  Distributed under the BSD 3-Clause License.
 *
 *  Build:
 *    GCC (bitwiseworks):   make -f makefile-gcc
 *    OpenWatcom 2.0:       wmake -f makefile-ow
 *
 *  Update History:
 *    07/03/92 - Original source (ASH Software, Inc.)
 *    2026     - Moved to src/, updated build system, documentation
 *
 *********************************************************************/

#define INCL_WIN
#define INCL_GPI

#include <os2.h>
#include <string.h>
#include "owndraw.h"

/*********************************************************************
 *  ODInitLBWidthHeight
 *
 *  Call this from your WM_MEASUREITEM handler.  It queries the current
 *  font metrics from the listbox presentation space, stores the
 *  per-character width for later use by ODDrawLBItems, and returns the
 *  item dimensions that PM needs to allocate space for each row.
 *
 *  Arguments:
 *    hListBox    - handle to the owner-draw listbox window
 *    sfCharWidth - AVERAGE_WIDTH: use lAveCharWidth (tighter packing)
 *                  MAXIMUM_WIDTH: use lMaxCharInc (guaranteed no overlap)
 *    sCharMax    - maximum number of characters in any item string;
 *                  used to compute the total item width
 *    lCharWidth  - OUT: receives the pixel width chosen by sfCharWidth;
 *                  pass this value to every ODDrawLBItems call
 *
 *  Returns:
 *    MRESULT where HIUSHORT = row height (lMaxBaselineExt) and
 *                  LOUSHORT = total item pixel width (sCharMax * charWidth).
 *    Return this value directly from your WM_MEASUREITEM handler.
 *********************************************************************/
MRESULT EXPENTRY ODInitLBWidthHeight(HWND  hListBox,
                                     SHORT sfCharWidth,
                                     SHORT sCharMax,
                                     LONG *lCharWidth)
{
    SHORT       sCharWidth;
    FONTMETRICS fm;
    HPS         hps;

    hps = WinGetPS(hListBox);
    GpiQueryFontMetrics(hps, (LONG)sizeof(FONTMETRICS), &fm);
    WinReleasePS(hps);

    if (sfCharWidth == AVERAGE_WIDTH)
        sCharWidth = (SHORT)fm.lAveCharWidth;
    else
        sCharWidth = (SHORT)fm.lMaxCharInc;

    *lCharWidth = (LONG)sCharWidth;

    /* Return row height in high word, total width in low word. */
    return MRFROM2SHORT(fm.lMaxBaselineExt, sCharMax * sCharWidth);
}

/*********************************************************************
 *  ODDrawLBItems
 *
 *  Call this from your WM_DRAWITEM handler.  It retrieves the item
 *  text, optionally expands embedded tab characters into spaces, then
 *  renders each column segment at its correct pixel offset so that
 *  columns align regardless of the proportional font in use.
 *
 *  Arguments:
 *    pOwnerItem  - pointer to the OWNERITEM structure from mp2
 *    lCharWidth  - per-character pixel width from ODInitLBWidthHeight
 *    psTabStops  - 1-based column array, e.g. {10,20,35,0}; terminated
 *                  by 0.  Column 1 is the leftmost character position.
 *
 *  Returns:
 *    (MRESULT)TRUE - the item text has been drawn; default highlighting
 *    is performed by the system after this function returns.
 *
 *  Notes:
 *    - Only redraws when pOwnerItem->fsState == pOwnerItem->fsStateOld
 *      (i.e. the item content, not just the selection highlight, changed).
 *    - The tab-stop array must remain valid for the lifetime of the
 *      listbox; declare it static or at file scope.
 *    - Item strings longer than MAX_TEXT_CHARS characters are truncated.
 *********************************************************************/
MRESULT EXPENTRY ODDrawLBItems(POWNERITEM pOwnerItem,
                               LONG       lCharWidth,
                               SHORT     *psTabStops)
{
    SHORT sTab;
    SHORT sLoop;
    SHORT sNumberOfChars;
    SHORT sTabCharPos;
    SHORT sNextCharPos;
    SHORT sPrevCharPos;
    LONG  lSaveXLeft;
    CHAR  cChar;
    CHAR  cTempReadString[MAX_TEXT_CHARS];
    CHAR  cTempString[MAX_TEXT_CHARS];

    /* Only repaint when the item content (not just highlight) changes.
     * The system handles highlight transitions on its own. */
    if (pOwnerItem->fsState != pOwnerItem->fsStateOld)
        return (MRESULT)TRUE;

    /* Fetch the item text from the listbox. */
    WinSendMsg(pOwnerItem->hwnd,
               LM_QUERYITEMTEXT,
               MPFROM2SHORT(pOwnerItem->idItem, MAX_TEXT_CHARS),
               MPFROMP(cTempReadString));

    lSaveXLeft = pOwnerItem->rclItem.xLeft;

    /* --- Pass 1: expand embedded tab characters into spaces ---
     *
     * Walk cTempReadString character by character.  When a '\t' is
     * encountered, advance sNextCharPos to the next tab stop boundary
     * by inserting spaces.  Non-tab characters are copied as-is.
     * The result is stored in cTempString, ready for column drawing. */
    sNextCharPos = 0;
    for (sLoop = 0; sLoop <= (SHORT)strlen(cTempReadString); sLoop++)
    {
        cChar = cTempReadString[sLoop];
        if (cChar == '\t')
        {
            /* Find the first tab stop beyond the current position. */
            sTab = -1;
            do {
                sTab++;
                sTabCharPos = psTabStops[sTab];
            } while ((sTabCharPos < sNextCharPos) && (sTabCharPos != 0));

            /* Convert to 0-based and pad with spaces up to that column. */
            sTabCharPos--;
            for (; sNextCharPos < sTabCharPos; sNextCharPos++)
                cTempString[sNextCharPos] = ' ';
        }
        else
        {
            cTempString[sNextCharPos] = cChar;
            sNextCharPos++;
        }
    }

    /* --- Pass 2: draw each column segment at its pixel offset ---
     *
     * Iterate through the tab-stop array.  For each column, advance
     * xLeft by the character offset of the column start, then call
     * WinDrawText for the characters belonging to that column.
     * The final column (tab stop == 0) gets sNumberOfChars=0xFFFF so
     * WinDrawText draws the remaining text to the right edge. */
    sTab         = -1;
    sNextCharPos =  1;   /* 1-based; the first column starts at position 1 */
    do {
        sTab++;
        sPrevCharPos = sNextCharPos;
        sNextCharPos = psTabStops[sTab];

        if (sNextCharPos > 0)
            sNumberOfChars = sNextCharPos - sPrevCharPos;
        else
            sNumberOfChars = 0x7FFF; /* last column: draw to end of string */

        /* Advance xLeft by sPrevCharPos character widths from the
         * accumulated position (not from lSaveXLeft each time). */
        pOwnerItem->rclItem.xLeft += (LONG)sPrevCharPos * lCharWidth;

        WinDrawText(pOwnerItem->hps,
                    sNumberOfChars,
                    (CHAR *)&cTempString[sPrevCharPos - 1],
                    &pOwnerItem->rclItem,
                    0, 0,
                    DT_LEFT | DT_VCENTER | DT_ERASERECT | DT_TEXTATTRS);

    } while (psTabStops[sTab] > 0);

    /* Restore xLeft so the system can apply default highlighting. */
    pOwnerItem->rclItem.xLeft = lSaveXLeft;

    /* Clear the old-state flag so highlighting is applied next time. */
    if (pOwnerItem->fsState)
        pOwnerItem->fsStateOld = FALSE;

    return (MRESULT)TRUE;
}
