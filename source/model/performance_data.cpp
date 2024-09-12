//
// Created by Sidney on 12/03/2024.
//

#include "performance_data.h"

performance_data::performance_data(const telemetry_container &data, uint32_t start, uint32_t end) :
	m_container(data)
{
	set_range(start, end);
}

void performance_data::set_range(uint32_t start, uint32_t end)
{
	m_data.clear();

	auto domain_range = { time_domain::cpu, time_domain::gpu, time_domain::frame_time };

	for(auto &domain : domain_range)
	{
		const telemetry_provider_field *field = field_for_domain(domain);
		if(!field)
			continue;

		performance_entry &entry = m_data.emplace_back();
		entry.field = field;
		entry.domain = domain;
		entry.data = field->get_data_points_in_range(start, end);

		std::sort(entry.data.begin(), entry.data.end(), [](const telemetry_data_point &lhs, const telemetry_data_point &rhs) {
			return lhs.value.toDouble() < rhs.value.toDouble();
		});
	}
}

const telemetry_provider_field *performance_data::field_for_domain(time_domain domain) const
{
	try
	{
		const auto &provider = m_container.find_provider("com.laminarresearch.test_main_class");

		switch(domain)
		{
			case time_domain::cpu:
				return &provider.find_field(0);
			case time_domain::gpu:
				return &provider.find_field(1);
			case time_domain::frame_time:
				return &provider.find_field(3);
		}
	}
	catch(...)
	{}

	return nullptr;
}

const QVector<telemetry_data_point> &performance_data::get_data_points(time_domain domain) const
{
	for(auto &entry : m_data)
	{
		if(entry.domain == domain)
			return entry.data;
	}

	return {};
}

double performance_data::calculate_percentile(float percentile, time_domain domain) const
{
	const auto &data_points = get_data_points(domain);

	if(data_points.isEmpty())
		return 0.0;

	QVector<telemetry_data_point> samples = data_points;

	std::sort(samples.begin(), samples.end(), [](const telemetry_data_point &lhs, const telemetry_data_point &rhs) {

		return lhs.value.toDouble() < rhs.value.toDouble();

	});

	double total_time = 0.0;

	for(const auto &sample : samples)
		total_time += sample.value.toDouble();

	const double needle = total_time * percentile;

	total_time = 0.0;

	for(const auto &sample : samples)
	{
		if(total_time >= needle)
			return sample.value.toDouble() * 1000.0;

		total_time += sample.value.toDouble();
	}

	return samples.back().value.toDouble() * 1000.0;
}
double performance_data::calculate_average(time_domain domain) const
{
	const auto &data_points = get_data_points(domain);

	if(data_points.isEmpty())
		return 0.0;

	double sum = 0.0;

	for(const auto &sample : data_points)
		sum += sample.value.toDouble();

	return (sum / data_points.size()) * 1000.0;
}
