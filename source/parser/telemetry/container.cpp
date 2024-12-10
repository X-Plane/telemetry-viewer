//
// Created by Sidney on 09/12/2024.
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
