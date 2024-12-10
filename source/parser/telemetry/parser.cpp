//
//  container.cpp
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

#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>
#include "parser.h"

struct file_reader
{
	file_reader(const uint8_t *data, size_t size) :
		m_data(data),
		m_start(data),
		m_end(data + size)
	{}

	file_reader(const uint8_t *data, size_t offset, size_t size) :
		m_data(data + offset),
		m_start(data),
		m_end(data + size)
	{}

	size_t get_read() const { return m_data - m_start; }
	size_t get_remaining() const { return m_end - m_data; }

	bool at_end() const { return m_data >= m_end; }

	void skip(size_t bytes)
	{
		m_data += bytes;
	}


	bool read_bool()
	{
		return *m_data ++;
	}

	uint8_t read_uint8()
	{
		return *m_data ++;
	}

	uint16_t read_uint16()
	{
		uint16_t data[2];

		data[0] = *m_data ++;
		data[1] = *m_data ++;

		return (data[1] << 8) | data[0];
	}
	int16_t read_int16()
	{
		int16_t data[2];

		data[0] = *m_data ++;
		data[1] = *m_data ++;

		return (data[1] << 8) | data[0];
	}

	uint32_t read_uint32()
	{
		uint32_t data[4];

		for(int i = 0; i < 4; i++)
			data[i] = *m_data ++;

		return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
	}
	int32_t read_int32()
	{
		int32_t data[4];

		for(int i = 0; i < 4; i++)
			data[i] = *m_data ++;

		return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
	}

