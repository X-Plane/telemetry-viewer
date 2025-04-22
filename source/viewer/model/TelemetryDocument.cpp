//
// Created by Sidney on 13/12/2024.
//

#include <QFile>
#include <telemetry/known_providers.h>
#include <telemetry/parser.h>
#include "TelemetryDocument.h"
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

	TelemetryRegion everything;
	everything.start = m_data.get_start_time();
	everything.end = m_data.get_end_time();
	everything.name = "Everything";
	everything.type = TelemetryRegion::Type::Everything;

	m_regions.clear();
	m_regions.push_back(everything);

	if(m_data.has_provider(provider_sim_apup::identifier))
	{
		auto &do_world_events = provider_sim_apup::get_field(m_data, provider_sim_apup::do_world);
		auto &aircraft_events = provider_sim_apup::get_field(m_data, provider_sim_apup::loaded_aircraft);

		bool previous_state = false;
		double previous_timestamp = m_data.get_start_time();

		auto flush_range = [this, &aircraft_events](double start, double end, TelemetryRegion::Type type) {

			// We want at least 12 seconds worth of data to add it to the timeline
			if((end - start) > 12.0)
			{
				QString title;

				if(type == TelemetryRegion::Type::Flying)
				{
					try
					{
						title = aircraft_events.get_data_point_after_time(start).value.get<const char *>();
					}
					catch(...)
					{
						title = "Flying";
					}
				}
				else if(type == TelemetryRegion::Type::InMenu)
					title = "In Menu";

				TelemetryRegion region;
				region.start = start;
				region.end = end;
				region.name = title;
				region.type = type;

				m_regions.push_back(region);
			}

		};

		for(auto &data : do_world_events.get_data_points())
		{
			const bool is_doing_world = data.value.get<bool>();

			if(is_doing_world != previous_state)
			{
				double timestamp = data.timestamp;

				// Add 10 seconds of padding to the end of in menu regions to give the sim time to stabilize
				if(is_doing_world)
					timestamp += 10.0;

				flush_range(previous_timestamp, timestamp, previous_state ? TelemetryRegion::Type::Flying : TelemetryRegion::Type::InMenu);

				previous_timestamp = timestamp;
				previous_state = is_doing_world;
			}
		}

		flush_range(previous_timestamp, m_data.get_end_time(), previous_state ? TelemetryRegion::Type::Flying : TelemetryRegion::Type::InMenu);
	}

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
