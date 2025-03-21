#pragma once

#include <string>

#define NCURSES_NOMACROS
#include <panel.h>

#include <app/Base.h>

namespace ncc
{

/*
 * This "miscellaneous" panel is used to display:
 *   - Heater 1
 *   - Heater 2
 *   - PoE
 *   - Temperature
 */
class McuMisc : public Base
{
public:
	~McuMisc() override = default;

	McuMisc(int x, int y, int w, int h, const std::string& label, int labelColor);

private:
	void UpdateInfo_(uint32_t secs, uint32_t usecs) override;
	bool HandleInput_(int ch) override;
	std::string OnMessage_(const std::vector<std::string>& req) override;
	void EnableHeater_(int num);
	void DisableHeater_(int num);
	void Draw_();
	void DrawHeater_(int num);
	void DrawPoE_();
	void DrawTempInC_();

private:
	bool m_heater[2] = { false, false };
	uint32_t m_lastUpdateSec = 0;
	int m_poeLevel = 0; // 0:PoE, 1:PoE+, 2:PoE++
	float m_tempInC = -40.0f;
	float m_operatingTempInC = 12.0f;
	float m_tempRange[2] = { -40.0f, 40.0f };
};

} // namespace ncc
