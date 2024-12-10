//
// Created by Sidney on 09/12/2024.
//

#ifndef TELEMETRY_STATISTIC_H
#define TELEMETRY_STATISTIC_H

#include <string>
#include <vector>
#include "data.h"

struct telemetry_statistic_entry
{
	std::string title;
	telemetry_data_value value;
};

class telemetry_statistic
{
public:
	telemetry_statistic(std::string title);

	const std::string &get_title() const { return m_title; }

	const std::vector<telemetry_statistic_entry> &get_entries() const { return m_entries; }
	std::vector<telemetry_statistic_entry> &get_entries() { return m_entries; }

	void add_entry(telemetry_statistic_entry &&entry);

private:
	std::string m_title;

	std::vector<telemetry_statistic_entry> m_entries;
};

#endif //TELEMETRY_STATISTIC_H
