//
// Created by Sidney on 28/07/2020.
//

#ifndef TELEMETRY_STUDIO_COLOR_H
#define TELEMETRY_STUDIO_COLOR_H

#include <QString>
#include <QHash>
#include <random>

inline QString generate_color(const QString &seed, float saturation, float lightness)
{
	static float golden_ratio = (1.0f + sqrtf(5.0f)) / 2.0f;
	const uint32_t hash = qHash(seed);

	std::minstd_rand engine(hash);

	float hue = std::uniform_real_distribution<float>{0.0f, 1.0f}(engine);
	hue = fmodf(hue + golden_ratio, 1.0f);

	return "hsl(" + QString::number(int(hue * 360)) + ", " + QString::number((int)(saturation * 100)) + "%, " + QString::number((int)(lightness * 100)) + "%)";
}

#endif //TELEMETRY_STUDIO_COLOR_H