	uint64_t read_uint64()
	{
		uint64_t data[8];

		for(int i = 0; i < 8; i++)
			data[i] = *m_data ++;

		return (data[7] << 56) | (data[6] << 48) | (data[5] << 40) | (data[4] << 32) | (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
	}
	int64_t read_int64()
	{
		int64_t data[8];

		for(int i = 0; i < 8; i++)
			data[i] = *m_data ++;

		return (data[7] << 56) | (data[6] << 48) | (data[5] << 40) | (data[4] << 32) | (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
	}

	float read_float()
	{
		union
		{
			uint8_t ival[4];
			float fval;
		} map;

		for(int i = 0; i < 4; i++)
			map.ival[i] = *m_data ++;

		if(std::isinf(map.fval) || std::isnan(map.fval))
			return 0.0f;

		return map.fval;
	}

	double read_double()
	{
		union
		{
			uint8_t ival[8];
			double fval;
		} map;

		for(int i = 0; i < 8; i++)
			map.ival[i] = *m_data ++;

		if(std::isinf(map.fval) || std::isnan(map.fval))
			return 0.0f;

		return map.fval;
	}

	std::string read_string()
	{
		const uint8_t length = *m_data ++;

		std::string result(length, '\0');

		for(uint8_t i = 0; i < length; i ++)
			result[i] = *m_data ++;

		return result;
	}

	telemetry_data_value read_value(telemetry_type type)
	{
		telemetry_data_value value;
		value.type = type;

		switch(type)
		{
			case telemetry_type::boolean:
			{
				value.b = read_bool();
				break;
			}

			case telemetry_type::uint8:
			{
				value.u8 = read_uint8();
				break;
			}
			case telemetry_type::uint16:
			{
				value.u16 = read_uint16();
				break;
			}
			case telemetry_type::uint32:
			{
				value.u32 = read_uint32();
				break;
			}
			case telemetry_type::uint64:
			{
				value.u64 = read_uint64();
				break;
			}


			case telemetry_type::int32:
			{
				value.i32 = read_int32();
				break;
			}
			case telemetry_type::int64:
			{
				value.i64 = read_int64();
				break;
			}

			case telemetry_type::f32:
			{
				value.f32 = read_float();
				break;
			}
			case telemetry_type::f64:
			{
				value.f64 = read_double();
				break;
			}

			case telemetry_type::vec2:
			{
				value.vec2[0] = read_float();
				value.vec2[1] = read_float();
				break;
			}
			case telemetry_type::dvec2:
			{
				value.dvec2[0] = read_double();
				value.dvec2[1] = read_double();
				break;
			}

			case telemetry_type::string:
			{
				value.string = read_string();
				break;
			}
		}

		return value;
	}

private:
	const uint8_t *m_data;
	const uint8_t *m_start;
	const uint8_t *m_end;
};



enum class telemetry_v2_command : uint8_t
{
	register_provider,
	packet,
	statistic,
	amend_provider,
	event
};

enum class telemetry_event_type : uint8_t
{
	begin = 'b',
	end = 'e',
	meta = 'm',
};

struct telemetry_event_temporary
{
	uint64_t id;
	uint64_t parent;
	double start_time;
	double end_time;
	std::vector<telemetry_event_entry> entries;
};

void finalize_container(telemetry_container &container, const telemetry_parser_options &options)
{
	{
		// Figure out the start and end time range

		double start_time = 0.0;
		double end_time = 0.0;

		for(auto &provider : container.get_providers())
		{
			for(auto &field : provider.get_fields())
			{
				if(field.empty())
					continue;

				start_time = std::min(start_time, field.get_data_points().front().timestamp);
				end_time = std::max(end_time, field.get_data_points().back().timestamp);
			}
		}

		container.set_start_time(std::floor(start_time));
		container.set_end_time(std::ceil(end_time));
	}

	if(options.data_point_processor)
	{
		// Run the data processor and do a final update of the start and end time, in case the processor nukes data points away
		double start_time = 0.0;
		double end_time = 0.0;

		for(auto &provider : container.get_providers())
		{
			for(auto &field : provider.get_fields())
			{
				if(field.empty())
					continue;

				std::vector<telemetry_data_point> data = options.data_point_processor(container, provider, field, field.get_data_points());

				if(!data.empty())
				{
					start_time = std::min(start_time, data.front().timestamp);
					end_time = std::max(end_time, data.back().timestamp);
				}

				field.set_data_points(std::move(data));
			}
		}

		container.set_start_time(std::floor(start_time));
		container.set_end_time(std::ceil(end_time));
	}
}

telemetry_container parser_tlmv2_data(file_reader &reader, const telemetry_parser_options &options)
{
	telemetry_container container(2);

	std::unordered_map<uint64_t, telemetry_event_temporary> events;

	auto get_event_data = [&](uint64_t id) -> telemetry_event_temporary & {

		auto iterator = events.find(id);
		if(iterator != events.end())
			return iterator->second;

		telemetry_event_temporary event;
		event.id = id;
		event.start_time = event.end_time = 0.0;
		event.parent = UINT64_MAX;

		return events.insert(std::make_pair(id, event)).first->second;

	};

	while(!reader.at_end())
	{
		telemetry_v2_command command = (telemetry_v2_command)reader.read_uint8();

		switch(command)
		{
			case telemetry_v2_command::register_provider:
			{
				const std::string identifier = reader.read_string();
				const std::string title = reader.read_string();
				const uint16_t version = reader.read_uint16();
				const uint16_t id = reader.read_uint16();

				telemetry_provider provider(id, version, identifier, title);

				const uint32_t num_fields = reader.read_uint32();
				for(uint32_t i = 0; i < num_fields; i ++)
				{
					const uint8_t id = reader.read_uint8();
					const telemetry_type type = static_cast<telemetry_type>(reader.read_uint8());
					const telemetry_unit unit = static_cast<telemetry_unit>(reader.read_uint8());
					const std::string title = reader.read_string();

					telemetry_field field(id, provider.get_id(), title, type, unit);
					provider.add_field(std::move(field));
				}

				container.add_provider(std::move(provider));
				break;
			}

			case telemetry_v2_command::amend_provider:
			{
				const uint16_t runtime_id = reader.read_uint16();
				telemetry_provider &provider = container.get_provider(runtime_id);

				const uint32_t fields = reader.read_uint32();

				for(uint32_t i = 0; i < fields; ++ i)
				{
					const uint8_t id = reader.read_uint8();
					const telemetry_type type = static_cast<telemetry_type>(reader.read_uint8());
					const telemetry_unit unit = static_cast<telemetry_unit>(reader.read_uint8());
					const std::string title = reader.read_string();

					if(!provider.has_field(id))
					{
						telemetry_field field(id, provider.get_id(), title, type, unit);
						provider.add_field(std::move(field));
					}
				}

				break;
			}

			case telemetry_v2_command::statistic:
			{
				const std::string title = reader.read_string();

				telemetry_statistic statistic(title);

				const size_t length = reader.read_uint32();
				const size_t read_position = reader.get_read();

				while(reader.get_read() - read_position < length)
				{
					const telemetry_type type = static_cast<telemetry_type>(reader.read_uint8());

					telemetry_statistic_entry entry;
					entry.title = reader.read_string();
					entry.value = reader.read_value(type);

					statistic.add_entry(std::move(entry));
				}

				container.add_statistic(std::move(statistic));
				break;
			}
			case telemetry_v2_command::event:
			{
				const uint64_t id = reader.read_uint64();
				const double timestamp = reader.read_double();
				const telemetry_event_type event_type = static_cast<telemetry_event_type>(reader.read_uint8());

				auto &event = get_event_data(id);

				switch(event_type)
				{
					case telemetry_event_type::begin:
						event.start_time = timestamp;
						break;
					case telemetry_event_type::end:
						event.end_time = timestamp;
						break;
				}

				const size_t length = reader.read_uint32();
				const size_t read_position = reader.get_read();

				while(reader.get_read() - read_position < length)
				{
					const telemetry_type type = static_cast<telemetry_type>(reader.read_uint8());

					telemetry_event_entry entry;
					entry.title = reader.read_string();
					entry.value = reader.read_value(type);

					// Weird in-band signalling
					if(entry.title == "parent")
					{
						event.parent = entry.value.get<uint64_t>();
						continue;
					}

					event.entries.push_back(std::move(entry));
				}

				break;
			}

			case telemetry_v2_command::packet:
			{
				const uint16_t runtime_id = reader.read_uint16();
				const uint32_t count = reader.read_uint32();

				telemetry_provider &provider = container.get_provider(runtime_id);

				for(uint32_t i = 0; i < count; ++ i)
				{
					const double timestamp = reader.read_double();

					const size_t length = reader.read_uint32();
					const size_t read = reader.get_read();

					while((reader.get_read() - read) < length)
					{
						const uint8_t id = reader.read_uint8();
						telemetry_field &field = provider.get_field(id);

						telemetry_data_point data_point;
						data_point.timestamp = timestamp;
						data_point.value = reader.read_value(field.get_type());

						field.add_data_point(std::move(data_point));
					}
				}

				break;
			}
		}
	}

	if(!events.empty())
	{
		std::vector<telemetry_event_temporary> all_events;

		// First flatten out the list of events and then sort it by id
		// because lower IDs can't be parents to higher IDs, this makes sure that we built every parent before we get to the children
		for(auto &[ id, event ] : events)
			all_events.push_back(std::move(event));

		std::sort(all_events.begin(), all_events.end(), [](const auto &lhs, const auto &rhs) {
			return lhs.id < rhs.id;
		});

		// Then just create events for our parsed data
		for(auto &event : all_events)
		{
			telemetry_event actual(event.id, event.start_time, event.end_time);
			actual.set_entries(std::move(event.entries));

			if(event.parent == UINT64_MAX)
				container.add_event(std::move(actual));
			else
			{
				auto &parent = container.get_event(event.parent);
				parent.add_child(std::move(actual));
			}
		}
	}

	finalize_container(container, options);

	return container;
}



telemetry_container parse_telemetry_data(const uint8_t *data, size_t size, const telemetry_parser_options &options)
{
	file_reader reader(data, size);

	const uint32_t telemetry_version = reader.read_uint32();
	const uint32_t length = reader.read_uint32();

	if(telemetry_version == 2 && length == 8)
		return parser_tlmv2_data(reader, options);

	throw std::invalid_argument("Unsupported telemetry data");
}
