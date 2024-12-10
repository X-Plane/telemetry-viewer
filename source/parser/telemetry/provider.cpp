//
//  provider.cpp
//  libtlm
//
//  Created by Sidney Just
//  Copyright (c) 2024 by Laminar Research
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//  documentation files (the "Software"), to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
//  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
//  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <stdexcept>
#include <cmath>
#include "provider.h"

telemetry_field::telemetry_field(uint8_t id, uint16_t provider, std::string title, telemetry_type type, telemetry_unit unit) :
	m_id(id),
	m_provider(provider),
	m_title(title),
	m_type(type),
	m_unit(unit)
{}

telemetry_data_point telemetry_field::get_data_point_closest_to_time(int32_t time) const
{
	for(size_t i = 0; i < m_data_points.size(); ++ i)
	{
		auto &data = m_data_points[i];

		if(data.timestamp >= time)
		{
			if(i > 0)
			{
				auto &previous = m_data_points[i - 1];

				if(std::fabs(data.timestamp - time) > std::fabs(previous.timestamp - time))
					return previous;
			}

			return m_data_points[i];
		}
	}

	throw std::invalid_argument("No data point before or at time");
}

telemetry_data_point telemetry_field::get_data_point_after_time(int32_t time) const
{
	for(size_t i = 0; i < m_data_points.size(); ++ i)
	{
		auto &data = m_data_points[i];

		if(data.timestamp < time)
			continue;

		if(i > 0)
			return m_data_points[i - 1];

		return data;
	}

	throw std::invalid_argument("No data point after time");
}

std::vector<telemetry_data_point> telemetry_field::get_data_points_in_range(int32_t start, int32_t end) const
{
	std::vector<telemetry_data_point> result;

	for(auto &data : m_data_points)
	{
		if(data.timestamp < start)
			continue;
		if(data.timestamp > end)
			break;

		result.push_back(data);
	}

	return result;
}

std::pair<telemetry_data_point, telemetry_data_point> telemetry_field::get_extreme_data_point_in_range(int32_t start, int32_t end) const
{
	telemetry_data_point min_value;
	telemetry_data_point max_value;

	switch(m_type)
	{
		case telemetry_type::string:
			throw std::invalid_argument("Can't get min/max data points for string telemetry types");
		case telemetry_type::vec2:
			throw std::invalid_argument("Can't get min/max data points for vec2 telemetry types");
		case telemetry_type::dvec2:
			throw std::invalid_argument("Can't get min/max data points for dvec2 telemetry types");

		default:
			break;
	}

	bool found_data_point = false;

	for(auto &data : m_data_points)
	{
		if(data.timestamp < start)
			continue;
		if(data.timestamp > end)
			break;

		if(!found_data_point)
		{
			found_data_point = true;

			min_value = data;
			max_value = data;

			continue;
		}

		switch(m_type)
		{
			// These values are already handled higher up, this is just to shut up any linters
			case telemetry_type::string:
			case telemetry_type::vec2:
			case telemetry_type::dvec2:
				break;

			case telemetry_type::boolean:
			{
				if(data.value.b < min_value.value.b)
					min_value = data;
				if(data.value.b > max_value.value.b)
					max_value = data;

				break;
			}


			case telemetry_type::uint8:
			{
				if(data.value.u8 < min_value.value.u8)
					min_value = data;
				if(data.value.u8 > max_value.value.u8)
					max_value = data;

				break;
			}
			case telemetry_type::uint16:
			{
				if(data.value.u16 < min_value.value.u16)
					min_value = data;
				if(data.value.u16 > max_value.value.u16)
					max_value = data;

				break;
			}
			case telemetry_type::uint32:
			{
				if(data.value.u32 < min_value.value.u32)
					min_value = data;
				if(data.value.u32 > max_value.value.u32)
					max_value = data;

				break;
			}
			case telemetry_type::uint64:
			{
				if(data.value.u64 < min_value.value.u64)
					min_value = data;
				if(data.value.u64 > max_value.value.u64)
					max_value = data;

				break;
			}


			case telemetry_type::int32:
			{
				if(data.value.i32 < min_value.value.i32)
					min_value = data;
				if(data.value.i32 > max_value.value.i32)
					max_value = data;

				break;
			}
			case telemetry_type::int64:
			{
				if(data.value.i64 < min_value.value.i64)
					min_value = data;
				if(data.value.i64 > max_value.value.i64)
					max_value = data;

				break;
			}

			case telemetry_type::f32:
			{
				if(data.value.f32 < min_value.value.f32)
					min_value = data;
				if(data.value.f32 > max_value.value.f32)
					max_value = data;

				break;
			}
			case telemetry_type::f64:
			{
				if(data.value.f64 < min_value.value.f64)
					min_value = data;
				if(data.value.f64 > max_value.value.f64)
					max_value = data;

				break;
			}
		}
	}

	if(!found_data_point)
		return std::make_pair(get_data_point_closest_to_time(start), get_data_point_closest_to_time(start));

	return std::make_pair(min_value, max_value);
}

void telemetry_field::set_data_points(std::vector<telemetry_data_point> &&data_points)
{
	m_data_points = std::move(data_points);
}
void telemetry_field::add_data_point(telemetry_data_point &&data)
{
	m_data_points.push_back(std::move(data));
}




telemetry_provider::telemetry_provider(uint16_t id, uint16_t version, const std::string &identifier, const std::string &title) :
	m_id(id),
	m_version(version),
	m_identifier(identifier),
	m_title(title)
{}

void telemetry_provider::add_field(telemetry_field &&field)
{
	m_fields.push_back(std::move(field));
}



bool telemetry_provider::has_field(uint8_t id) const
{
	for(auto &field : m_fields)
	{
		if(field.get_id() == id)
			return true;
	}

	return false;
}

const telemetry_field &telemetry_provider::get_field(uint8_t id) const
{
	for(auto &field : m_fields)
	{
		if(field.get_id() == id)
			return field;
	}

	throw std::out_of_range("No field with id " + std::to_string(id));
}

telemetry_field &telemetry_provider::get_field(uint8_t id)
{
	for(auto &field : m_fields)
	{
		if(field.get_id() == id)
			return field;
	}

	throw std::out_of_range("No field with id " + std::to_string(id));
}
