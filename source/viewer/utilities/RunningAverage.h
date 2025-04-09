//
// Created by Sidney on 13/01/2025.
//

#ifndef RUNNINGAVERAGE_H
#define RUNNINGAVERAGE_H

#include <array>

template<size_t WindowSize>
class RunningAverage
{
public:
	RunningAverage() = default;

	double update(double new_value)
	{
		if(m_count < WindowSize)
		{
			m_sum += new_value;
			m_window[m_write_pos] = new_value;

			m_count ++;
		}
		else
		{
			m_sum = m_sum - m_window[m_write_pos] + new_value;
			m_window[m_write_pos] = new_value;
		}

		m_write_pos = (m_write_pos + 1) % WindowSize;
		return get_average();
	}

	double get_average() const
	{
		if(m_count == 0)
			return 0;

		return m_sum / m_count;
	}

	void reset()
	{
		m_sum = 0;
		m_write_pos = m_count = 0;
	}

private:
	std::array<double, WindowSize> m_window;

	double m_sum = 0.0f;
	size_t m_write_pos = 0;
	size_t m_count = 0;
};

#endif //RUNNINGAVERAGE_H
