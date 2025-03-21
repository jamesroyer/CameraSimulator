#pragma once

#include <cstddef>

#define NCURSES_NOMACROS
#include <panel.h>

namespace ncc // Ncurses Camera
{

class Status
{
public:
	void SetCommandCount(size_t size) { m_cmdSize = size; }
	void SetCommandOffset(size_t offset) { m_cmdOffset = offset; }

	void UpdateStatusLine(int y)
	{
		ClearStatus_(y);
		attron(COLOR_PAIR(4));
		mvprintw(y, 0, "Cmds: %lu, Offset: %lu", m_cmdSize, m_cmdOffset);
		attroff(COLOR_PAIR(4));

		ClearStatus_(y+1);
		attron(COLOR_PAIR(4));
		mvprintw(y+1, 0, "Type 'quit' to exit or 'help' for more information.");
		attroff(COLOR_PAIR(4));
	}

private:
	size_t m_cmdSize = 0;
	size_t m_cmdOffset = 0;

	void ClearStatus_(int y)
	{
		attrset(COLOR_PAIR(0) | A_INVIS);
		mvprintw(y, 0, "%*.s", COLS, " ");
		attrset(0);
	}
};

} // namespace ncc
