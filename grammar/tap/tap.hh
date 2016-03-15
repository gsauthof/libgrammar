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
#ifndef GRAMMAR_TAP_HH
#define GRAMMAR_TAP_HH

#include <utility>
#include <stdint.h>

namespace grammar {

  namespace tap {

    std::pair<const char * const*, const char * const*> well_known_tags();

    enum Tag : uint32_t  {
      TRANSFER_BATCH                 = 1,
      NETWORK_INFO                   = 6,
      CALL_EVENT_DETAIL_LIST         = 3,
      AUDIT_CONTROL_INFO             = 15,
      CHARGE_TYPE                    = 71,
      CHARGE_REFUND_INDICATOR        = 344,
      CAMEL_INVOCATION_FEE           = 422,
      CHARGE                         = 62,
      UTC_TIME_OFFSET_CODE           = 232,
      UTC_TIME_OFFSET                = 231,
      CHARGING_TIME_STAMP            = 74,
      CHARGE_DETAIL_TIME_STAMP       = 410,
      CALL_EVENT_START_TIME_STAMP    = 44,
      LOCAL_TIME_STAMP               = 16,
      DEPOSIT_TIME_STAMP             = 88,
      COMPLETION_TIME_STAMP          = 76,
      CHARGING_POINT                 = 73,
      CONTENT_TRANSACTION_BASIC_INFO = 304,
      ORDER_PLACED_TIME_STAMP        = 300,
      REQUESTED_DELIVERY_TIME_STAMP  = 301,
      ACTUAL_DELIVERY_TIME_STAMP     = 302,
      CONTENT_SERVICE_USED           = 352,
      CONTENT_CHARGING_POINT         = 345,
      SERVICE_START_TIME_STAMP       = 447,
      TAX_VALUE                      = 397
    };

  }

  namespace rap {
    enum Tag : uint32_t  {
      RETURN_BATCH           = 534,
      RETURN_DETAIL_LIST     = 536,
      RAP_AUDIT_CONTROL_INFO = 541
    };

  }

  namespace nrt {
    enum Tag : uint32_t  {
      NRTRDE            = 1,
      CALL_EVENT_LIST   = 2,
      CALL_EVENTS_COUNT = 20
    };
  }

}

#endif
