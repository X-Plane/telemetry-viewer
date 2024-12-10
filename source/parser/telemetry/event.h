//
//  event.h
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

#ifndef TELEMETRY_EVENT_H
#define TELEMETRY_EVENT_H

#include <string>
#include <vector>
#include <unordered_set>
#include "data.h"

struct telemetry_event_entry
{
	std::string title;
	telemetry_data_value value;
};

class telemetry_event
{
public:
	telemetry_event(uint64_t id, double start_time, double end_time);

	uint64_t get_id() const { return m_id; }
	double get_start() const { return m_start_time; }
	double get_end() const { return m_end_time; }
	double get_duration() const { return m_end_time - m_start_time; }

	const std::vector<telemetry_event_entry> &get_entries() const { return m_entries; }
	std::vector<telemetry_event_entry> &get_entries() { return m_entries; }

	void add_entry(telemetry_event_entry &&data);
	void set_entries(std::vector<telemetry_event_entry> &&entries);

	const std::vector<telemetry_event> &get_children() const { return m_children; }
	std::vector<telemetry_event> &get_children() { return m_children; }

	bool has_child(uint64_t id) const;
	const telemetry_event &get_child(uint64_t id) const;
	telemetry_event &get_child(uint64_t id);

	void add_child(telemetry_event &&event);

private:
	uint64_t m_id;
	double m_start_time;
	double m_end_time;

	std::vector<telemetry_event_entry> m_entries;
	std::vector<telemetry_event> m_children;
	std::unordered_set<uint64_t> m_child_ids;
};

#endif //TELEMETRY_EVENT_H
