#pragma once

#include <app/Base.h>

#include <string>
#include <functional>
#include <map>

#define NCURSES_NOMACROS
#include <panel.h>

namespace ncc
{

// +---------------------+
// |5  130  135  140  145|
// |....|....|....|....|.|
// +---------------------+
class Compass : public Base
{
public:
	~Compass() override = default;
	Compass(
		int x,
		int y,
		int w,
		int h,
		const std::string& label,
		int labelColor,
		const std::string& keys,
		bool neg = false);

	void MoveTo(int pos);
	void MoveBy(int value);
	void MoveCont();
	int GetPosition() const;

private:
	std::string GenerateRepeatingPattern_(const std::string pattern, size_t width, size_t offset = 0);
	void Draw_();
	void DrawPos_();
	void DrawNeg_();
	void SetMoving_(bool value);
	void SetDirection_(int value);
	void Advance_();
	void AdvancePos_();
	void AdvanceNeg_();

	std::string OnMessage_(const std::vector<std::string>& msg) override;
	bool HandleInput_(int ch) override;
	void UpdateInfo_(uint32_t secs, uint32_t usecs) override;

private:
	const std::string m_label;
	const std::string m_ticks = "....|";
	size_t m_tickOffset = 0;
	int m_pos = 0;

	enum class movement_t { idle, movecont, moveto };

	movement_t m_movement = movement_t::idle;
	int m_direction = 1; 		// -1 => backward, 1 => forward
	int m_desiredPos = 0;		// 0-359
	bool m_stepMove = false;	// Override m_moving for one step.

	std::map<int, std::function<void()>> m_handlers;
	std::pair<int, int> m_range = { 0, 0 };
};

} // namespace ncc
