//
// Created by Sidney on 09/12/2024.
//

#include "statistic.h"

telemetry_statistic::telemetry_statistic(std::string title) :
	m_title(title)
{}

void telemetry_statistic::add_entry(telemetry_statistic_entry &&entry)
{
	m_entries.push_back(std::move(entry));
}
