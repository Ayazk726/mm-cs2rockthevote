#ifndef _INCLUDE_RTV_MAP_LIST_LINE_H_
#define _INCLUDE_RTV_MAP_LIST_LINE_H_

#include "utils/maplist.h"
#include "utils/str.h"

#include <algorithm>
#include <cctype>
#include <string>

// One parsed line of the local maplist.txt. A line has the usual name / workshop
// id fields and may carry the CS2KZ tiers the website's map list export uses:
//
//   kz_grotto                                  stock map
//   kz_grotto:3121168339                       workshop map
//   kz_grotto:3121168339:2                     classic (CKZ) tier 2
//   kz_grotto:3121168339:2(3)                  classic 2, vanilla (VNL) 3
//
// Tiers are 1-10. A missing or out-of-range value leaves the mode's tier at 0;
// DisplayKzTiers then asks the CS2KZ API, and shows nothing only if it has no tier
// for the map either.
struct MapListLine
{
	mmu::MapListEntry map;
	int classicTier = 0; // CKZ tier, 0 = unknown
	int vanillaTier = 0; // VNL tier, 0 = unknown
};

namespace maplist_line
{
	// CS2KZ's nub_tier range.
	constexpr int kMaxTier = 10;

	inline bool IsAllDigits(const std::string &s)
	{
		return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
	}

	// "5" or "5(3)" -> classic 5, vanilla 3 or 0. The value is 0 when out of range.
	// Returns false when `field` is not tier-shaped at all.
	inline bool ParseTierField(const std::string &field, int &classicTier, int &vanillaTier)
	{
		classicTier = 0;
		vanillaTier = 0;
		// "10(10)" is the longest a tier field can be.
		if (field.empty() || field.size() > 6)
		{
			return false;
		}

		const size_t open = field.find('(');
		const std::string classic = open == std::string::npos ? field : str::Trim(field.substr(0, open));
		std::string vanilla;
		if (open != std::string::npos)
		{
			// The vanilla part is "(digits)" and nothing else.
			if (field.back() != ')' || open + 2 > field.size() - 1)
			{
				return false;
			}
			vanilla = str::Trim(field.substr(open + 1, field.size() - open - 2));
		}

		// A tier is one or two digits, so "3287874815" stays a workshop id.
		auto tierNumber = [](const std::string &digits) -> int
		{
			if (digits.empty() || !IsAllDigits(digits) || digits.size() > 2)
			{
				return -1;
			}
			const int value = (digits.size() == 2 ? (digits[0] - '0') * 10 : 0) + (digits.back() - '0');
			return value >= 1 && value <= kMaxTier ? value : 0;
		};

		const int classicValue = tierNumber(classic);
		if (classicValue < 0)
		{
			return false;
		}
		int vanillaValue = 0;
		if (open != std::string::npos)
		{
			vanillaValue = tierNumber(vanilla);
			if (vanillaValue < 0)
			{
				return false;
			}
		}

		classicTier = classicValue;
		vanillaTier = vanillaValue;
		return true;
	}

	// Parse one maplist.txt line. Returns false for blanks and comments, as
	// mmu::ParseMapListLine does.
	inline bool ParseLine(const std::string &rawLine, MapListLine &out)
	{
		out = MapListLine();

		std::string line = str::Trim(rawLine);
		if (line.empty() || line[0] == '#' || line[0] == '/' || line[0] == ';')
		{
			return false;
		}

		// The tier field is last, after the workshop id: "name:workshopid:ckz(vnl)".
		const size_t lastColon = line.rfind(':');
		if (lastColon != std::string::npos)
		{
			int classicTier = 0;
			int vanillaTier = 0;
			const std::string tail = str::Trim(line.substr(lastColon + 1));
			if (ParseTierField(tail, classicTier, vanillaTier))
			{
				const std::string rest = str::Trim(line.substr(0, lastColon));
				const size_t idColon = rest.rfind(':');
				const bool afterWorkshopId = idColon != std::string::npos && IsAllDigits(str::Trim(rest.substr(idColon + 1)));
				// Without a workshop id before it, "kz_grotto:2" keeps meaning a
				// workshop id. The "(n)" part alone is unambiguous.
				if (afterWorkshopId || tail.find('(') != std::string::npos)
				{
					out.classicTier = classicTier;
					out.vanillaTier = vanillaTier;
					line = rest;
				}
			}
		}

		return mmu::ParseMapListLine(line, out.map);
	}
} // namespace maplist_line

#endif // _INCLUDE_RTV_MAP_LIST_LINE_H_
