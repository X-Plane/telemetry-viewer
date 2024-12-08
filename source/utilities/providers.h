//
// Created by Sidney on 08/12/2024.
//

#ifndef PROVIDERS_H
#define PROVIDERS_H

#include "../model/telemetry_container.h"

struct provider_timing
{
	static inline const char *identifier = "com.laminarresearch.test_main_class";

	enum field_id
	{
		cpu,
		gpu,
		fps,
		time,
		plugin,
	};

	static const telemetry_provider_field &get_field(const telemetry_container &container, field_id id) { return container.find_provider(identifier).find_field(uint8_t(id)); }
};

struct provider_sim_apup
{
	static inline const char *identifier = "com.laminarresarch.sim_apup";

	enum field_id
	{
		do_world,
		do_world_3d,
		loaded_aircraft
	};

	static const telemetry_provider_field &get_field(const telemetry_container &container, field_id id) { return container.find_provider(identifier).find_field(uint8_t(id)); }
};

#endif //PROVIDERS_H
