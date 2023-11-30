//
// Created by Sidney on 28/07/2020.
//

#ifndef TELEMETRY_STUDIO_DATA_DECIMATOR_H
#define TELEMETRY_STUDIO_DATA_DECIMATOR_H

#include <QVector>

struct telemetry_data_point;

QVector<telemetry_data_point> decimate_data(const QVector<telemetry_data_point> &input, uint32_t threshold);

#endif //TELEMETRY_STUDIO_DATA_DECIMATOR_H
