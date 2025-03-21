#include <app/Compass.h>
#include <app/Base.h>
#include <app/Registry.h>

#include <string>

namespace ncc
{

Compass::Compass(
		int x,
		int y,
		int w,
		int h,
		const std::string& label,
		int labelColor,
		const std::string& keys,
		bool neg)
	: Base(x, y, w, h, label, labelColor, false)
	, m_label(label)
{
	if (keys.length() != 4) throw std::runtime_error("Need four keys to bind to functions.");

	registry.Add(label + "_move" , "[" + keys.substr(0,2) + "]", label + ": move", "idle", 6);
	registry.Add(label + "_dir"  , "[" + keys.substr(2,2) + "]", label + ": direction", "backward", 8);

	m_handlers[keys[0]] = [this]() { SetMoving_(false); };
	m_handlers[keys[1]] = [this]() { SetMoving_(true); };
	m_handlers[keys[2]] = [this]() { m_stepMove = true; SetDirection_(-1);  };
	m_handlers[keys[3]] = [this]() { m_stepMove = true; SetDirection_( 1); };

	if (neg)
	{
		Set(1);
		m_range.first = -180;
		m_range.second = 180;
	}
	Draw_();
}

/*
 * There are two movement options:
 *   1. Continuous movement in either direction (wraps around).
 *   2. Limited movement with defined by an lower and upper bound.
 */
void Compass::MoveTo(int pos)
{
	if (m_range.first == 0 && m_range.second == 0)
	{
		pos = WrapAround(pos, 0, 0, 359);
		m_desiredPos = pos;

		// Continuous movement:
		// Compute the difference then normalize it to [-180,180].
		m_direction = ((m_desiredPos - m_pos + 360) % 360 < 180) ? 1 : -1;
	}
	else
	{
		// Partial rotation - rotate from min to max.
		m_desiredPos = std::min(m_range.second, m_desiredPos);
		m_desiredPos = std::max(m_range.first, m_desiredPos);
		m_direction = (m_desiredPos > m_pos ? 1 : -1);
//		ShowMessage_(std::string("desiredPos=") + std::to_string(m_desiredPos));
	}
	m_movement = movement_t::moveto;
	registry.Update(m_label + "_move", (m_movement == movement_t::idle ? "idle" : "moving"));
}

void Compass::MoveBy(int value)
{
	m_desiredPos = m_pos + value;
	if (m_range.first == 0 && m_range.first == 0)
	{
		// XXX: Add better logic to find shortest route.
		m_direction = (m_desiredPos > m_pos ? 1 : -1);
	}
	else
	{
		m_desiredPos = std::min(m_range.second, m_desiredPos);
		m_desiredPos = std::max(m_range.first, m_desiredPos);
		m_direction = (m_desiredPos > m_pos ? 1 : -1);
	}
	m_movement = movement_t::moveto;
	registry.Update(m_label + "_move", (m_movement == movement_t::idle ? "idle" : "moving"));
}

void Compass::MoveCont()
{
	m_movement = movement_t::movecont;
	registry.Update(m_label + "_move", (m_movement == movement_t::idle ? "idle" : "moving"));
}

int Compass::GetPosition() const
{
	return m_pos;
}

std::string Compass::GenerateRepeatingPattern_(const std::string pattern, size_t width, size_t offset)
{
	if (offset > pattern.size() || offset > width)
		throw std::out_of_range("Offset cannot exceed pattern length or generated string length.");
	
	std::string repeatedPattern;
	repeatedPattern.reserve(width / pattern.length() + 2 * pattern.length());
	while (repeatedPattern.length() < offset + width)
	{
		repeatedPattern += pattern;
	}
	std::string result = repeatedPattern.substr(offset, width);
	if (result.length() != width)
		throw std::runtime_error("Generated string from pattern is too short.");

	return result;
}

void Compass::Draw_()
{
	if (Get() == 0)
		DrawPos_();
	else
		DrawNeg_();
}

void Compass::DrawPos_()
{
	int h __attribute__((unused));
	int w;
	getmaxyx(m_win, h, w);

	w -= 2;

	int c = w / 2;
	int nearest = (m_pos + 2) / 5 * 5;
	int lo = nearest - (c + 2) / 5 * 5;
	int hi = nearest + (c + 2) / 5 * 5;
	int dist = nearest - m_pos;

	std::ostringstream oss;
	oss << std::setfill('0');
	for (int i = lo; i <= hi; i += 5)
	{
		int x = i;
		if (x < 0)    x += 360;
		if (x >= 360) x -= 360;
		oss << std::setw(3) << x << "  ";
	}
	size_t mid = oss.str().size() / 2 - dist;
	try
	{
		int y = (m_useLabelBox ? 3 : 1);
//		mvwprintw(win  , y, x, fmt, ...);
		mvwprintw(m_win, y+0, 1, "%s", oss.str().substr(mid - c - 1, w).c_str());
		mvwprintw(m_win, y+1, 1, "%s", GenerateRepeatingPattern_(m_ticks, w, m_tickOffset).c_str());
		mvwchgat(m_win, y+0, c  , 3, A_BOLD, 1, nullptr);
		mvwchgat(m_win, y+1, c+1, 1, A_BOLD, 1, nullptr);
	}
	catch (const std::exception& e)
	{
		mvwprintw(stdscr, LINES-3, 1, "%s (%ld)", e.what(), m_tickOffset);
	}
}

void Compass::DrawNeg_()
{
	int h __attribute__((unused));
	int w;
	getmaxyx(m_win, h, w);

	w -= 2;

	int c = w / 2;
	int nearest;
	int lo;
	int hi;
	int dist;
	if (m_pos >= 0)
	{
		nearest = (m_pos + 2) / 5 * 5;
		lo = nearest - (c + 2) / 5 * 5;
		hi = nearest + (c + 2) / 5 * 5;
		dist = nearest - m_pos;
	}
	else
	{
		nearest = (m_pos - 2) / 5 * 5;
		lo = nearest - (c + 2) / 5 * 5;
		hi = nearest + (c + 2) / 5 * 5;
		dist = nearest - m_pos;
	}

	std::ostringstream oss;
	oss << std::setfill('0') << std::internal;
	for (int i = lo; i <= hi; i += 5)
	{
		int x = i;
		if (i == 0)
			oss << " " << std::setw(3) << x << "  ";
		else if (i >= 0)
			oss << std::setw(3) << x << "  ";
		else
			oss << std::setw(4) << x << " ";
	}
	size_t mid;
	if (oss.str().substr(oss.str().length() - 2) == "  ")
		mid = oss.str().size() / 2 - dist;
	else
		mid = oss.str().size() / 2 - dist + 1;

	try
	{
		int y = (m_useLabelBox ? 3 : 1);
		mvwprintw(m_win, y+0, 1, "%s", oss.str().substr(mid - c - 1, w).c_str());
		mvwprintw(m_win, y+1, 1, "%s", GenerateRepeatingPattern_(m_ticks, w, m_tickOffset).c_str());
		if (m_pos >= 0)
			mvwchgat(m_win, y+0, c, 3, A_BOLD, 1, nullptr);
		else
			mvwchgat(m_win, y+0, c-1, 4, A_BOLD, 1, nullptr);
		mvwchgat(m_win, y+1, c+1, 1, A_BOLD, 1, nullptr);
	}
	catch (const std::exception& e)
	{
		mvwprintw(stdscr, LINES-3, 1, "%s (%ld)", e.what(), m_tickOffset);
	}
}

void Compass::SetMoving_(bool value)
{
	m_movement = (value ? movement_t::movecont : movement_t::idle);
	registry.Update(m_label + "_move", (m_movement == movement_t::idle ? "idle" : "moving"));
}

void Compass::SetDirection_(int value)
{
	m_direction = value;
	registry.Update(m_label + "_dir", (m_direction == 1 ? "forward" : "backward"));
	Advance_();
}

void Compass::Advance_()
{
	if (Get() == 0)
		AdvancePos_();
	else
		AdvanceNeg_();
}

void Compass::AdvancePos_()
{
	if (m_movement != movement_t::idle || m_stepMove)
	{
		if (m_stepMove) m_stepMove = false;
		m_tickOffset = WrapAround(m_tickOffset, m_direction, 0, m_ticks.size() - 1);
		m_pos = WrapAround(m_pos, m_direction, 0, 359);
		Draw_();
	}
}

void Compass::AdvanceNeg_()
{
	if (m_movement != movement_t::idle || m_stepMove)
	{
		if (m_stepMove) m_stepMove = false;
		if (m_pos + m_direction >= m_range.first && m_pos + m_direction <= m_range.second)
		{
			m_tickOffset = WrapAround(m_tickOffset, m_direction, 0, m_ticks.size() - 1);
			m_pos = LimitRange(m_pos, m_direction, m_range.first, m_range.second);
			Draw_();
		}
	}
}

void Compass::UpdateInfo_(uint32_t, uint32_t)
{
	switch (m_movement)
	{
	case movement_t::idle:
		break;

	case movement_t::movecont:
		Advance_();
		break;

	case movement_t::moveto:
		if (m_pos == m_desiredPos)
		{
			m_movement = movement_t::idle;
			registry.Update(m_label + "_move", (m_movement == movement_t::idle ? "idle" : "moving"));
			SendMessage_({"RspMoveTo " , m_label });
		}
		else
			Advance_();
		break;
	}
}

bool Compass::HandleInput_(int ch)
{
	auto it = m_handlers.find(ch);
	if (it != m_handlers.end())
	{
		it->second();
	}
	return true;
}

// Inq/Req									Ack					Rsp
// ----------------------------------------	------------------- -------------------
// InqPos <label>												RspPos <label> N
// CmdMoveTo <label> N						AckMoveTo <label>	RspMoveTo <label>
// CmdMoveBy <label> N						AckMoveBy			RspMoveTo **bug**
// CmdMoveAt <label> N											RspMoveAt
// CmdMoveDir <label> <backward|forward>						RspMoveDir
//
// Note: <label> is the title of the pan (e.g. "Pan" or "Tilt").

std::string Compass::OnMessage_(const std::vector<std::string>& req)
//std::string Compass::ProcessRequest_(const std::vector<std::string>& req)
{
	std::string rsp;

	if (req.size() == 1 && req[0] == "help")
	{
		std::ostringstream oss;
		oss
			<< "Component " << m_label << '\n'
			<< "  InqPos " << m_label << '\n'
			<< "  CmdMoveTo " << m_label << " N "
			<< (Get() == 0 ? "(N => [-180,180])" : "(N => [0,360])") << "\n"
			<< "  CmdMoveBy " << m_label << " N\n"
			<< "  CmdMoveAt " << m_label << " N\n"
			<< "  CmdMoveDir " << m_label << " [backward|forward]\n";
		rsp = oss.str();
	}

	if (req.size() < 2)
		return rsp;

#if 0
	std::string msg;
	for (auto s : req)
		msg += s + " ";
	ShowMessage_(std::string("Message received!")
		+ " req.size()=" + std::to_string(req.size())
		+ " msg=" + msg);
#endif

	if (req[1] != m_label)
		return rsp;


	if (req[0] == "InqPos")
	{
		rsp = "RspPos " + m_label + " " + std::to_string(m_pos) + "\n";
	}
	else if (req[0] == "CmdMoveTo" && req.size() == 3)
	{
		int32_t pos = std::stoi(req[2]);
		MoveTo(pos);
		rsp = "AckMoveTo " + m_label + "\n";
	}
	else if (req[0] == "CmdMoveBy" && req.size() == 3)
	{
		uint32_t value = std::stoi(req[2]);
		MoveBy(value);
		rsp = "AckMoveBy\n";
	}
	else if (req[0] == "CmdMoveAt" && req.size() == 3)
	{
		MoveCont();
		rsp = "RspMoveAt\n";
	}
	else if (req[0] == "InqMoveDir" && req.size() == 3)
	{
		rsp = "CmdMoveDir ";
		if (m_direction == -1)
			rsp += "backward\n";
		else
			rsp += "forward\n";
	}
	else if (req[0] == "CmdMoveDir" && req.size() == 3)
	{
		std::string dir = req[2];
		if (dir == "backward")
			m_direction = -1;
		else if (dir == "forward")
			m_direction = 1;

		rsp = "RspMoveDir " + dir + "\n";
	}
	return rsp;
}

} // namespace ncc
