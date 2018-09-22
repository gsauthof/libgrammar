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

#include <catch2/catch.hpp>

#include <boost/filesystem.hpp>

#include <test/test.hh>

#include <ixxx/ixxx.hh>
#include <ixxx/util.hh>

#include <grammar/asn1/mini_parser.hh>
#include <grammar/asn1/grammar.hh>

#include <fstream>
#include <iostream>
#include <array>
#include <string>

using namespace std;
using namespace grammar::asn1::mini;
namespace bf = boost::filesystem;

static void compare_identity(const char *rel_filename_str)
{
  bf::path rel_filename(rel_filename_str);
  bf::path in(test::path::in());
  in /= rel_filename;

  bf::path out(test::path::out());
  out /= "identity";
  out /= rel_filename;

  CAPTURE(in);
  CAPTURE(out);

  INFO("remove: " << out);
  bf::remove(out);
  INFO("create directory: " << out.parent_path());
  bf::create_directories(out.parent_path());

  INFO("map file: " << in.generic_string());
  auto f = ixxx::util::mmap_file(in.generic_string());
  Parser parser;
  parser.read(f.s_begin(), f.s_end());

  {
  grammar::Grammar g(parser.grammar());
  ofstream o;
  o.exceptions(ofstream::failbit | ofstream::badbit);
  o.open(out.generic_string(), ofstream::out | ofstream::binary);

  grammar::asn1::Printer p(o);
  p << g;
  o << endl;
  }

  INFO("comparing files");
  auto o = ixxx::util::mmap_file(out.generic_string());
  bool are_equal = std::equal(f.begin(), f.end(), o.begin(), o.end());
  if (!are_equal) {
    FAIL_CHECK("Files are not equal: " << in << " vs. " << out);
  }
  CHECK(are_equal);
}

TEST_CASE("identity", "[asn1][id]")
{
  const array<const char*, 5> filenames = {
    "asn1/record.asn1",
    "asn1/set.asn1",
    "asn1/rfc3525-MEDIA-GATEWAY-CONTROL_mini.asn1",
    "asn1/LDAP_simplified_mini.asn1",
    "asn1/tap_3_12_strip.asn1"
  };
  size_t i = 0;
  for (auto filename : filenames) {
      SECTION(string("compare-") + to_string(i)) {
          compare_identity(filename);
      }
      ++i;
  }
}
