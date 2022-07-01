//
// Created by Sidney on 03/06/2020.
//

#include <stdexcept>
#include "telemetry_container.h"

const char *telemetry_unit_to_string(telemetry_unit unit)
{
	switch(unit)
	{
		case telemetry_unit::value:
			return "Value";
		case telemetry_unit::fps:
			return "FPS";
		case telemetry_unit::time:
			return "Time";
		case telemetry_unit::memory:
			return "Memory";
	}

	return "";
}

telemetry_provider_field &telemetry_provider::find_field(uint8_t id)
{
	for(auto &field : fields)
	{
		if(field.id == id)
			return field;
	}

	throw std::invalid_argument("Unknown field ID");
}

telemetry_provider &telemetry_container::find_provider(uint16_t runtime_id)
{
	for(auto &provider : providers)
	{
		if(provider.runtime_id == runtime_id)
			return provider;
	}

	throw std::invalid_argument("Unknown provider ID");
}

