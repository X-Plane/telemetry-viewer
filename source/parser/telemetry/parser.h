//
// Created by Sidney on 09/12/2024.
//

#ifndef TELEMETRY_PARSER_H
#define TELEMETRY_PARSER_H

#include <functional>
#include "container.h"

struct telemetry_parser_options
{
	std::function<std::vector<telemetry_data_point> (const telemetry_container &, const telemetry_provider &, const telemetry_field &, const std::vector<telemetry_data_point> &)> data_point_processor;
};

telemetry_container parse_telemetry_data(const uint8_t *data, size_t size, const telemetry_parser_options &options);

#endif //TELEMETRY_PARSER_H
