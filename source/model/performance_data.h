//
// Created by Sidney on 12/03/2024.
//

#ifndef PERFORMANCE_DATA_H
#define PERFORMANCE_DATA_H

#include "telemetry_container.h"

enum class time_domain
{
	cpu,
	gpu
};

class performance_data
{
public:
	performance_data(const telemetry_container &data, uint32_t start, uint32_t end);

	bool contains_data() const { return !m_cpu_data.isEmpty() && !m_gpu_data.isEmpty(); }

	void set_range(uint32_t start, uint32_t end);

	double calculate_percentile(float percentile, time_domain domain) const;
	double calculate_average(time_domain domain) const;

private:
	const QVector<telemetry_data_point> &get_data_points(time_domain domain) const;

	const telemetry_container &m_data;

	QVector<telemetry_data_point> m_cpu_data;
	QVector<telemetry_data_point> m_gpu_data;
};

#endif //PERFORMANCE_DATA_H
