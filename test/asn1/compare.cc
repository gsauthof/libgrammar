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

#include <asn1/mini_parser.hh>
#include <asn1/grammar.hh>

#include <grammar.hh>
#include <tsort.hh>

#include <tap/tap.hh>
#include <xml/xsd.hh>
#include <xml/rng.hh>

#include <fstream>
#include <iostream>
#include <functional>

using namespace std;
using namespace grammar::asn1::mini;
using namespace grammar;
namespace bf = boost::filesystem;

static void compare(const char *suffix,
    std::function<void(ostream &, Grammar &g)> print_fn,
    const char *rel_filename_str)
{
  bf::path rel_filename(rel_filename_str);
  bf::path in(test::path::in());
  in /= rel_filename;

  bf::path out(test::path::out());
  out /= suffix;
  out /= rel_filename;
  out.replace_extension("out");

  bf::path ref(test::path::ref());
  ref /= suffix;
  ref /= rel_filename;
  ref.replace_extension("out");

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

    print_fn(o, g);
  }

  BOOST_TEST_CHECKPOINT("comparing files: " << ref << " vs. " << out);
  if (bf::file_size(ref)) {
    // mmap of zero-length file fails as specified by POSIX ...
    ixxx::util::RO_Mapped_File r(ref.generic_string());
    ixxx::util::RO_Mapped_File o(out.generic_string());
    bool are_equal = std::equal(r.begin(), r.end(), o.begin(), o.end());
    if (!are_equal) {
      cerr << "Files are not equal: " << ref << " vs. " << out << "\n";
    }
    BOOST_CHECK(are_equal);
  } else {
    BOOST_CHECK(!bf::file_size(out));
  }
}

static void print_app_tag_translations(ostream &o, Grammar &g)
{
  print_tag_translations(o, g, 1);
}

static void compare_tag_translate(const char *rel_filename_str)
{
  compare("tag_translate", print_app_tag_translations, rel_filename_str);
}

static void print_tag_closures(ostream &o, Grammar &g)
{
  grammar::asn1::add_terminals(g);
  g.init_links();
  derive_tags(g);

  print_tag_closure(o, g, "OCTET STRING" ,1);
  print_tag_closure(o, g, "INTEGER" ,1);
  auto p = grammar::tap::well_known_tags();
  for (auto i = p.first; i != p.second; ++i) {
    if (g.contains_symbol(*i)) {
      print_tag_closure(o, g, *i, 1);
    }
  }
}

static void compare_tag_closure(const char *rel_filename_str)
{
  compare("tag_closure", print_tag_closures, rel_filename_str);
}

static void pu(ostream &o, Grammar &g)
{
  grammar::asn1::add_terminals(g);
  g.init_links();
  print_unreachable_symbols(o, g);
}

static void compare_unreachable(const char *rel_filename_str)
{
  compare("unreachable", pu, rel_filename_str);
}

static void pp(ostream &o, Grammar &g)
{
  grammar::asn1::add_terminals(g);
  g.init_links();
  print_primitive_tags(o, g, 1);
}

static void compare_primitive(const char *rel_filename_str)
{
  compare("primitive", pp, rel_filename_str);
}

static void print_name_map(ostream &o, Grammar &g)
{
  grammar::asn1::add_terminals(g);
  g.init_links();
  erase_unreachable_symbols(g);
  derive_tags(g);
  print_name_to_shape_klasse_tag_map(o, g);
}

static void compare_name_map(const char *rel_filename_str)
{
  compare("name_map", print_name_map, rel_filename_str);
}

static void ts(ostream &o, Grammar &g)
{
  grammar::asn1::add_terminals(g);
  g.init_links();
  try {
    tsort(g);
  } catch (const std::invalid_argument &e) {
  }
  grammar::asn1::Printer p(o);
  p << g;
  o << endl;
}

static void compare_tsort(const char *rel_filename_str)
{
  compare("tsort", ts, rel_filename_str);
}

static void print_xsd(ostream &o, Grammar &g)
{
  grammar::asn1::add_terminals(g);
  g.init_links();

  grammar::xml::xsd::Printer p(o);
  p.set_any_attribute(false);
  p.set_comment(string());
  p << g;
  o << endl;
}

static void compare_xsd(const char *rel_filename_str)
{
  compare("xsd", print_xsd, rel_filename_str);
}

static void print_rng(ostream &o, Grammar &g)
{
  grammar::asn1::add_terminals(g);
  g.init_links();

  grammar::xml::rng::Printer p(o);
  p.set_comment(string());
  p << g;
  o << endl;
}

static void compare_rng(const char *rel_filename_str)
{
  compare("rng", print_rng, rel_filename_str);
}

boost::unit_test::test_suite *create_asn1_compare_suite()
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
  auto cmp = BOOST_TEST_SUITE("cmp");
  auto tt = BOOST_TEST_SUITE("tag_translate");
  tt->add(BOOST_PARAM_TEST_CASE(&compare_tag_translate,
        filenames.begin(), filenames.end()));
  auto closure = BOOST_TEST_SUITE("tag_closure");
  closure->add(BOOST_PARAM_TEST_CASE(&compare_tag_closure,
        filenames.begin(), filenames.end()));
  auto unreachable = BOOST_TEST_SUITE("unreachable");
  unreachable->add(BOOST_PARAM_TEST_CASE(&compare_unreachable,
        filenames.begin(), filenames.end()));

  auto primitive = BOOST_TEST_SUITE("primitive");
  primitive->add(BOOST_PARAM_TEST_CASE(&compare_primitive,
        filenames.begin(), filenames.end()));

  auto name_map = BOOST_TEST_SUITE("name_map");
  name_map->add(BOOST_PARAM_TEST_CASE(&compare_name_map,
        filenames.begin(), filenames.end()));

  auto tsort = BOOST_TEST_SUITE("tsort");
  tsort->add(BOOST_PARAM_TEST_CASE(&compare_tsort,
        filenames.begin(), filenames.end()));
  auto xsd = BOOST_TEST_SUITE("xsd");
  xsd->add(BOOST_PARAM_TEST_CASE(&compare_xsd,
        filenames.begin(), filenames.end()));
  auto rng = BOOST_TEST_SUITE("rng");
  rng->add(BOOST_PARAM_TEST_CASE(&compare_rng,
        filenames.begin(), filenames.end()));
  grammar_->add(asn1_);
  asn1_->add(cmp);
  cmp->add(tt);
  cmp->add(closure);
  cmp->add(unreachable);
  cmp->add(primitive);
  cmp->add(name_map);
  cmp->add(tsort);
  cmp->add(xsd);
  cmp->add(rng);
  return grammar_;
}
