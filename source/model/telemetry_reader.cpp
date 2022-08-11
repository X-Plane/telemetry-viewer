//
// Created by Sidney on 03/06/2020.
//

#include <vector>
#include <QFile>
#include "../utilities/data_decimator.h"
#include "../utilities/color.h"
#include "telemetry_reader.h"

struct file_reader
{
public:
	file_reader(const uint8_t *data) :
		m_data(data),
		m_start(data)
	{}

	file_reader(const uint8_t *data, size_t offset) :
		m_data(data + offset),
		m_start(data)
	{}

	size_t get_read() const { return (m_data - m_start); }
	bool at_end(size_t length) { return get_read() >= length; }

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
		uint16_t data[2];

		data[0] = *m_data ++;
		data[1] = *m_data ++;

		return (data[1] << 8) | data[0];
	}

	uint32_t read_uint32()
	{
		uint32_t data[4];

		data[0] = *m_data ++;
		data[1] = *m_data ++;
		data[2] = *m_data ++;
		data[3] = *m_data ++;

		return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
	}
	int32_t read_int32()
	{
		uint32_t data[4];

		data[0] = *m_data ++;
		data[1] = *m_data ++;
		data[2] = *m_data ++;
		data[3] = *m_data ++;

		return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
	}

	uint64_t read_uint64()
	{
		uint64_t data[8];

		data[0] = *m_data ++;
		data[1] = *m_data ++;
		data[2] = *m_data ++;
		data[3] = *m_data ++;
		data[4] = *m_data ++;
		data[5] = *m_data ++;
		data[6] = *m_data ++;
		data[7] = *m_data ++;

		return (data[7] << 56) | (data[6] << 48) | (data[5] << 40) | (data[4] << 32) | (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
	}
	int64_t read_int64()
	{
		uint64_t data[8];

		data[0] = *m_data ++;
		data[1] = *m_data ++;
		data[2] = *m_data ++;
		data[3] = *m_data ++;
		data[4] = *m_data ++;
		data[5] = *m_data ++;
		data[6] = *m_data ++;
		data[7] = *m_data ++;

		return (data[7] << 56) | (data[6] << 48) | (data[5] << 40) | (data[4] << 32) | (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
	}

	float read_float()
	{
		union
		{
			uint8_t ival[4];
			float fval;
		} map;

		map.ival[0] = *m_data ++;
		map.ival[1] = *m_data ++;
		map.ival[2] = *m_data ++;
		map.ival[3] = *m_data ++;

		return map.fval;
	}

	double read_double()
	{
		union
		{
			uint8_t ival[8];
			double fval;
		} map;

		map.ival[0] = *m_data ++;
		map.ival[1] = *m_data ++;
		map.ival[2] = *m_data ++;
		map.ival[3] = *m_data ++;
		map.ival[4] = *m_data ++;
		map.ival[5] = *m_data ++;
		map.ival[6] = *m_data ++;
		map.ival[7] = *m_data ++;

		return map.fval;
	}

	QString read_string()
	{
		const uint8_t length = *m_data ++;

		QString result(length, '\0');

		for(uint8_t i = 0; i < length; i ++)
			result[i] = *m_data ++;

		return result;
	}

private:
	const uint8_t *m_data;
	const uint8_t *m_start;
};

enum class telemetry_command : uint8_t
{
	register_provider,
	packet,
	statistic,
	amend_provider,
	event
};


QVariant read_variant(file_reader &reader, telemetry_type type)
{
	switch(type)
	{
		case telemetry_type::boolean:
		{
			return QVariant(reader.read_bool());
		}

		case telemetry_type::uint8:
		{
			return QVariant(reader.read_uint8());
		}
		case telemetry_type::uint16:
		{
			return QVariant(reader.read_uint16());
		}
		case telemetry_type::uint32:
		{
			return QVariant(reader.read_uint32());
		}
		case telemetry_type::uint64:
		{
			return QVariant(reader.read_uint64());
		}


		case telemetry_type::int32:
		{
			return QVariant(reader.read_int32());
		}
		case telemetry_type::int64:
		{
			return QVariant(reader.read_int64());
		}

		case telemetry_type::floatv:
		{
			return QVariant(reader.read_float());
		}
		case telemetry_type::doubv:
		{
			return QVariant(reader.read_double());
		}

		case telemetry_type::vec2:
		{
			return QVariant(QPointF(reader.read_float(), reader.read_float()));
		}
		case telemetry_type::dvec2:
		{
			return QVariant(QPointF(reader.read_double(), reader.read_double()));
		}

		case telemetry_type::string:
		{
			return QVariant(QVariant(reader.read_string()));
		}
	}

	return QVariant();
}


telemetry_container read_telemetry_data(const uint8_t *data, size_t size)
{
	telemetry_container container;

	file_reader reader(data, 8);

	while(!reader.at_end(size))
	{
		telemetry_command command = (telemetry_command)reader.read_uint8();

		switch(command)
		{
			case telemetry_command::register_provider:
			{
				telemetry_provider provider;

				provider.identifier = reader.read_string();
				provider.title = reader.read_string();
				provider.version = reader.read_uint16();
				provider.runtime_id = reader.read_uint16();

				const uint32_t fields = reader.read_uint32();

				for(uint32_t i = 0; i < fields; ++ i)
				{
					telemetry_provider_field field;

					field.enabled = false;
					field.id = reader.read_uint8();
					field.provider_id = provider.runtime_id;
					field.type = (telemetry_type)reader.read_uint8();
					field.unit = (telemetry_unit)reader.read_uint8();
					field.title = reader.read_string();
					field.color = generate_color(field.title, 0.9f, 0.4f);

					provider.fields.push_back(field);
				}

				container.providers.push_back(provider);
				break;
			}

			case telemetry_command::amend_provider:
			{
				uint16_t runtime_id = reader.read_uint16();
				telemetry_provider &provider = container.find_provider(runtime_id);

				provider.fields.clear();

				const uint32_t fields = reader.read_uint32();

				for(uint32_t i = 0; i < fields; ++ i)
				{
					telemetry_provider_field field;

					field.enabled = false;
					field.id = reader.read_uint8();
					field.provider_id = runtime_id;
					field.type = (telemetry_type)reader.read_uint8();
					field.unit = (telemetry_unit)reader.read_uint8();
					field.title = reader.read_string();
					field.color = generate_color(field.title, 0.9f, 0.4f);

					provider.fields.push_back(field);
				}

				break;
			}

			case telemetry_command::packet:
			{
				const uint16_t runtime_id = reader.read_uint16();
				const uint32_t packets = reader.read_uint32();

				telemetry_provider &provider = container.find_provider(runtime_id);

				for(uint32_t i = 0; i < packets; ++ i)
				{
					double timestamp = reader.read_double();

					const size_t length = reader.read_uint32();
					const size_t read = reader.get_read();

					while((reader.get_read() - read) < length)
					{
						telemetry_provider_field &field = provider.find_field(reader.read_uint8());
						QVariant data = read_variant(reader, field.type);

						field.data_points.push_back(std::make_pair(timestamp, data));
					}
				}

				break;
			}

			case telemetry_command::statistic:
			{
				telemetry_statistic stat;

				stat.title = reader.read_string();

				const size_t length = reader.read_uint32();
				const size_t read = reader.get_read();

				while((reader.get_read() - read) < length)
				{
					telemetry_type type = (telemetry_type)reader.read_uint8();

					QString title = reader.read_string();
					QVariant value = read_variant(reader, type);

					stat.entries.push_back(std::make_pair(title, value));
				}


				container.statistics.push_back(stat);
				break;
			}
		}
	}

#if 1
	for(auto &provider : container.providers)
	{
		for(auto &field : provider.fields)
		{
			field.data_points = decimate_data(field.data_points, 1000);
		}
	}
#endif

	double start_time = 0.0;
	double end_time = 0.0;

	for(auto &provider : container.providers)
	{
		for(auto &field : provider.fields)
		{
			if(field.data_points.empty())
				continue;

			start_time = std::min(start_time, field.data_points.first().first);
			end_time = std::max(end_time, field.data_points.last().first);
		}
	}

	container.start_time = floor(start_time);
	container.end_time = ceil(end_time);

	return container;
}

telemetry_container read_telemetry_data(const QString &path)
{
	QFile file(path);

	if(!file.open(QIODevice::ReadOnly))
		return telemetry_container();

	const size_t length = file.bytesAvailable();

	uint8_t *data = new uint8_t[length];
	file.read((char *)data, length);
	file.close();

	telemetry_container container = read_telemetry_data(data, length);

	delete[] data;

	return container;
}
