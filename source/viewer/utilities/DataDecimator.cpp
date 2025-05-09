//
// Created by Sidney on 28/07/2020.
//

#include <cmath>
#include "DataDecimator.h"

std::vector<telemetry_data_point> decimate_data(const std::vector<telemetry_data_point> &input, uint32_t threshold)
{
	if(threshold >= input.size() || threshold == 0)
		return input;

	const float increment = float(input.size() - 2) / (threshold - 2);

	std::vector<telemetry_data_point> result;

	result.reserve(threshold);
	result.push_back(input.front());

	size_t a = 0;

	for(size_t i = 0; i < threshold - 2; ++ i)
	{
		// Calculate the average
		size_t range_start = std::floor((i + 1) * increment) + 1;
		size_t range_end = std::min((size_t)(std::floor((i + 2) * increment) + 1), input.size());

		double average_x = 0.0;
		double average_y = 0.0;

		for(size_t j = range_start; j < range_end; ++ j)
		{
			average_x += input[j].timestamp;
			average_y += input[j].value.get<double>();
		}

		average_x /= (range_end - range_start);
		average_y /= (range_end - range_start);



		range_start = std::floor((i + 0) * increment) + 1;
		range_end = std::min((size_t)(std::floor((i + 1) * increment) + 1), input.size());

		const double point_a_x = input[a].timestamp;
		const double point_a_y = input[a].value.get<double>();

		int32_t max_area = -1;
		size_t max_area_index;

		for(size_t j = range_start; j < range_end; ++ j)
		{
			int32_t area = abs((point_a_x - average_x) * (input[j].value.get<double>() - point_a_y) - (point_a_x - input[j].timestamp) * (average_y - point_a_y)) * 0.5;

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

	result.push_back(input.back());

	return result;
}
