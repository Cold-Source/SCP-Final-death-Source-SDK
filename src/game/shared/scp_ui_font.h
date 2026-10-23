#ifndef SCP_UI_FONT_H
#define SCP_UI_FONT_H
#pragma once

#include "tier1/utlvector.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"

// The engine scheme loader assigns a foreign fallback to Cyrillic even when
// Trebuchet contains those glyphs. Bind the complete text range directly.
struct SCPUITextFont
{
    int tall;
    int flags;
    vgui::HFont font;
};

inline CUtlVector<SCPUITextFont> &SCPUITextFonts()
{
    static CUtlVector<SCPUITextFont> fonts;
    return fonts;
}

inline void SCPUIClearTextFonts()
{
    // ISurface owns the handles and resets its font textures on video changes.
    SCPUITextFonts().RemoveAll();
}

inline vgui::HFont SCPUIGetTextFont( vgui::ISurface *surface, vgui::IScheme *scheme,
    const char *name, bool proportional = true, int extraFlags = 0 )
{
    if ( !surface || !scheme ) return 0;
    const vgui::HFont source = scheme->GetFont( name, proportional );
    if ( !source ) return source;
    const int tall = surface->GetFontTallRequested( source );
    if ( tall <= 0 ) return source;
    const int flags = vgui::ISurface::FONTFLAG_ANTIALIAS | extraFlags;
    CUtlVector<SCPUITextFont> &fonts = SCPUITextFonts();
    for ( int i = 0; i < fonts.Count(); ++i )
        if ( fonts[i].tall == tall && fonts[i].flags == flags ) return fonts[i].font;
    const vgui::HFont font = surface->CreateFont();
    const bool valid = surface->SetFontGlyphSet( font, "Trebuchet MS", tall, 700,
        0, 0, flags, 0x0020, 0x04FF );
    SCPUITextFont entry = { tall, flags, valid ? font : source };
    fonts.AddToTail( entry );
    return entry.font;
}
#endif
