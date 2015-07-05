// Copyright 2015, Georg Sauthoff <mail@georg.so>

/* {{{ LGPLv3

    This file is part of libgrammar.

    libgrammar is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libgrammar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with libgrammar.  If not, see <http://www.gnu.org/licenses/>.

}}} */
#include "tap.hh"

using namespace std;

namespace grammar {

  namespace tap {


    static const char* const well_known_tags_[] = {
      "AbsoluteAmount",
      "AsciiString",
      "Bid",
      "BCDString",
      "ChargeType",
      "ChargedItem",
      "Currency",
      "DateTime",
      "DateTimeLong",
      "HexString",
      "LocalTimeStamp",
      "NumberString",
      "PercentageRate",
      "PlmnId",
      "UtcTimeOffset",
      "UtcTimeOffsetCode"
    };

    std::pair<const char * const*, const char * const*> well_known_tags()
    {
      return make_pair(well_known_tags_,
          well_known_tags_ + sizeof(well_known_tags_)/sizeof(const char*));
    }

  }

}
