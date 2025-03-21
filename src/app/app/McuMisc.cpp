#include <app/McuMisc.h>
#include <core/Logger.h>
#include <app/Registry.h>
#include <core/Utils.h>

namespace ncc
{

McuMisc::McuMisc(int x, int y, int w, int h, const std::string& label, int labelColor)
	: Base(x, y, w, h, label, labelColor, false)
{
	registry.Add("heater1", "[1!]", "Heater 1"   , "off", 3);
	registry.Add("heater2", "[2@]", "Heater 2"   , "off", 3);
	registry.Add("poe"    , "[Pp]", "PoE"        , "PoE", 5);
	registry.Add("temp"   , "[Tt]", "Temperature", ""   , 8);
	Draw_();
}

void McuMisc::UpdateInfo_(uint32_t secs, uint32_t)
{
#if 1
	// Temperature is coming from MQTT (i.e. TempTask).
	DrawTempInC_();
#else
	// Both heaters are on, increase temperature by 2.0C every second.
	// One heater is on, increase temperature by 1.0C every second.
	// Both heaters are off, decrease temperature by 1.0C every second.

	if (m_lastUpdateSec != secs)
	{
		if (m_heater[0] && m_heater[1])
			m_tempInC = std::min(m_tempRange[1], m_tempInC + 2.0f);
		else if (m_heater[0] || m_heater[1])
			m_tempInC = std::min(m_tempRange[1], m_tempInC + 1.0f);
		else
		{
			// Once camera is warmed up, don't drop below operating temperature.
			if (m_tempInC > m_operatingTempInC)
				m_tempInC = std::max(m_operatingTempInC, m_tempInC - 1.0f);
		}

		m_lastUpdateSec = secs;

		DrawTempInC_();
	}
#endif
}

bool McuMisc::HandleInput_(int ch)
{
	switch (ch)
	{
	case '1': EnableHeater_(0); break;
	case '!': DisableHeater_(0); break;
	case '2': EnableHeater_(1); break;
	case '@': DisableHeater_(1); break;
	case 'P': m_poeLevel = WrapAround(m_poeLevel, -1, 0, 2); DrawPoE_(); break;
	case 'p': m_poeLevel = WrapAround(m_poeLevel,  1, 0, 2); DrawPoE_(); break;
	case 'T': m_tempInC = LimitRange(m_tempInC, -2.0f, -40.0f, 40.0f); DrawTempInC_(); break;
	case 't': m_tempInC = LimitRange(m_tempInC,  2.0f, -40.0f, 40.0f); DrawTempInC_(); break;
	}
	return true;
}

// Inq/Req				Ack				Rsp
// --------------------	---------------	-------------------
// InqHeater1							RspHeater1 <on|off>
// InqHeater2							RspHeater2 <on|off>
// CmdHeater1 <on|off>					RspHeater1 <on|off>
// CmdHeater2 <on|off>					RspHeater2 <on|off>
// InqTemp								RspTemp <N>
// InqPoE								RspPoE <PoE|PoE+|PoE++>

std::string McuMisc::OnMessage_(const std::vector<std::string>& req)
{
	std::string rsp;

	logger()->debug("McuMisc::OnMessage_({})", Join(req)); // (req.size() > 0 ? req[0] : ""));
	if (req.empty())
	{
		return rsp;
	}

	if (req[0] == "help")
	{
		std::ostringstream oss;
		oss << "Component Misc\n"
			<< "InqHeater1\n"
			<< "InqHeater2\n"
			<< "CmdHeater1 [on|off]\n"
			<< "CmdHeater2 [on|off]\n"
			<< "InqTemp\n"
			<< "InqPoE\n";
		return oss.str();
	}

	if (req[0] == "InqHeater1" || req[0] == "InqHeater2")
	{
		std::string digit = req[0].substr(req[0].length() - 1);
		int num = std::stoi(digit) - 1;
		rsp = "RspHeater" + digit + " " + (m_heater[num] ? "on" : "off") + "\n";
	}
	else if ((req[0] == "CmdHeater1" || req[0] == "CmdHeater2") && req.size() == 2)
	{
		std::string digit = req[0].substr(req[0].length() - 1);
		int num = std::stoi(digit) - 1;
		std::string onoff = req[1];
		if (onoff == "on")
			EnableHeater_(num);
		else if (onoff == "off")
			DisableHeater_(num);
		rsp = "RspHeater" + digit + " " + (m_heater[num] ? "on" : "off") + "\n";
	}
	else if (req[0] == "InqTemp")
	{
		rsp = "RspTemp " + std::to_string(m_tempInC) + "\n";
	}
	else if (req[0] == "SetTemp")
	{
		if (req.size() > 1)
		{
			m_tempInC = std::stof(req[1]);
		}
	}
	else if (req[0] == "SetHeater")
	{
		if (req.size() > 2)
		{
			bool enable = (req[1] == "true");
			int num = std::stoi(req[2]);
			if (enable)
				EnableHeater_(num);
			else
				DisableHeater_(num);
		}
	}
	else if (req[0] == "InqPoE")
	{
		switch (m_poeLevel)
		{
		case 0: rsp = "RspPoE PoE"; break;
		case 1: rsp = "RspPoE PoE+"; break;
		case 2: rsp = "RspPoE PoE++"; break;
//		default: rsp= "RspPoE ERROR"; break;
		}
		rsp += "\n";
	}

	return rsp;
}

void McuMisc::EnableHeater_(int num)
{
	m_heater[num] = true;
	DrawHeater_(num);
}

void McuMisc::DisableHeater_(int num)
{
	m_heater[num] = false;
	DrawHeater_(num);
}

void McuMisc::Draw_()
{
	DrawHeater_(0);
	DrawHeater_(1);
	DrawPoE_();
	DrawTempInC_();
}

void McuMisc::DrawHeater_(int num)
{
	int y = (m_useLabelBox ? 3 : 1);
	y += num;
	registry.Update("heater" + std::to_string(num + 1), (m_heater[num] ? "on" : "off"));
	ClearStatus_(y);
	mvwprintw(m_win, y, 2, "Heater%d:%22s%6s", num, " ", (m_heater[num] ? "on" : "off"));
}

void McuMisc::DrawPoE_()
{
	const char* lvl;
	switch (m_poeLevel)
	{
	case 0: lvl = "PoE"; break;
	case 1: lvl = "PoE+"; break;
	case 2: lvl = "PoE++"; break;
	case 3: lvl = "unhandled"; break;
	}
	registry.Update("poe", lvl);

	int y = (m_useLabelBox ? 5 : 3);
	ClearStatus_(y);
	mvwprintw(m_win, y, 2, "PoE:%23s%9s", " ", lvl);
}

void McuMisc::DrawTempInC_()
{
	char value[12];
	sprintf(value, "%6.2f C", m_tempInC);
	registry.Update("temp", value);

	int y = (m_useLabelBox ? 6 : 4);
	ClearStatus_(y);
	mvwprintw(m_win, y, 2, "Temperature:%16s%6.2f C", " ", m_tempInC);
}

} // namespace ncc
