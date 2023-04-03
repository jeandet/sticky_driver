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
#include <config.h>

#include <pybind11/chrono.h>
#include <pybind11/iostream.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <filter.hpp>
#include <sticky.hpp>

namespace py = pybind11;

py::array_t<double> to_py_array(const std::vector<std::pair<double, double>>& input)
{
    py::array_t<double> samples { { std::size(input), 2UL } };

    auto proxy = samples.mutable_unchecked<2>();
    for (auto i = 0UL; i < std::size(input); i++)
    {
        proxy(i, 0) = input[i].first;
        proxy(i, 1) = input[i].second;
    }
    return samples;
}

PYBIND11_MODULE(sticky, m)
{
    m.doc() = "sticky module";
    m.attr("__version__") = STICKY_VERSION;

    py::class_<Sticky>(m, "Sticky")
        .def(py::init<const std::string&>())
        .def("open", &Sticky::open)
        .def("_get_data",
            [](const Sticky& s, std::size_t count)
            {
                auto data = s.get_data(count);
                return py::bytes { reinterpret_cast<char*>(data.data()), std::size(data) };
            })
        .def("measure",
            [](const Sticky& s, std::size_t samples_count, bool filter = false) {
                return to_py_array(s.measure(samples_count,filter));
            })
        .def("configured", &Sticky::configured)
        .def("opened", &Sticky::opened);
}
