//
// Created by Sidney on 09/12/2024.
//

#ifndef TELEMETRY_PROVIDER_H
#define TELEMETRY_PROVIDER_H

#include <vector>
#include <string>
#include "data.h"

class telemetry_field
{
public:
	telemetry_field(uint8_t id, uint16_t provider, std::string title, telemetry_type type, telemetry_unit unit);

	uint8_t get_id() const { return m_id; }
	uint16_t get_provider_id() const { return m_provider; }
	const std::string &get_title() const { return m_title; }

	telemetry_type get_type() const { return m_type; }
	telemetry_unit get_unit() const { return m_unit; }

	bool empty() const { return m_data_points.empty(); }
	const std::vector<telemetry_data_point> &get_data_points() const { return m_data_points; }

	telemetry_data_point get_data_point_closest_to_time(int32_t time) const;
	telemetry_data_point get_data_point_after_time(int32_t time) const;

	std::vector<telemetry_data_point> get_data_points_in_range(int32_t start, int32_t end) const;
	std::pair<telemetry_data_point, telemetry_data_point> get_extreme_data_point_in_range(int32_t start, int32_t end) const;

	void set_data_points(std::vector<telemetry_data_point> &&data_points);
	void add_data_point(telemetry_data_point &&data);

private:
	uint8_t m_id;
	uint16_t m_provider;
	std::string m_title;

	telemetry_type m_type;
	telemetry_unit m_unit;

	std::vector<telemetry_data_point> m_data_points;
};

class telemetry_provider
{
public:
	telemetry_provider(uint16_t id, uint16_t version, const std::string &identifier, const std::string &title);

	uint16_t get_id() const { return m_id; }
	uint16_t get_version() const { return m_version; }

	const std::string &get_identifier() const { return m_identifier; }
	const std::string &get_title() const { return m_title; }

	bool has_field(uint8_t id) const;
	const telemetry_field &get_field(uint8_t id) const;
	telemetry_field &get_field(uint8_t id);

	const std::vector<telemetry_field> &get_fields() const { return m_fields; }
	std::vector<telemetry_field> &get_fields() { return m_fields; }

	void add_field(telemetry_field &&field);

private:
	uint16_t m_id;
	uint16_t m_version;

	std::string m_identifier;
	std::string m_title;

	std::vector<telemetry_field> m_fields;
};

#endif //TELEMETRY_PROVIDER_H
