//
// Created by Sidney on 09/12/2024.
//

#ifndef TELEMETRY_CONTAINER_H
#define TELEMETRY_CONTAINER_H

#include <vector>
#include <string>

#include "data.h"
#include "provider.h"
#include "event.h"
#include "statistic.h"

class telemetry_container
{
public:
	telemetry_container() = default;
	telemetry_container(uint32_t version);

	bool has_provider(uint16_t id) const;
	bool has_provider(const std::string &identifier) const;

	const telemetry_provider &get_provider(uint16_t id) const; // Will throw std::out_of_range() error
	const telemetry_provider &get_provider(const std::string &identifier) const; // Will throw std::out_of_range() error

	telemetry_provider &get_provider(uint16_t id); // Will throw std::out_of_range() error
	telemetry_provider &get_provider(const std::string &identifier); // Will throw std::out_of_range() error

	const std::vector<telemetry_provider> &get_providers() const { return m_providers; }
	std::vector<telemetry_provider> &get_providers() { return m_providers; }

	const telemetry_event &get_event(uint64_t id) const;
	telemetry_event &get_event(uint64_t id);

	const std::vector<telemetry_event> &get_events() const { return m_events; }
	std::vector<telemetry_event> &get_events() { return m_events; }

	const std::vector<telemetry_statistic> &get_statistics() const { return m_statistics; }
	std::vector<telemetry_statistic> &get_statistics() { return m_statistics; }

	int32_t get_start_time() const { return m_start_time; }
	int32_t get_end_time() const { return m_end_time; }

	void set_start_time(int32_t start_time);
	void set_end_time(int32_t end_time);

	void add_provider(telemetry_provider &&provider);
	void add_event(telemetry_event &&event);
	void add_statistic(telemetry_statistic &&statistic);

private:
	uint32_t m_version = 0;

	int32_t m_start_time = 0;
	int32_t m_end_time = 0;

	std::vector<telemetry_provider> m_providers;
	std::vector<telemetry_event> m_events;
	std::vector<telemetry_statistic> m_statistics;
};

#endif //TELEMETRY_CONTAINER_H
