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
#include "identity.hh"

#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/filesystem.hpp>

#include <test/test.hh>

#include <ixxx/ixxx.h>
#include <ixxx/util.hh>
#include <ixxx/util/boost.hh>


#include <grammar/asn1/mini_parser.hh>
#include <grammar/asn1/grammar.hh>

#include <fstream>
#include <iostream>

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

  BOOST_TEST_MESSAGE("in: " << in);
  BOOST_TEST_MESSAGE("out: " << out);

  BOOST_TEST_CHECKPOINT("remove: " << out);
  bf::remove(out);
  BOOST_TEST_CHECKPOINT("create directory: " << out.parent_path());
  bf::create_directories(out.parent_path());

  BOOST_TEST_CHECKPOINT("map file: " << in.generic_string());
  ixxx::util::RO_Mapped_File f(in.generic_string());
  Parser parser;
  parser.read(f.sbegin(), f.send());

  {
  grammar::Grammar g(parser.grammar());
  ofstream o;
  o.exceptions(ofstream::failbit | ofstream::badbit);
  o.open(out.generic_string(), ofstream::out | ofstream::binary);

  grammar::asn1::Printer p(o);
  p << g;
  o << endl;
  }

  BOOST_TEST_CHECKPOINT("comparing files");
  ixxx::util::RO_Mapped_File o(out.generic_string());
  bool are_equal = std::equal(f.begin(), f.end(), o.begin(), o.end());
  if (!are_equal) {
    cerr << "Files are not equal: " << in << " vs. " << out << '\n';
  }
  BOOST_CHECK(are_equal);
}

boost::unit_test::test_suite *create_asn1_id_suite()
{
  const array<const char*, 5> filenames = {
    "asn1/record.asn1",
    "asn1/set.asn1",
    "asn1/rfc3525-MEDIA-GATEWAY-CONTROL_mini.asn1",
    "asn1/LDAP_simplified_mini.asn1",
    "asn1/tap_3_12_strip.asn1"
  };
  auto grammar_ = BOOST_TEST_SUITE("grammar_");
  auto asn1_ = BOOST_TEST_SUITE("asn1_");
  auto asn1_id = BOOST_TEST_SUITE("identity");
  asn1_id->add(BOOST_PARAM_TEST_CASE(&compare_identity,
        filenames.begin(), filenames.end()));
  grammar_->add(asn1_);
  asn1_->add(asn1_id);
  return grammar_;
}
