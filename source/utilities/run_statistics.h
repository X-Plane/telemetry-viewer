//
// Created by Sidney on 30/11/2023.
//

#ifndef RUN_STATISTICS_H
#define RUN_STATISTICS_H

#include <QVector>

class telemetry_data_point;

double calculate_percentile_for_time(float percentile, const QVector<telemetry_data_point> &data_points);
double calculate_percentile_for_values(float percentile, const QVector<telemetry_data_point> &data_points);
double calculate_average(const QVector<telemetry_data_point> &data_points);

#endif //RUN_STATISTICS_H
