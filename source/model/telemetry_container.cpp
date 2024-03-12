//
// Created by Sidney on 03/06/2020.
//

#include <stdexcept>
#include "telemetry_container.h"

const char *telemetry_unit_to_string(telemetry_unit unit)
{
	switch(unit)
	{
		case telemetry_unit::value:
			return "Value";
		case telemetry_unit::fps:
			return "FPS";
		case telemetry_unit::time:
			return "Time";
		case telemetry_unit::memory:
			return "Memory";
		case telemetry_unit::duration:
			return "Duration";
	}

	return "";
}

const char *telemetry_event_type_to_string(telemetry_event_type event_type)
{
	switch(event_type)
	{
	case telemetry_event_type::begin:
		return "Begin";
	case telemetry_event_type::end:
		return "End";
	case telemetry_event_type::meta:
		return "Meta";
	}

	return "";
}

telemetry_provider_field &telemetry_provider::find_field(uint8_t id)
{
	for(auto &field : fields)
	{
		if(field.id == id)
			return field;
	}

	throw std::invalid_argument("Unknown field ID");
}
const telemetry_provider_field &telemetry_provider::find_field(uint8_t id) const
{
	for(auto &field : fields)
	{
		if(field.id == id)
			return field;
	}

	throw std::invalid_argument("Unknown field ID");
}

telemetry_provider &telemetry_container::find_provider(uint16_t runtime_id)
{
	for(auto &provider : providers)
	{
		if(provider.runtime_id == runtime_id)
			return provider;
	}

	throw std::invalid_argument("Unknown provider ID");
}
const telemetry_provider &telemetry_container::find_provider(uint16_t runtime_id) const
{
	for(auto &provider : providers)
	{
		if(provider.runtime_id == runtime_id)
			return provider;
	}

	throw std::invalid_argument("Unknown provider ID");
}

const telemetry_provider &telemetry_container::find_provider(const QString &identifier) const
{
	for(auto &provider : providers)
	{
		if(provider.identifier == identifier)
			return provider;
	}

	throw std::invalid_argument("Unknown provider identifier");
}

QVector<telemetry_data_point> telemetry_provider_field::get_data_points_in_range(int32_t start, int32_t end) const
{
	QVector<telemetry_data_point> result;

	for(auto &data : data_points)
	{
		if(data.timestamp >= start && data.timestamp <= end)
			result.push_back(data);
	}

	return result;
}
telemetry_data_point telemetry_provider_field::get_data_point_after_time(int32_t time) const
{
	for(int i = 0; i < data_points.size(); ++ i)
	{
		auto &data = data_points[i];

		if(data.timestamp < time)
			continue;

		if(i > 0)
			return data_points[i - 1];

		return data;
	}

	throw std::invalid_argument("No data point after time");
}
