/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "elementmap.h"

#include "containers.h"

#include "tupletmap.h"
#include "tiemap.h"
#include "slur.h"
#include "chordrest.h"

namespace mu::engraving {
EngravingItem* ElementMap::findNew(EngravingItem* o) const
{
    return mu::value(*this, o, nullptr);
}

void ElementMap::add(EngravingItem* o, EngravingItem* n)
{
    insert_or_assign(o, n);
}

//---------------------------------------------------------
//   findNew
//---------------------------------------------------------

Tuplet* TupletMap::findNew(Tuplet* o)
{
    for (const Tuplet2& t2 : map) {
        if (t2.o == o) {
            return t2.n;
        }
    }
    return 0;
}
}
