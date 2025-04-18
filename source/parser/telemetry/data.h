//
//  data.h
//  libtlm
//
//  Created by Sidney Just
//  Copyright (c) 2024 by Laminar Research
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//  documentation files (the "Software"), to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
//  and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
//  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
//  FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#ifndef TELEMETRY_DATA_H
#define TELEMETRY_DATA_H

#include <cstdint>
#include <string>
#include <stdexcept>
#include <type_traits>

enum class telemetry_type : uint8_t
{
	uint8 = 0,
	uint16 = 2,

	uint32 = 3,
	int32 = 4,

	uint64 = 6,
	int64 = 7,

	f64 = 10,
	f32 = 11,

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

struct telemetry_data_value
{
	union
	{
		bool b;

		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;

		uint32_t i32;
		uint64_t i64;

		float f32;
		double f64;

		float vec2[2];
		double dvec2[2];
	};

	std::string string;
	telemetry_type type;

	template<class T>
	T get() const
	{
		if constexpr (std::is_integral<T>::value || std::is_floating_point<T>::value)
		{
			switch(type)
			{
				using enum telemetry_type;

				case boolean:
					return static_cast<T>(b);

				case uint8:
					return static_cast<T>(u8);
				case uint16:
					return static_cast<T>(u16);
				case uint32:
					return static_cast<T>(u32);
				case uint64:
					return static_cast<T>(u64);

				case int32:
					return static_cast<T>(i32);
				case int64:
					return static_cast<T>(i64);

				case f32:
					return static_cast<T>(this->f32);
				case f64:
					return static_cast<T>(this->f64);

				default:
					break;
			}
		}

		if constexpr (std::is_same<T, bool>::value)
		{
			switch(type)
			{
				using enum telemetry_type;

				case boolean:
					return b;

				case uint8:
					return u8;
				case uint16:
					return u16;
				case uint32:
					return u32;
				case uint64:
					return u64;

				case int32:
					return i32;
				case int64:
					return i64;

				case f32:
					return this->f32;
				case f64:
					return this->f64;

				default:
					break;
			}
		}

		if constexpr (std::is_same<T, std::string>::value)
		{
			switch(type)
			{
				using enum telemetry_type;

				case string:
					return this->string;

				default:
					break;
			}
		}
		if constexpr (std::is_same<T, const char *>::value)
		{
			switch(type)
			{
				using enum telemetry_type;

				case string:
					return this->string.c_str();

				default:
					break;
			}
		}

		throw std::runtime_error("Unsupported type conversion");
	}

	template<class T>
	bool can_convert_to() const
	{
		try
		{
			get<T>();
			return true;
		}
		catch(...)
		{
			return false;
		}
	}
};

struct telemetry_data_point
{
	double timestamp;
	telemetry_data_value value;
};

#endif //TELEMETRY_DATA_H
