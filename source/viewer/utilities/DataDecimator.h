//
// Created by Sidney on 28/07/2020.
//

#ifndef TELEMETRY_STUDIO_DATA_DECIMATOR_H
#define TELEMETRY_STUDIO_DATA_DECIMATOR_H

#include <vector>
#include <telemetry/data.h>

std::vector<telemetry_data_point> decimate_data(const std::vector<telemetry_data_point> &input, uint32_t threshold);

#endif //TELEMETRY_STUDIO_DATA_DECIMATOR_H
