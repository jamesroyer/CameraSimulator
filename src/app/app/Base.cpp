#include <app/Base.h>
//#include <core/Logger.h>

namespace ncc
{

int WrapAround(int v, int delta, int minval, int maxval)
{
	const int mod = maxval + 1 - minval;
	v += delta - minval;
	v += (1 - v / mod) * mod;
	return v % mod + minval;
}

Base::~Base()
{
	del_panel(m_panel);
	delwin(m_win);
}

Base::Base(
		int x,
		int y,
		int w,
		int h,
		const std::string& label,
		int labelColor,
		bool useLabelBox,
		int bgColor,
		int fgColor)
	: m_label(label)
	, m_labelColor(labelColor)
	, m_bgColor(bgColor)
	, m_fgColor(fgColor)
	, m_x(x)
	, m_y(y)
	, m_w(w)
	, m_h(h)
	, m_useLabelBox(useLabelBox)
{
	Init();

	if (m_bgColor >= 0 && m_fgColor >= 0)
	{
		wbkgd(m_win, COLOR_PAIR(m_bgColor) | ' ');
		wattrset(m_win, COLOR_PAIR(m_fgColor));
	}
}

void Base::Init()
{
	if (m_w < 0)
	{
		m_w = COLS;
	}
	if (m_y < 0)
	{
		m_y = LINES - m_h - 1; // 1 => room for status line
	}
	RecreateWindow_();
}

void Base::RecreateWindow_()
{
	// Delete existing window and panel.
	if (m_panel)
	{
		del_panel(m_panel);
		m_panel = nullptr;
	}
	if (m_win)
	{
		delwin(m_win);
		m_win = nullptr;
	}

	// (Re-)create window and panel.
	m_win = newwin(m_h, m_w, m_y, m_x);
	m_panel = new_panel(m_win);
	set_panel_userptr(m_panel, this);

	if (m_bgColor >= 0 && m_fgColor >= 0)
	{
		wbkgd(m_win, COLOR_PAIR(m_bgColor) | ' ');
		wattrset(m_win, COLOR_PAIR(m_fgColor));
	}

	// Draw the panel and update the screen.
	Draw();
}

// TODO: Implement a layout mananager to shrink, stretch, show, or hide
// windows if the user adjusts the terminal window.
//
// Currently only the Command window supports resize.
void Base::Resize()
{
	if (!m_supportsResize)
	{
		return;
	}

	// Give derived class a chance to adjust position.
	Resize_();

	RecreateWindow_();
}

void Base::Resize_()
{
}

void Base::Draw()
{
	int h __attribute__((unused));
	int w;
	getmaxyx(m_win, h, w);

	box(m_win, 0, 0);
	if (m_h > 3 && m_useLabelBox)
	{
		mvwaddch(m_win, 2, 0, ACS_LTEE);
		mvwhline(m_win, 2, 1, ACS_HLINE, w - 2);
		mvwaddch(m_win, 2, w - 1, ACS_RTEE);
		CenterText(0, 1, w, m_label, COLOR_PAIR(m_labelColor));
	}
	else
	{
		PrintText(2, 0, m_label, COLOR_PAIR(m_labelColor));
	}
}

void Base::CenterText(int x, int y, int w, const std::string& s, chtype color)
{
	int len = s.length();
	float temp = (w - len) / 2.0;
	x = x + temp;
	PrintText(x, y, s, color);
}

void Base::PrintText(int x, int y, const std::string& s, chtype color)
{
	WINDOW* win = m_win;
	if (win == nullptr)
	{
		win = stdscr;
	}
	wattron(win, color);
	mvwprintw(win, y, x, "%s", s.c_str());
	wattroff(win, color);
}

bool Base::IsHidden() const
{
	return m_hidden;
}

void Base::Show()
{
	show_panel(m_panel);
	m_hidden = false;
	update_panels();
	doupdate();
}

void Base::Hide()
{
	hide_panel(m_panel);
	m_hidden = true;
	update_panels();
	doupdate();
}

void Base::ToggleShowHide()
{
	IsHidden() ? Show() : Hide();
}

void Base::Set(int value)
{
	m_value = value;
}

int Base::Get() const
{
	return m_value;
}

void Base::UpdateInfo(uint32_t secs, uint32_t usecs)
{
	UpdateInfo_(secs, usecs);
}

bool Base::HandleInput(int ch)
{
	return HandleInput_(ch);
}

std::string Base::OnMessage(const std::vector<std::string>& msg)
{
	return OnMessage_(msg);
}

void Base::SetSendMessageFn(SendMessageFn fn)
{
	m_sendMsgFn = fn;
}

void Base::SendMessage_(const std::vector<std::string>& msg)
{
	if (m_sendMsgFn)
	{
		m_sendMsgFn(msg);
	}
}

void Base::UpdateInfo_(uint32_t, uint32_t)
{
}

bool Base::HandleInput_(int ch)
{
	return true;
}

std::string Base::OnMessage_(const std::vector<std::string>&)
{
	return std::string{};
}

void Base::ClearStatus_(int y)
{
	int h __attribute__((unused));
	int w;
	getmaxyx(m_win, h, w);

	wattrset(m_win, COLOR_PAIR(0) | A_INVIS);
	mvwprintw(m_win, y, 1, "%*.s", w-2, " ");
	wattrset(m_win, 0);
}

void Base::ShowMessage_(const std::string& msg)
{
	ClearStatus_(LINES - 2);
	mvprintw(LINES - 2, 0, "%s: %s", m_label.c_str(), msg.c_str());
}

} // namespace ncc
