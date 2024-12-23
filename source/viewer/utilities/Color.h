//
// Created by Sidney on 28/07/2020.
//

#ifndef TELEMETRY_STUDIO_COLOR_H
#define TELEMETRY_STUDIO_COLOR_H

#include <QColor>
#include <random>

// This is the hash function of Qt5. It got changed in Qt 6, but I don't like the colours it generates
// So here is the copy and paste of the original hash function
inline uint32_t qt5Hash(const QString &string)
{
	uint32_t h = 0;

	for (int i = 0; i < string.length(); ++i)
		h = 31 * h + string[i].unicode();

	return h;
}

inline QColor generate_color(const QString &seed, float saturation, float lightness)
{
	static float golden_ratio = (1.0f + sqrtf(5.0f)) / 2.0f;
	const uint32_t hash = qt5Hash(seed);

	std::minstd_rand engine(hash);

	float hue = std::uniform_real_distribution<float>{0.0f, 1.0f}(engine);
	hue = fmodf(hue + golden_ratio, 1.0f);

	return QColor::fromHslF(hue, saturation, lightness);
}

#endif //TELEMETRY_STUDIO_COLOR_H
