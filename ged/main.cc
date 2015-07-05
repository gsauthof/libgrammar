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
#include "arguments.hh"

#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
  try {
    ged::evaluate_arguments(argc, argv);
  } catch (std::exception &e) {
    cerr << "Error: " << e.what() << '\n';
    return 1;
  } catch (...) {
    cerr << "Got non-std::exception exception\n";
    return 1;
  }
  return 0;
}
