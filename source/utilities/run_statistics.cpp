//
// Created by Sidney on 30/11/2023.
//

#include "model/telemetry_container.h"
#include "run_statistics.h"

double calculate_percentile_for_time(float percentile, const QVector<telemetry_data_point> &data_points)
{
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

	return -1.0;
}

double calculate_percentile_for_values(float percentile, const QVector<telemetry_data_point> &data_points)
{
	if(data_points.isEmpty())
		return 0.0;

	QVector<telemetry_data_point> samples = data_points;

	std::sort(samples.begin(), samples.end(), [](const telemetry_data_point &lhs, const telemetry_data_point &rhs) {

		return lhs.value.toDouble() < rhs.value.toDouble();

	});

	return samples[size_t(samples.size() * percentile) - 1].value.toDouble();
}

double calculate_average(const QVector<telemetry_data_point> &data_points)
{
	if(data_points.isEmpty())
		return 0.0;

	double sum = 0.0;

	for(const auto &sample : data_points)
		sum += sample.value.toDouble();

	return sum / data_points.size();
}
