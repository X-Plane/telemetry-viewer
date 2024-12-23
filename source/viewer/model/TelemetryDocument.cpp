//
// Created by Sidney on 13/12/2024.
//

#include <QFile>
#include "TelemetryDocument.h"
#include "telemetry/parser.h"
#include "../utilities/DataDecimator.h"

TelemetryDocument *TelemetryDocument::load_file(const QString &path)
{
	QFile file(path);

	if(!file.open(QIODevice::ReadOnly))
		return {};

	const size_t length = file.bytesAvailable();

	std::vector<uint8_t> data;
	data.resize(length);

	file.read((char *)data.data(), length);
	file.close();

	TelemetryDocument *result = load_file(std::move(data));
	result->m_path = path;

	return result;
}

TelemetryDocument *TelemetryDocument::load_file(std::vector<uint8_t> &&data)
{
	TelemetryDocument *result = new TelemetryDocument();
	result->load(std::move(data));

	return result;
}

void TelemetryDocument::load(std::vector<uint8_t> &&data)
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

bool TelemetryDocument::save(const QString &path)
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
