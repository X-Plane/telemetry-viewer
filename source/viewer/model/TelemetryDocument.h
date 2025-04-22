//
// Created by Sidney on 13/12/2024.
//

#ifndef TELEMETRY_DOCUMENT_H
#define TELEMETRY_DOCUMENT_H

#include <QString>
#include <telemetry/container.h>

struct TelemetryRegion
{
	enum class Type : uint8_t
	{
		Everything,
		InMenu,
		Flying
	};

	double start, end;
	QString name;
	Type type;
};

class TelemetryDocument
{
public:
	static TelemetryDocument *load_file(const QString &path);
	static TelemetryDocument *load_file(std::vector<uint8_t> &&data, const QString &name);

	void set_name(const QString &name);

	bool save(const QString &path);
	bool is_draft() const { return m_path.isEmpty(); }
	bool has_data() const { return !m_binary_data.empty(); }

	const QString &get_name() const { return m_name; }
	const QString &get_path() const { return m_path; }
	const telemetry_container &get_data() const { return m_data; }
	const QVector<TelemetryRegion> &get_regions() const { return m_regions; }

protected:
	TelemetryDocument() = default;

	void load(std::vector<uint8_t> &&data, const QString &name);

private:
	QString m_path;
	QString m_name;
	telemetry_container m_data;
	std::vector<uint8_t> m_binary_data;

	QVector<TelemetryRegion> m_regions;
};

#endif //TELEMETRY_DOCUMENT_H
