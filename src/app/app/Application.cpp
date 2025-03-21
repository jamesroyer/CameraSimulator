// ----------------------------------------------------------------------------
// TODO
//   - Allow resize.
//     - Panels can be rearranged to make them fit.
//     - Pan and Tilt can be reduced horizontally and they will adapt.
//   - Add highligting/bold to values.
//   - Add a scrolling log window.
//   - Figure out how to use pts to simulate a device.
//     - CameraApp should send commands and get responses as if it was talking
//       to real hardware.
//   - Support heaters in the image module as well as the MCU.
// ----------------------------------------------------------------------------
#include <app/Application.h>
#include <app/Command.h>
#include <app/Compass.h>
#include <core/IMqttClient.h>
//#include <app/Info.h>
//#include <app/Lens.h>
#include <core/Logger.h>
#include <app/McuMisc.h>

#include <curses.h>
#include <sstream>

#include <sys/time.h>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>

extern int g_exit;

namespace ncc
{

Application::Application(IMqttClient& mqttClient)
	: m_mqtt(mqttClient)
{
	logger()->trace("Application::Application()");

	// Initialize curses.
	initscr();
	start_color();
	curs_set(0);
	noecho();
	cbreak();
	keypad(stdscr, true);
	nodelay(stdscr, true);

	// Initialize all the colors.
	init_pair(1, COLOR_RED   , COLOR_BLACK);
	init_pair(2, COLOR_GREEN , COLOR_BLACK);
	init_pair(3, COLOR_BLUE  , COLOR_BLACK);
	init_pair(4, COLOR_CYAN  , COLOR_BLACK);
	init_pair(5, COLOR_YELLOW, COLOR_BLACK);

	init_pair(10, COLOR_BLACK  , COLOR_BLUE);
	init_pair(11, COLOR_RED    , COLOR_BLUE);
	init_pair(12, COLOR_GREEN  , COLOR_BLUE);
	init_pair(13, COLOR_YELLOW , COLOR_BLUE);
	init_pair(14, COLOR_BLUE   , COLOR_BLUE);
	init_pair(15, COLOR_MAGENTA, COLOR_BLUE);
	init_pair(16, COLOR_CYAN   , COLOR_BLUE);
	init_pair(17, COLOR_WHITE  , COLOR_BLUE);

	AddSubscriptions_();
//	m_notifier.Add(const std::string &topic, MethodHandle callback)

	InitWindows_();
	SetActiveWindow_("Cmd");
}

Application::~Application()
{
	logger()->trace("Application::~Application()");
	endwin();
}

void Application::Run()
{
	logger()->trace("Application::Run()");

	update_panels();

	ShowStatusLine_();

	doupdate();

	for (;;)
	{
		if (g_exit)
		{
			break;
		}

		struct timeval tv;
		gettimeofday(&tv, nullptr);

		struct tm* ct = localtime(&tv.tv_sec);

		for (auto& [name, win] : m_wins)
		{
			if (win)
			{
				win->UpdateInfo(ct->tm_sec, tv.tv_usec);
			}
		}

		// Keyboard input is sent to "Cmd" window for processing.
		// All other windows receive input via their "..." method.
		int ch = getch();
		if (ch == KEY_F(1))
			break;

		if (ch == KEY_RESIZE)
		{
			Resize_();
		}
		else if (ch != ERR)
		{
			if (!m_activeWin)
			{
				SetActiveWindow_("Cmd");
			}
			if (m_activeWin)
			{
				if (!m_activeWin->HandleInput(ch))
				{
					break;
				}
			}
		}

		update_panels();
		doupdate();

		usleep(30000);
	}
}

void Application::InitWindows_()
{
	Base* win {nullptr};

	{
		// TODO: Use a layout manager to grow/shrink pan & tilt panels.
		int x = 0;
		int y = 0;
		int w = 40;
		int h = 4; // useLabelBox => 4, !useLabelBox => 6
		win = new Compass(x, y, w, h, "Pan" , 4, "Mm,.");
//		win->SetResponseHandler([this](const std::string& msg) { SendResponse_(msg); });
		win->SetSendMessageFn([this](const std::vector<std::string>& msg) { OnCompMessage_(msg); });
		m_wins["Pan"] = win;
	}

	{
		int x = 40;
		int y = 0;
		int w = 40;
		int h = 4; // Base::useLabelBox => 4, !Base::useLabelBox => 6
		win = new Compass(x, y, w, h, "Tilt", 4, "Nn[]", true);
		win->SetSendMessageFn([this](const std::vector<std::string>& msg) { OnCompMessage_(msg); });
		m_wins["Tilt"] = win;
	}

#if 0
	{
		int x = 0;
		int y = 4; // Base::useLabelBox => 6, !Base::useLabelBox => 4
		int w = 40;
		int h = 6; // Base::useLabelBox => 8, !Base::useLabelBox => 6
		win = new Lens(x, y, w, h, "Lens", 2);
		win->SetSendMessageFn([this](const std::vector<std::string>& msg) { OnCompMessage_(msg); });
		m_wins["Lens"] = win;
	}
#endif

	{
		int x = 40;
		int y = 4; // Base::useLabelBox => 6, !Base::useLabelBox => 4
		int w = 40;
		int h = 6; // Base::useLabelBox => 8, !Base::useLabelBox => 6
		win = new McuMisc(x, y, w, h, "Miscellaneous", 4);
		win->SetSendMessageFn([this](const std::vector<std::string>& msg) { OnCompMessage_(msg); });
		m_wins["Misc"] = win;
	}

#if 1
	{
		int w = COLS;
		int h = 3;
		int x = 0;
		int y = LINES - h - 2;
		win = new Command(x, y, w, h, "Cmd", 5);
		win->SetSendMessageFn([this](const std::vector<std::string>& msg) { OnCompMessage_(msg); });
		m_wins["Cmd"] = win;
	}
#else
	win = new LogWin(0, 14, 80, 10, "Log", 5);
	win->SetResponseHandler([this](const std::string& msg) { SendResponse_(msg); });
#endif

#if 0
	{
		int w = 40;
		int h = 17;
		int x = COLS/2 - w/2;
		int y = LINES/2 - h/2;
		win = new Info(x, y, w, h, "Info", 5);
		//win->SetResponseHandler([this](const std::string& msg) { SendResponse_(msg); });
		win->SetSendMessageFn([this](const std::vector<std::string>& msg) { OnCompMessage_(msg); });
		m_wins["Info"] = win;
	}
#endif
}

void Application::SetActiveWindow_(const std::string& name)
{
	auto it = m_wins.find(name);
	if (it != m_wins.end())
	{
		m_activeWin = it->second;
	}
	else
	{
		m_activeWin = nullptr;
	}
}

#if 0
std::string Application::LensProcessor(const std::string& data)
{
	std::string resp;

	m_lensData += data;
	auto pos = m_lensData.find('\n');
	while (pos != std::string::npos)
	{
		std::string line = m_lensData.substr(0, pos);
		m_lensData.erase(0, pos + 1);

		auto req = ProcessMessage_(line);

		for (auto it = m_lensWins.begin(); it != m_lensWins.end(); ++it)
		{
			if (*it != nullptr)
				// XXX: Does this need to append '\n'?
				resp += (*it)->ProcessRequest(req);
		}

		pos = m_lensData.find('\n');
	}

	return resp;
}
#endif

#if 0
std::string Application::McuProcessor(const std::string& data)
{
	std::string temp{data};
	boost::replace_all(temp, "\n", "\\n");
	LOG_DEBUG() << "Application::McuProcessor(\"" << temp << "\")";
	std::string resp;

	m_mcuData += data;
	auto pos = m_mcuData.find('\n');
	while (pos != std::string::npos)
	{
		std::string line = m_mcuData.substr(0, pos);
		m_mcuData.erase(0, pos + 1);

		auto req = ProcessMessage_(line);

		for (auto it = m_mcuWins.begin(); it != m_mcuWins.end(); ++it)
		{
			if (*it != nullptr)
				// XXX: Does this need to append '\n'?
				resp += (*it)->ProcessRequest(req);
		}

		pos = m_mcuData.find('\n');
	}

	return resp;
}
#endif

std::vector<std::string> Application::ProcessMessage_(const std::string& msg)
{
	std::istringstream ss(msg);
	std::vector<std::string> req;

	while (!ss.eof())
	{
		std::string word;
		ss >> word;
		if (!word.empty())
			req.push_back(word);
	}
	return req;
}

void Application::Resize_()
{
	// Updates the COLS and LINES (ncurses internal data) to have new values.
	resize_term(0, 0);

	// Allow windows a chance to resize.
	for (auto [name, win] : m_wins)
	{
		win->Resize();
	}

	// Update the screen with any resize changes.
	ShowStatusLine_();
	update_panels();
	doupdate();
}

void Application::ClearStatusLine_()
{
	if (m_statusX != -1)
	{
		attrset(COLOR_PAIR(0) | A_INVIS);
		attrset(COLOR_PAIR(0) | A_INVIS);
		mvprintw(m_statusX, 0, "%*.s", COLS, " ");
		attrset(0);
	}
	m_statusX = LINES - 1;
}

void Application::ShowStatusLine_()
{
	ClearStatusLine_();

	attron(COLOR_PAIR(4));
	mvprintw(LINES - 1, 0, "Type 'quit' to exit or 'help' for more information.");
	attroff(COLOR_PAIR(4));
}

void Application::AddSubscriptions_()
{
	m_mqtt.RegisterSub("/temperature-monitor/temperature", this);
	m_mqtt.RegisterSub("/heater/#", this);

#if 0
	m_notifier.Add(
		"/temperature-monitor/temperature",
		[this](const std::string& topic, const nlohmann::json& json) {
			OnMessage_(topic, json);
		});
#endif

#if 1
//	m_mqtt.RegisterSub("/heater/#", this);
#else
	m_notifier.Add(
		"/heater/#",
		[this](const std::string& topic, const nlohmann::json& json) {
			OnMessage_(topic, json);
		});
#endif
}

void Application::OnConnect(int rc)
{
	logger()->trace("Application::OnConnect(rc={})", rc);
}

void Application::OnDisconnect(int rc)
{
	logger()->trace("Application::OnDisconnect(rc={})", rc);
}

// Handles messages coming from components.
void Application::OnCompMessage_(const std::vector<std::string>& msg)
{
//	logger()->debug("Application::OnCompMessage_(msg={})", (!msg.empty() ? msg[0] : ""));
	if (msg.empty())
	{
		return;
	}

	if (msg[0].substr(0, 4) == "help")
	{
		SetActiveWindow_("Info");
	}
	if (msg[0].substr(0, 5) == "close")
	{
		SetActiveWindow_("Cmd");
//		m_activeWin = nullptr;
	}

	// Caution: This function can create an infinite loop and result in stack
	// exhaustion.
	SendCompMessage_(msg);
}

// Sends messages to components (i.e. ncurses windows).
void Application::SendCompMessage_(const std::vector<std::string>& msg)
{
	for (auto win : m_wins)
	{
		win.second->OnMessage(msg);
	}
}

// Handles messages coming from MQTT Broker.
void Application::OnMessage(const std::string& topic, const nlohmann::json& json)
{
	const std::string tempTopic {"/temperature-monitor/temperature"};
	const std::string heatTopic {"/heater/#"};

//	logger()->trace("Application::OnMessage(topic={}, json={})", topic, json.dump());
	if (m_mqtt.IsTopicMatch(tempTopic, topic))
	{
		// TODO: Verify json field exists
		SendCompMessage_({ "SetTemp", std::to_string(json["temperature"].get<float>())});
	}
	if (m_mqtt.IsTopicMatch(heatTopic, topic))
	{
//		SendCompMessage_({"SetHeater", json["enabled"], json["heater"]});
		SendCompMessage_({
			std::string("CmdHeater") + std::to_string(json["heater"].get<int>()),
			(json["enabled"] ? "on" : "off")
		});
	}
}
} // namespace ncc
