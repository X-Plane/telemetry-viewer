//
//  event.cpp
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

#include <stdexcept>
#include "event.h"

telemetry_event::telemetry_event(uint64_t id, double start_time, double end_time) :
	m_id(id),
	m_start_time(start_time),
	m_end_time(end_time)
{
	if(m_end_time < m_start_time)
		throw std::invalid_argument("End time must be greater than start time");
}

void telemetry_event::add_entry(telemetry_event_entry &&data)
{
	m_entries.push_back(std::move(data));
}
void telemetry_event::set_entries(std::vector<telemetry_event_entry> &&entries)
{
	m_entries = std::move(entries);
}

const telemetry_event &telemetry_event::get_child(uint64_t id) const
{
	for(auto &child : m_children)
	{
		if(child.m_id == id)
			return child;

		if(!child.has_child(id))
			continue;

		return child.get_child(id);
	}

	throw std::out_of_range("No child with id " + std::to_string(id));
}
telemetry_event &telemetry_event::get_child(uint64_t id)
{
	for(auto &child : m_children)
	{
		if(child.m_id == id)
			return child;

		if(!child.has_child(id))
			continue;

		return child.get_child(id);
	}

	throw std::out_of_range("No child with id " + std::to_string(id));
}

void telemetry_event::add_child(telemetry_event &&event)
{
	m_child_ids.insert(event.get_id());
	m_children.push_back(std::move(event));
}

bool telemetry_event::has_child(uint64_t id) const
{
	if(m_child_ids.contains(id))
		return true;

	for(auto &child : m_children)
	{
		if(child.has_child(id))
			return true;
	}

	return false;
}
