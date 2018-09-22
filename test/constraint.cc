
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
#include <test/test.hh>

#include <grammar/grammar.hh>
#include <grammar/constraint/variant.hh>

#include <boost/filesystem.hpp>

using namespace std;

struct Access_Pattern {
  using result_type = string;

  string operator()(const grammar::Constraint::Base &b) const { return "base"; }
  string operator()(const grammar::Constraint::Pattern &b) const { return b.str(); }
};


    TEST_CASE("constraint " "basic", "[constraint]")
    {
      boost::filesystem::path cs_file(test::path::in());
      cs_file /= "../../grammar/xml/tap_3_12_constraints.zsv";

      std::unordered_map<std::string, grammar::Constraint::Variant> m(
          grammar::make_constraint_map(
            grammar::read_constraints(cs_file.generic_string())));


      auto r = m.at("CalledNumber").accept(Access_Pattern());
      CHECK(r == "[0-9]+");

      auto s = m.at("ChargedPartyIdType").accept(Access_Pattern());
      CHECK(s == "base");

    }

    TEST_CASE("constraint " "tadig", "[constraint]")
    {
      boost::filesystem::path cs_file(test::path::in());
      cs_file /= "../../grammar/xml/tadig_codes.zsv";

      auto v = grammar::read_constraints(cs_file.generic_string());

      REQUIRE(v.enums.at("Sender").values().size() > 2);
      REQUIRE(v.enums.at("Recipient").values().size() > 2);

      CHECK(v.enums.at("Sender").values()[0] == "AAAOA");
      CHECK(v.enums.at("Recipient").values()[1] == "AAAOW");

    }



