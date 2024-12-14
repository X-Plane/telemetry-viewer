//
// Created by Sidney on 13/12/2024.
//

#include <QFile>
#include "telemetry_document.h"
#include "telemetry/parser.h"
#include "../utilities/data_decimator.h"

telemetry_document *telemetry_document::load_file(const QString &path)
{
	QFile file(path);

	if(!file.open(QIODevice::ReadOnly))
		return {};

	const size_t length = file.bytesAvailable();

	std::vector<uint8_t> data;
	data.resize(length);

	file.read((char *)data.data(), length);
	file.close();

	telemetry_document *result = load_file(std::move(data));
	result->m_path = path;

	return result;
}

telemetry_document *telemetry_document::load_file(std::vector<uint8_t> &&data)
{
	telemetry_document *result = new telemetry_document();
	result->load(std::move(data));

	return result;
}

void telemetry_document::load(std::vector<uint8_t> &&data)
{
	telemetry_parser_options options;
	options.data_point_processor = [](const telemetry_container &container, const telemetry_provider &provider, const telemetry_field &field, const std::vector<telemetry_data_point> &data_points) {

		std::vector<telemetry_data_point> result = decimate_data(data_points, 1000);

		// Expand the first and last point to the very end of the telemetry range
		if(!result.empty())
		{
			auto first = result.front();
			if(first.timestamp > container.get_start_time())
			{
				first.timestamp = container.get_start_time();
				result.insert(result.begin(), first);
			}

			auto last = result.back();
			if(last.timestamp < container.get_end_time())
			{
				last.timestamp = container.get_end_time();
				result.push_back(last);
			}
		}

		return result;

	};

	m_data = parse_telemetry_data(data.data(), data.size(), options);

	m_path.clear();
	m_binary_data = std::move(data);
}

bool telemetry_document::save(const QString &path)
{
	QFile file(path);

	if(file.open(QIODevice::WriteOnly))
	{
		file.write((const char *)m_binary_data.data(), m_binary_data.size());
		m_path = path;

		return true;
	}

	return false;
}
