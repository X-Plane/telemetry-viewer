//
//  parser.h
//  libtlm
//
//  Created by Sidney Just
//  Copyright (c) 2024 by Laminar Research
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//  documentation files (the "Software"), to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
//  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
//  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
