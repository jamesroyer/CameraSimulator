#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <functional>

#define NCURSES_NOMACROS
#include <panel.h>

namespace ncc // Ncurses Camera
{

int WrapAround(int v, int delta, int minval, int maxval);

template <typename T>
T LimitRange(T v, T delta, T minval, T maxval)
{
	if (delta > 0 && (v + delta) >= v)
	{
		v = std::min(v + delta, maxval);
	}
	else if (delta < 0 && (v + delta) <= v)
	{
		v = std::max(v + delta, minval);
	}
	return v;
}

/*
 * Ideas:
 * - Grey out components if they are disabled.
 * - Support layout management.
 *   - Min width/height
 *   - Recreate window when terminal is resized
 */
class Base
{
public:
//	using ResponseFn = std::function<void(const std::string& data)>;
	using SendMessageFn = std::function<void(const std::vector<std::string>&)>;

	virtual ~Base();
	Base(
		int x,
		int y,
		int w,
		int h,
		const std::string& label,
		int labelColor,
		bool useLabelBox = true,
		int bgColor = -1,
		int fgColor = -1);
	void Init();
	void Resize();
	void Draw();
	void CenterText(int x, int y, int w, const std::string& s, chtype color);
	void PrintText(int x, int y, const std::string& s, chtype color);
	bool IsHidden() const;
	void Show();
	void Hide();
	void ToggleShowHide();

	void UpdateInfo(uint32_t secs, uint32_t usecs);
	bool HandleInput(int ch);

	void Set(int value);
	int Get() const;

	std::string OnMessage(const std::vector<std::string>& req);
	void SetSendMessageFn(SendMessageFn fn);

protected:
	void RecreateWindow_();
	virtual void UpdateInfo_(uint32_t secs, uint32_t usecs);
	virtual bool HandleInput_(int ch);
	virtual std::string OnMessage_(const std::vector<std::string>& req);
	virtual void Resize_();
	void ClearStatus_(int y);
	void ShowMessage_(const std::string& msg);

	void SendMessage_(const std::vector<std::string>& msg);

protected:
	WINDOW* m_win = nullptr;
	PANEL*  m_panel = nullptr;
	const std::string m_label;
	const int m_labelColor;
	const int m_bgColor {-1};
	const int m_fgColor {-1};
	int m_x {0};
	int m_y {0};
	int m_w {0};
	int m_h {0};
	bool m_supportsResize {false};
	bool m_useLabelBox {true};

private:
	bool m_hidden {false};
	int m_value {0};
	SendMessageFn m_sendMsgFn;
};

} // namespace ncc
