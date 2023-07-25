//
// Created by Sidney on 28/07/2020.
//

#include "data_decimator.h"
#include <cmath>

QVector<std::pair<double, QVariant>> decimate_data(const QVector<std::pair<double, QVariant>> &input, uint32_t threshold)
{
	if(threshold >= input.size() || threshold == 0)
		return input;

	const float increment = float(input.size() - 2) / (threshold - 2);

	QVector<std::pair<double, QVariant>> result;

	result.reserve(threshold);
	result.push_back(input.first());

	size_t a = 0;

	for(size_t i = 0; i < threshold - 2; ++ i)
	{
		// Calculate the average
		size_t range_start = floor((i + 1) * increment) + 1;
		size_t range_end = std::min((int)(floor((i + 2) * increment) + 1), input.size());

		double average_x = 0.0;
		double average_y = 0.0;

		for(size_t j = range_start; j < range_end; ++ j)
		{
			average_x += input[j].first;
			average_y += input[j].second.toDouble();
		}

		average_x /= (range_end - range_start);
		average_y /= (range_end - range_start);



		range_start = floor((i + 0) * increment) + 1;
		range_end = std::min((int)(floor((i + 1) * increment) + 1), input.size());

		const double point_a_x = input[a].first;
		const double point_a_y = input[a].second.toDouble();

		int32_t max_area = -1;
		size_t max_area_index;

		for(size_t j = range_start; j < range_end; ++ j)
		{
			int32_t area = abs((point_a_x - average_x) * (input[j].second.toDouble() - point_a_y) - (point_a_x - input[j].first) * (average_y - point_a_y)) * 0.5;

			if(area > max_area)
			{
				max_area = area;
				max_area_index = j;
			}
		}

		if(max_area >= 0)
		{
			result.push_back(input[max_area_index]);
			a = max_area_index;
		}
	}

	result.push_back(input.last());

	return result;
}
