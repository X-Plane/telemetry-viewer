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
QPair<telemetry_data_point, telemetry_data_point> telemetry_provider_field::get_min_max_data_point_in_range(int32_t start, int32_t end) const
{
	telemetry_data_point min_value;
	min_value.timestamp = -1;

	telemetry_data_point max_value;
	max_value.timestamp = -1;

	for(auto &data : data_points)
	{
		if(data.timestamp < start)
			continue;
		if(data.timestamp > end)
			break;

		switch(type)
		{
			case telemetry_type::boolean:
			{
				if(min_value.timestamp < 0 || data.value.toBool() < min_value.value.toBool())
					min_value = data;
				if(max_value.timestamp < 0 || data.value.toBool() > max_value.value.toBool())
					max_value = data;

				break;
			}

			case telemetry_type::uint8:
			case telemetry_type::uint16:
			case telemetry_type::uint32:
			case telemetry_type::uint64:
			{
				if(min_value.timestamp < 0 || data.value.toULongLong() < min_value.value.toULongLong())
					min_value = data;
				if(max_value.timestamp < 0 || data.value.toULongLong() > max_value.value.toULongLong())
					max_value = data;

				break;
			}

			case telemetry_type::int32:
			case telemetry_type::int64:
			{
				if(min_value.timestamp < 0 || data.value.toLongLong() < min_value.value.toLongLong())
					min_value = data;
				if(max_value.timestamp < 0 || data.value.toLongLong() > max_value.value.toLongLong())
					max_value = data;

				break;
			}

			case telemetry_type::floatv:
			case telemetry_type::doubv:
			{
				if(min_value.timestamp < 0 || data.value.toDouble() < min_value.value.toDouble())
					min_value = data;
				if(max_value.timestamp < 0 || data.value.toDouble() > max_value.value.toDouble())
					max_value = data;

				break;
			}
		}
	}

	return qMakePair(min_value, max_value);
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
