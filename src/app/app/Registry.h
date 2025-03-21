#pragma once

#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>

/*
 * This class is used to display *live* information on the Info (Help) panel.
 * Each panel will update the information periodically which will be
 * updated on the Info panel.
 *
 * Notes:
 * - Could this done using PUB/SUB?
 */

namespace ncc
{

class Registry
{
private:
	struct Entry
	{
		std::string name;
		std::string keys;
		std::string desc;
		std::string value;
		size_t maxValueLen;

		Entry() = default;
		Entry(
				const std::string& name,
				const std::string& keys,
				const std::string& desc,
				const std::string& value,
				size_t maxValueLen)
			: name(name)
			, keys(keys)
			, desc(desc)
			, value(value)
			, maxValueLen(maxValueLen)
		{}

		Entry(const Entry& other)
			: name(other.name)
			, keys(other.keys)
			, desc(other.desc)
			, value(other.value)
			, maxValueLen(other.maxValueLen)
		{
		}

		Entry& operator=(const Entry& other)
		{
			if (&other != this)
			{
				name = other.name;
				keys = other.keys;
				desc = other.desc;
				value = other.value;
				maxValueLen = other.maxValueLen;
			}
			return *this;
		}
	};

public:
	void Add(
		const std::string& name,
		const std::string& keys,
		const std::string& desc,
		const std::string& value,
		size_t maxValueLen)
	{
		auto it = Find_(name);
		if (it == m_entries.end())
			m_entries.push_back(Entry(name, keys, desc, value, maxValueLen));
	}

	void Update(const std::string& name, const std::string& value)
	{
		auto it = Find_(name);
		if (it != m_entries.end())
			it->value = value;
	}

	bool GetLine(size_t num, std::string& line)
	{
		auto it = m_entries.begin();
		while (it != m_entries.end())
		{
			if (num == 0)
			{
				std::ostringstream oss;
				oss << std::left << std::setw(m_colWidth[0]) << it->keys.substr(0, m_colWidth[0])
					<< " "
					<< std::setw(m_colWidth[1]) << it->desc.substr(0, m_colWidth[1])
					<< " "
					<< std::right << std::setw(m_colWidth[2]) << it->value.substr(0, m_colWidth[2]);
				line = oss.str();
				return true;
			}
			--num;
			++it;
		}
		return false;
	}

private:
	std::vector<Entry>::iterator Find_(const std::string& name)
	{
		auto it = std::find_if(m_entries.begin(), m_entries.end(), [name](const Entry& entry) {
			return entry.name == name;
		});
		return it;
	}

private:
	std::vector<Entry> m_entries;
	std::vector<size_t> m_colWidth = { 4, 20, 10 };
};

extern Registry registry;

} // namespace ncc
