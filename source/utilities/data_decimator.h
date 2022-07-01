//
// Created by Sidney on 28/07/2020.
//

#ifndef TELEMETRY_STUDIO_DATA_DECIMATOR_H
#define TELEMETRY_STUDIO_DATA_DECIMATOR_H

#include <QVector>
#include <QVariant>
#include <cstdint>

QVector<std::pair<double, QVariant>> decimate_data(const QVector<std::pair<double, QVariant>> &input, uint32_t threshold);

#endif //TELEMETRY_STUDIO_DATA_DECIMATOR_H
