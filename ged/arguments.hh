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
#ifndef GED_ARGUMENTS_HH
#define GED_ARGUMENTS_HH

#include <string>
#include <deque>
#include <stdexcept>

namespace ged {

  class Argument_Error : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };

  void evaluate_arguments(int argc, char **argv);


}


#endif
