//
// Created by Sidney on 12/03/2024.
//

#include "performance_calculator.h"

performance_calculator::performance_calculator(const telemetry_provider_field &field, uint32_t start, uint32_t end)
{
	m_samples = field.get_data_points_in_range(start, end);

	std::sort(m_samples.begin(), m_samples.end(), [](const telemetry_data_point &lhs, const telemetry_data_point &rhs) {
		return lhs.value.toDouble() < rhs.value.toDouble();
	});
}

double performance_calculator::calculate_average() const
{
	if(m_samples.isEmpty())
		return 0.0;

	double sum = 0.0;

	for(const auto &sample : m_samples)
		sum += sample.value.toDouble();

	return sum / m_samples.size();
}

double performance_calculator::calculate_percentile(float percentile) const
{
	if(m_samples.isEmpty())
		return 0.0;

	double total_time = 0.0;

	for(const auto &sample : m_samples)
		total_time += sample.value.toDouble();

	const double needle = total_time * percentile;

	total_time = 0.0;

	for(const auto &sample : m_samples)
	{
		if(total_time >= needle)
			return sample.value.toDouble();

		total_time += sample.value.toDouble();
	}

	return m_samples.back().value.toDouble();
}

double performance_calculator::get_median_value(size_t start, size_t end) const
{
	Q_ASSERT(start < m_samples.size());
	Q_ASSERT(end <= m_samples.size());

	const size_t count = end - start;
	const size_t half = count / 2;

	if(count & 0x1)
	{
		const double right = m_samples[half + start].value.toDouble();
		const double left = m_samples[half - 1 + start].value.toDouble();

		return (right + left) / 2.0;
	}

	return m_samples[half + start].value.toDouble();
}
