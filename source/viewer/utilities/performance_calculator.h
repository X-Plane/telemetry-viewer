//
// Created by Sidney on 12/03/2024.
//

#ifndef PERFORMANCE_DATA_H
#define PERFORMANCE_DATA_H

#include <telemetry/container.h>

class performance_calculator
{
public:
	performance_calculator(const telemetry_field &field, uint32_t start, uint32_t end);

	size_t get_sample_count() const { return m_samples.size(); }

	double calculate_average() const;
	double calculate_percentile(float percentile) const;

	double get_sample(size_t index) const { return m_samples.at(index).value.get<double>(); }
	double get_median_value(size_t start, size_t end) const;

	double get_minimum() const { return m_samples.front().value.get<double>(); }
	double get_maximum() const { return m_samples.back().value.get<double>(); }

private:
	std::vector<telemetry_data_point> m_samples;
};

#endif //PERFORMANCE_DATA_H
