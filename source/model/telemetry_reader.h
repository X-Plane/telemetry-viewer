//
// Created by Sidney on 03/06/2020.
//

#ifndef TELEMETRY_STUDIO_TELEMETRY_READER_H
#define TELEMETRY_STUDIO_TELEMETRY_READER_H

#include <stdint.h>
#include "telemetry_container.h"

telemetry_container read_telemetry_data(const uint8_t *data, size_t size);
telemetry_container read_telemetry_data(const QString &path);

#endif //TELEMETRY_STUDIO_TELEMETRY_READER_H
