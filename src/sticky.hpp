/*------------------------------------------------------------------------------
-- This file is a part of the Sticky library
-- Copyright (C) 2023, Plasma Physics Laboratory - CNRS
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
-------------------------------------------------------------------------------*/
/*-- Author : Alexis Jeandet
-- Mail : alexis.jeandet@member.fsf.org
----------------------------------------------------------------------------*/
#pragma once
#include <algorithm>
#include <array>
#include <config.h>
#include <optional>
#include <string>
#include <tuple>
#include <vector>
#include <cstdint>

#include "filter.hpp"


class Sticky
{
    std::string _port_name;
    std::array<double, 2> _calibration_factor;
    int _fd = -1;

    bool _configured = false;

    std::vector<uint8_t> _read(std::size_t count) const;

    template <typename T>
    static inline std::optional<std::size_t> find_shift(const T& buff)
    {
        for (auto shift = 0UL; shift < std::min(1500UL, std::size(buff)); shift++)
        {
            bool shift_is_valid = true;
            for (auto i = shift; i < std::min(shift + 5000, std::size(buff) - 5); i += 5)
            {
                if (static_cast<uint8_t>(buff[i + 5])
                    != ((static_cast<uint8_t>(buff[i]) + 1) % 256))
                {
                    shift_is_valid = false;
                    break;
                }
            }
            if (shift_is_valid)
                return shift;
        }
        return std::nullopt;
    }

    template <typename T>
    static inline std::size_t samples_count(std::size_t offset, const T& buff)
    {
        return (std::size(buff) - offset) / 5;
    }


    template <typename T, typename U>
    static inline std::vector<std::pair<double, double>> extract_samples(
        const T& buff, std::size_t count, const U& calibration_factors)
    {
        const auto maybe_offset = find_shift(buff);
        if (maybe_offset)
        {
            const auto offset = *maybe_offset;
            auto samples = std::vector<std::pair<double, double>>(
                std::min(samples_count(offset, buff), count));
            for (auto i = 0UL; i < std::size(samples); i++)
            {
                std::int16_t sample_a = static_cast<uint8_t>(buff[offset + (i * 5) + 1]) * 256
                    + static_cast<uint8_t>(buff[offset + (i * 5) + 2]);
                std::int16_t sample_b = static_cast<uint8_t>(buff[offset + (i * 5) + 3]) * 256
                    + static_cast<uint8_t>(buff[offset + (i * 5) + 4]);
                samples[i].first = sample_a * calibration_factors[0];
                samples[i].second = sample_b * calibration_factors[1];
            }
            return samples;
        }
        return {};
    }

    static inline auto filter(
        std::vector<std::pair<double, double>>&& input, std::size_t margin = 1000)
    {
        auto samples = std::vector<std::pair<double, double>>((std::size(input) - 2 * margin) / 4);
        { // forward
            Filter filter_a;
            Filter filter_b;
            std::transform(std::cbegin(input), std::cend(input), std::begin(input),
                [&](const std::pair<double, double>& e) -> std::pair<double, double> {
                    return { filter_a.filter(e.first), filter_b.filter(e.second) };
                });
        }
        { // backward
            Filter filter_a;
            Filter filter_b;
            std::transform(std::crbegin(input), std::crend(input), std::rbegin(input),
                [&](const std::pair<double, double>& e) -> std::pair<double, double> {
                    return { filter_a.filter(e.first), filter_b.filter(e.second) };
                });
        }
        { // decimate
            for (auto i = 0UL; i < std::size(samples); i++)
            {
                for (auto j = (margin + (i * 4)); j < (margin + (i * 4) + 4); j++)
                {
                    samples[i].first += input[j].first / 4.;
                    samples[i].second += input[j].second / 4.;
                }
            }
        }
        return samples;
    }

public:
    inline explicit Sticky(const std::string& port_name,
        std::array<double, 2> calibration_factor = { 0.0001315962626661403, 0.0001315962626661403 })
            : _port_name { port_name }, _calibration_factor { calibration_factor }
    {
        this->open();
    }
    inline ~Sticky() { this->close(); }

    bool open(const std::optional<std::string> port_name = std::nullopt);
    bool close();

    std::vector<uint8_t> get_data(std::size_t count) const;
    inline std::vector<std::pair<double, double>> measure(
        std::size_t count, bool filter = false) const
    {
        if (filter)
            return Sticky::filter(
                Sticky::extract_samples(this->get_data((4 * count + 2000) * 5 + 100),
                    4 * count + 2000, this->_calibration_factor));
        else
            return Sticky::extract_samples(
                this->get_data(count * 5 + 100), count, this->_calibration_factor);
    }

    inline const std::string& port_name() const { return this->_port_name; }

    inline const auto& calibration_factor() const { return this->_calibration_factor; }

    inline bool configured() const { return this->_configured; }
    inline bool opened() const { return this->_fd != -1; }
};
