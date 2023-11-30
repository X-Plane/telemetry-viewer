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
enum class telemetry_event_type : uint8_t
{
	begin = 'b',
	end = 'e',
	meta = 'm',
};

const char *telemetry_unit_to_string(telemetry_unit unit);
const char *telemetry_event_type_to_string(telemetry_event_type event_type);

struct telemetry_statistic
{
	QString title;
	QVector<std::pair<QString, QVariant>> entries;
};

struct telemetry_event
{
	uint64_t id;
	double time;
	telemetry_event_type event_type;
	QVector<std::pair<QString, QVariant>> fields;
};

struct telemetry_event_span
{
	uint64_t id;
	double begin;
	double end;
	QVector<std::pair<QString, QVariant>> fields;
	QVector<telemetry_event_span> child_spans;
};

struct telemetry_provider_field
{
	QVector<std::pair<double, QVariant>> get_data_points_in_range(int32_t start, int32_t end) const;
	std::pair<double, QVariant> get_data_point_after_time(int32_t time) const;

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
	QVector<telemetry_event> events;
	QVector<telemetry_event_span> event_spans;

	int32_t start_time;
	int32_t end_time;

	QByteArray raw_data;

	telemetry_provider &find_provider(uint16_t runtime_id);
};

#endif //TELEMETRY_STUDIO_TELEMETRY_CONTAINER_H
