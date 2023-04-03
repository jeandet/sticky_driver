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
#include <array>
#include <config.h>
#include <optional>
#include <string>
#include <vector>
#pragma once

class Filter
{
    /*
     * generated with:
     *
     * from scipy import signal
     * sos=signal.iirfilter(6, 0.25, rs=80, btype='lowpass',analog=False,
     * ftype='cheby2',output='sos') print(f""" const double b[{sos.shape[0]}][3] = {{{ ', '.join([
     * '{' + ', '.join(list(map(str , stage))) + '}' for stage in sos[:,:3] ])}}}; const double
     * a[{sos.shape[0]}][3] = {{{ ', '.join([ '{' + ', '.join(list(map(str , stage))) + '}' for
     * stage in sos[:,3:] ])}}};
     * """)
     */
    const double b[3][3]
        = { { 0.0003093761776877881, 0.00027126310014594703, 0.00030937617768778814 },
              { 1.0, -0.9780833528217364, 1.0 }, { 1.0, -1.3786886998937251, 1.0 } };
    const double a[3][3] = { { 1.0, -1.449543617902121, 0.5298911166658338 },
        { 1.0, -1.570227988783793, 0.6515750588208723 },
        { 1.0, -1.7779954896683987, 0.8644540496942458 } };
    double ctx[3][3] = { { 0. } };

public:
    inline explicit Filter() { }
    inline ~Filter() { }

    double filter(double x)
    {
        // Direct-Form-II
        for (int i = 0; i < 3; i++)
        {
            double W = (x - (a[i][1] * ctx[i][0]) - (a[i][2] * ctx[i][1]));
            x = (b[i][0] * W) + (b[i][1] * ctx[i][0]) + (b[i][2] * ctx[i][1]);
            ctx[i][1] = ctx[i][0];
            ctx[i][0] = W;
        }
        return x;
    }
};
