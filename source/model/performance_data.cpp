//
// Created by Sidney on 12/03/2024.
//

#include "performance_data.h"

performance_data::performance_data(const telemetry_container &data, uint32_t start, uint32_t end) :
	m_data(data)
{
	set_range(start, end);
}

void performance_data::set_range(uint32_t start, uint32_t end)
{
	m_cpu_data.clear();
	m_gpu_data.clear();

	try
	{
		const auto &provider = m_data.find_provider("com.laminarresearch.test_main_class");

		m_cpu_data = provider.find_field(0).get_data_points_in_range(start, end);
		m_gpu_data = provider.find_field(1).get_data_points_in_range(start, end);

		// Pre sort the CPU and GPU perf data. This allows us to properly calculate percentiles
		std::sort(m_cpu_data.begin(), m_cpu_data.end(), [](const telemetry_data_point &lhs, const telemetry_data_point &rhs) {
			return lhs.value.toDouble() < rhs.value.toDouble();
		});
		std::sort(m_gpu_data.begin(), m_gpu_data.end(), [](const telemetry_data_point &lhs, const telemetry_data_point &rhs) {
			return lhs.value.toDouble() < rhs.value.toDouble();
		});
	}
	catch(...)
	{}
}

const QVector<telemetry_data_point> &performance_data::get_data_points(time_domain domain) const
{
	return (domain == time_domain::cpu) ? m_cpu_data : m_gpu_data;
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
			return sample.value.toDouble();

		total_time += sample.value.toDouble();
	}

	return samples.back().value.toDouble();
}
double performance_data::calculate_average(time_domain domain) const
{
	const auto &data_points = get_data_points(domain);

	if(data_points.isEmpty())
		return 0.0;

	double sum = 0.0;

	for(const auto &sample : data_points)
		sum += sample.value.toDouble();

	return sum / data_points.size();
}
