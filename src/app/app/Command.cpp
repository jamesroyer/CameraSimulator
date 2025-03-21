#include <app/Command.h>

#include <iterator>
#include <sstream>

namespace ncc
{

Command::Command(
		int x,
		int y,
		int w,
		int h,
		std::string const& label,
		int labelColor)
//	: Base(x, y, w, h, label, 12, 13, 0)
	: Base(x, y, w, h, label, labelColor)
{
	m_supportsResize = true;

	// For testing, pre-populate the commands for quick access.
	m_cmds.push_back("CmdMoveBy Pan 30");
	m_cmds.push_back("CmdMoveTo Pan -90");
	m_cmds.push_back("CmdMoveBy Tilt 15");
	m_cmds.push_back("CmdMoveTo Tilt -45");
	m_status.SetCommandCount(m_cmds.size());

	Draw_();
}

void Command::Resize_()
{
	// Based on new COLS and LINES values, we can adjust this windows
	// position and size to keep it at the bottom of the screen.
	m_w = COLS;
	m_y = LINES - m_h - 1; // minus one is for status line.
}

bool Command::HandleInput_(int ch)
{
	bool ret {true};

	switch (ch)
	{
	case KEY_UP:
		GetPrevCommand_();
		break;

	case KEY_DOWN:
		GetNextCommand_();
		break;

	case 27: // ESC
		m_line.clear();
		m_cursorPos = 0;
		break;

	case KEY_BACKSPACE:
	case 127:
		if (m_cursorPos > 0)
		{
			m_line.erase(--m_cursorPos, 1);
			Draw_();
		}
		break;

	case KEY_DC:
		if ((size_t)m_cursorPos < m_line.size())
		{
			m_line.erase(m_cursorPos, 1);
		}
		break;

	case KEY_ENTER:
	case 10:
		if (!m_line.empty())
		{
			if (m_line == "quit" || m_line == "exit")
			{
				ret = false;
			}
			m_lines.push_back(std::to_string(m_lines.size() + 1) + ": " + m_line);
			AddCommand_(m_line);
			m_line.clear();
			m_cursorPos = 0;
		}
		break;

	case KEY_BTAB:
	case KEY_CTAB:
	case KEY_STAB:
	case KEY_CATAB:
	case 9:
		m_line += "<TAB>";
		m_cursorPos += 5;
		break;

	default:
		if (isprint(ch))
		{
			m_line += char(ch);
			++m_cursorPos;
		}
		break;
	}
	return ret;
}

void Command::AddCommand_(std::string const& cmd)
{
	// Don't allow duplicates and add new commands to end of list.
	RemoveCommand_(cmd);
	m_cmds.push_back(cmd);
	m_cmdOff = 0;
	m_status.SetCommandCount(m_cmds.size());

	SendMessage_(SplitLine_(cmd));
}

void Command::RemoveCommand_(std::string const& cmd)
{
	for (auto it = m_cmds.begin(); it != m_cmds.end(); ++it)
	{
		if (*it == cmd)
		{
			m_cmds.erase(it);
			break;
		}
	}
}

void Command::GetPrevCommand_()
{
	size_t size = m_cmds.size();
	if (m_cmdOff < size)
	{
		m_line = m_cmds.at(size - m_cmdOff - 1);
		m_cursorPos = m_line.size();
		++m_cmdOff;
		m_status.SetCommandOffset(m_cmdOff);
	}
}

void Command::GetNextCommand_()
{
	/*
	 *                    m_cmdOff   m_cmds[n]
	 * 1: one             3          n=0
	 * 2: two             2          n=1
	 * 3: three           1          n=2
	 * -: (empty line)    0
	 */
	if (m_cmdOff > 0)
	{
		--m_cmdOff;
		if (m_cmdOff == 0)
		{
			m_line.clear();
			m_cursorPos = 0;
		}
		else
		{
			size_t size = m_cmds.size();
			m_line = m_cmds.at(size - m_cmdOff);
			m_cursorPos = m_line.size();
		}
		m_status.SetCommandOffset(m_cmdOff);
	}
}

void Command::Draw_()
{
	int h __attribute__((unused));
	int w;
	getmaxyx(m_win, h, w);
#if 0
	int x = 2;
	int y = 3;

	int h1 = h - y - 2;
	uint32_t maxLines = h1;
	if ((size_t) h1 > m_lines.size())
	{
		maxLines = m_lines.size();
	}
	uint32_t index = m_lines.size() - maxLines;
	auto it = std::next(m_lines.begin(), index);

	for (; it != m_lines.end(); ++it)
	{
		ClearStatus_(y);
		mvwaddnstr(m_win, y++, x, (*it).c_str(), w - x - x);
	}
#endif
	ClearStatus_(h-2);
	mvwprintw(m_win, h-2, 1, "> %-.*s", w-4, m_line.c_str());

	ShowCursor_(h);
}

void Command::ShowCursor_(int h)
{
	wattron(m_win, COLOR_PAIR(17));
	mvwprintw(m_win, h-2, 3 + m_cursorPos, " ");
	wattroff(m_win, COLOR_PAIR(17));
}

void Command::UpdateInfo_(uint32_t secs, uint32_t usecs)
{
	Draw_();
}

#if 0
// [[deprecated("Moved to Base class")]]
void Command::ClearStatus_(int y)
{
	int h __attribute__((unused));
	int w;
	getmaxyx(m_win, h, w);

	wattrset(m_win, COLOR_PAIR(0) | A_INVIS);
	mvwprintw(m_win, y, 1, "%*.s", w-2, " ");
	wattrset(m_win, 0);
}
#endif

#if 0
void Command::AddLine(const std::string& line)
{
	mvwprintw(m_win, m_curLine+2, 1, "%s", line.c_str());
	++m_curLine;
	++m_lineCount;
}
#endif

std::vector<std::string> Command::SplitLine_(const std::string& cmd)
{
	std::istringstream ss{cmd};
	std::istream_iterator<std::string> begin(ss);
	std::istream_iterator<std::string> end;
	std::vector<std::string> result;
	for (auto it = begin; it != end; ++it)
	{
		result.push_back(*it);
	}
	return result;
}

} // namespace ncc
