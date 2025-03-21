#include <core/Utils.h>

#include <sstream>

namespace ncc
{

std::string Join(const std::vector<std::string>& v)
{
	// TODO: Figure out how to use STL

	std::ostringstream oss;
	const std::string delim {" "};
	bool useDelim {false};
	for (auto s : v)
	{
		if (useDelim) oss << delim; else useDelim = true;
		oss << s;
	}
	return oss.str();
}

} // namespace ncc
