//
// Created by Sidney on 13/12/2024.
//

#ifndef TELEMETRY_DOCUMENT_H
#define TELEMETRY_DOCUMENT_H

#include <QString>
#include <telemetry/container.h>

class TelemetryDocument
{
public:
	static TelemetryDocument *load_file(const QString &path);
	static TelemetryDocument *load_file(std::vector<uint8_t> &&data);

	bool save(const QString &path);
	bool is_draft() const { return m_path.isEmpty(); }
	bool has_data() const { return !m_binary_data.empty(); }

	const QString &get_path() const { return m_path; }
	const telemetry_container &get_data() const { return m_data; }

protected:
	TelemetryDocument() = default;

	void load(std::vector<uint8_t> &&data);

private:
	QString m_path;
	telemetry_container m_data;
	std::vector<uint8_t> m_binary_data;
};

#endif //TELEMETRY_DOCUMENT_H
