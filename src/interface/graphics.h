#ifndef FILEZILLA_GRAPHICS_HEADER
#define FILEZILLA_GRAPHICS_HEADER

#include <wx/rawbmp.h>
#include <wx/window.h>

static inline unsigned char AlphaComposite_Over_GetAlpha(unsigned char bg_alpha, unsigned char fg_alpha)
{
	return bg_alpha + fg_alpha - bg_alpha * fg_alpha / 255;
}

// Do not call with zero new_alpha
static inline unsigned char AlphaComposite_Over(unsigned char bg, unsigned char bg_alpha, unsigned char fg, unsigned char fg_alpha, unsigned char new_alpha)
{
	return (bg * (255 - fg_alpha) * bg_alpha / 255 + fg * fg_alpha) / new_alpha;
}

// Alpha compositing of a single pixel, b gets composited over a
// (well-known over operator), result stored in a.
// All RGB and A values have range from 0 to 255, RGB values aren't
// premultiplied by A.
// Safe for multiple compositions.
static inline void AlphaComposite_Over_Inplace(wxAlphaPixelData::Iterator &bg, wxAlphaPixelData::Iterator &fg)
{
	if (!fg.Alpha()) {
		// Nothing to do. Also prevents zero new_alpha
		return;
	}

	unsigned char const new_alpha = AlphaComposite_Over_GetAlpha(bg.Alpha(), fg.Alpha());
	bg.Red()   = AlphaComposite_Over(bg.Red(),   bg.Alpha(), fg.Red(),   fg.Alpha(), new_alpha);
	bg.Green() = AlphaComposite_Over(bg.Green(), bg.Alpha(), fg.Green(), fg.Alpha(), new_alpha);
	bg.Blue()  = AlphaComposite_Over(bg.Blue(),  bg.Alpha(), fg.Blue(),  fg.Alpha(), new_alpha);
	bg.Alpha() = new_alpha;
}

static inline wxColour AlphaComposite_Over(wxColour const& bg, wxColour const& fg) {
	if (!fg.IsOk() || !fg.Alpha()) {
		// Nothing to do. Also prevents zero new_alpha
		return bg;
	}

	unsigned char const new_alpha = AlphaComposite_Over_GetAlpha(bg.Alpha(), fg.Alpha());
	return wxColour(
		AlphaComposite_Over(bg.Red(),   bg.Alpha(), fg.Red(),   fg.Alpha(), new_alpha),
		AlphaComposite_Over(bg.Green(), bg.Alpha(), fg.Green(), fg.Alpha(), new_alpha),
		AlphaComposite_Over(bg.Blue(),  bg.Alpha(), fg.Blue(),  fg.Alpha(), new_alpha),
		new_alpha
	);
}

static inline void SetWindowBackgroundTint(wxWindow& wnd, wxColour const& tint)
{
	wxColour const bg = wnd.GetDefaultAttributes().colBg;
	wxColour const newColour = AlphaComposite_Over(bg, tint);
	if (newColour != wnd.GetBackgroundColour()) {
		if (wnd.SetBackgroundColour(newColour)) {
			wnd.Refresh();
		}
	}
}

#endif
