//
// Created by Sidney on 03/06/2020.
//

#ifndef TELEMETRY_STUDIO_TELEMETRY_CONTAINER_H
#define TELEMETRY_STUDIO_TELEMETRY_CONTAINER_H

#include <QString>
#include <QVector>
#include <QVariant>
#include <cstdint>

enum class telemetry_type : uint8_t
{
	uint8 = 0,
	uint16 = 2,

	uint32 = 3,
	int32 = 4,

	uint64 = 6,
	int64 = 7,

	doubv = 10,
	floatv = 11,

	string = 15,

	boolean = 20,

	vec2 = 50,
	dvec2 = 51,
};
enum class telemetry_unit : uint8_t
{
	value = 0,
	fps = 1,
	time = 2,
	memory = 3,
	duration = 4
};

const char *telemetry_unit_to_string(telemetry_unit unit);

struct telemetry_statistic
{
	QString title;
	QVector<std::pair<QString, QVariant>> entries;
};

struct telemetry_provider_field
{
	bool enabled;
	uint8_t id;
	uint16_t provider_id;
	telemetry_type type;
	telemetry_unit unit;
	QString title;
	QString color;
	QVector<std::pair<double, QVariant>> data_points;
};

struct telemetry_provider
{
	QString identifier;
	QString title;
	uint16_t version;
	uint16_t runtime_id;
	QVector<telemetry_provider_field> fields;

	telemetry_provider_field &find_field(uint8_t id);
};

struct telemetry_container
{
	QVector<telemetry_statistic> statistics;
	QVector<telemetry_provider> providers;

	int32_t start_time;
	int32_t end_time;

	telemetry_provider &find_provider(uint16_t runtime_id);
};

#endif //TELEMETRY_STUDIO_TELEMETRY_CONTAINER_H
