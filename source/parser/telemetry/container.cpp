//
//  container.cpp
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
#include "container.h"

telemetry_container::telemetry_container(uint32_t version) :
	m_version(version),
	m_start_time(0),
	m_end_time(0)
{}

bool telemetry_container::has_provider(uint16_t id) const
{
	for(auto &provider : m_providers)
	{
		if(provider.get_id() == id)
			return true;
	}

	return false;
}
bool telemetry_container::has_provider(const std::string &identifier) const
{
	for(auto &provider : m_providers)
	{
		if(provider.get_identifier() == identifier)
			return true;
	}

	return false;
}

const telemetry_provider &telemetry_container::get_provider(uint16_t id) const
{
	for(auto &provider : m_providers)
	{
		if(provider.get_id() == id)
			return provider;
	}

	throw std::out_of_range("No provider with id " + std::to_string(id));
}
const telemetry_provider &telemetry_container::get_provider(const std::string &identifier) const
{
	for(auto &provider : m_providers)
	{
		if(provider.get_identifier() == identifier)
			return provider;
	}

	throw std::out_of_range("No provider with identifier " + identifier);
}

telemetry_provider &telemetry_container::get_provider(uint16_t id)
{
	for(auto &provider : m_providers)
	{
		if(provider.get_id() == id)
			return provider;
	}

	throw std::out_of_range("No provider with id " + std::to_string(id));
}
telemetry_provider &telemetry_container::get_provider(const std::string &identifier)
{
	for(auto &provider : m_providers)
	{
		if(provider.get_identifier() == identifier)
			return provider;
	}

	throw std::out_of_range("No provider with identifier " + identifier);
}


const telemetry_event &telemetry_container::get_event(uint64_t id) const
{
	for(auto &event : m_events)
	{
		if(event.get_id() == id)
			return event;

		if(!event.get_children().empty())
		{
			try
			{
				return event.get_child(id);
			}
			catch(...)
			{}
		}
	}

	throw std::out_of_range("No event with id " + std::to_string(id));
}
telemetry_event &telemetry_container::get_event(uint64_t id)
{
	for(auto &event : m_events)
	{
		if(event.get_id() == id)
			return event;

		if(event.has_child(id))
			return event.get_child(id);
	}

	throw std::out_of_range("No event with id " + std::to_string(id));
}

void telemetry_container::set_start_time(int32_t start_time)
{
	m_start_time = start_time;
}
void telemetry_container::set_end_time(int32_t end_time)
{
	m_end_time = end_time;
}

void telemetry_container::add_provider(telemetry_provider &&provider)
{
	m_providers.push_back(std::move(provider));
}
void telemetry_container::add_event(telemetry_event &&event)
{
	m_events.push_back(std::move(event));
}
void telemetry_container::add_statistic(telemetry_statistic &&statistic)
{
	m_statistics.push_back(std::move(statistic));
}
