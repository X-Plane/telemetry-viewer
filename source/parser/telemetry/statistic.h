//
//  statistic.h
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

#ifndef TELEMETRY_STATISTIC_H
#define TELEMETRY_STATISTIC_H

#include <string>
#include <vector>
#include "data.h"

struct telemetry_statistic_entry
{
	std::string title;
	telemetry_data_value value;
};

class telemetry_statistic
{
public:
	telemetry_statistic(std::string title);

	const std::string &get_title() const { return m_title; }

	const std::vector<telemetry_statistic_entry> &get_entries() const { return m_entries; }
	std::vector<telemetry_statistic_entry> &get_entries() { return m_entries; }

	void add_entry(telemetry_statistic_entry &&entry);

private:
	std::string m_title;

	std::vector<telemetry_statistic_entry> m_entries;
};

#endif //TELEMETRY_STATISTIC_H
