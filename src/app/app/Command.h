#pragma once

#include <app/Base.h>
#include <app/Status.h>

#include <deque>
#include <map>
#include <string>

#define NCURSES_NOMACROS
#include <panel.h>

namespace ncc
{

/*
 * Ideas:
 * - Switching between different modes:
 *   - Commands (words specifying a component and what it should do)
 *   - Characters (Use characters in Registry to quickly control components)
 *   - Tab between panels to control specific panels
 * - Show output from commands above command prompt or in Log panel
 *   and allow scrolling.
 * - Support long lines by scrolling text.
 * - Allow cursoring through line to fix typos and scroll as necessary.
 */
class Command : public Base //, public ISubscriber
{
public:
	~Command() override = default;
	Command(int x, int y, int w, int h, const std::string& label, int labelColor);

private:
	void UpdateInfo_(uint32_t secs, uint32_t usecs) override;
	bool HandleInput_(int ch) override;
//	std::string ProcessRequest_(const std::vector<std::string>& req) override;

	void Draw_();

	void AddCommand_(const std::string& cmd);
	void RemoveCommand_(const std::string& cmd);
	void GetPrevCommand_();
	void GetNextCommand_();
	void ShowCursor_(int h);
	void Resize_() override;
	std::vector<std::string> SplitLine_(const std::string& cmd);

private:
	Status m_status;
	std::map<int, std::function<void()>> m_handlers;
	std::pair<int, int> m_range = { 0, 0 };

	std::deque<std::string> m_cmds;
	std::deque<std::string> m_lines;
	std::string m_line;
	int m_cursorPos {0};
	size_t m_cmdOff {0};
};

} // namespace ncc
