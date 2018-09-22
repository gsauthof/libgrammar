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

#include <grammar/grammar.hh>
#include <grammar/tsort.hh>

#include <grammar/tap/tap.hh>
#include <grammar/xml/xsd.hh>
#include <grammar/xml/rng.hh>
#include <grammar/xml/capnp.hh>

#include <fstream>
#include <iostream>
#include <functional>
#include <array>

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

    print_fn(o, g);
  }

  INFO("comparing files: " << ref << " vs. " << out);
  if (bf::file_size(ref)) {
    // mmap of zero-length file fails as specified by POSIX ...
    auto r = ixxx::util::mmap_file(ref.generic_string());
    auto o = ixxx::util::mmap_file(out.generic_string());
    bool are_equal = std::equal(r.begin(), r.end(), o.begin(), o.end());
    if (!are_equal) {
      cerr << "Files are not equal: " << ref << " vs. " << out << "\n";
    }
    CHECK(are_equal);
  } else {
    CHECK(bf::file_size(out) == 0);
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

static void print_capnp(ostream &o, Grammar &g)
{
  grammar::asn1::add_terminals(g);
  g.init_links();

  grammar::xml::capnp::Printer p(o);
  p.set_comment(string());
  p << g;
  o << endl;
}

static void compare_capnp(const char *rel_filename_str)
{
  compare("capnp", print_capnp, rel_filename_str);
}



static const array<const char*, 5> filenames = {
"asn1/record.asn1",
"asn1/set.asn1",
"asn1/rfc3525-MEDIA-GATEWAY-CONTROL_mini.asn1",
"asn1/LDAP_simplified_mini.asn1",
"asn1/tap_3_12_strip.asn1"
};

template<typename F, typename C>
static void map_test(F f, const C &filenames)
{
  size_t i = 0;
  for (auto filename : filenames) {
      SECTION(string("compare-") + to_string(i)) {
          f(filename);
      }
      ++i;
  }
}

TEST_CASE("compare tag translate", "[asn1][cmp]")
{
    map_test(compare_tag_translate, filenames);
}
TEST_CASE("compare tag closure", "[asn1][cmp]")
{
    map_test(compare_tag_closure, filenames);
}
TEST_CASE("compare unreachable", "[asn1][cmp]")
{
    map_test(compare_unreachable, filenames);
}
TEST_CASE("compare primitive", "[asn1][cmp]")
{
    map_test(compare_primitive, filenames);
}
TEST_CASE("compare name map", "[asn1][cmp]")
{
    map_test(compare_name_map, filenames);
}
TEST_CASE("compare tsort", "[asn1][cmp]")
{
    map_test(compare_tsort, filenames);
}
TEST_CASE("compare xsd", "[asn1][cmp]")
{
    map_test(compare_xsd, filenames);
}
TEST_CASE("compare rng", "[asn1][cmp]")
{
    map_test(compare_rng, filenames);
}
TEST_CASE("compare capnp", "[asn1][cmp]")
{
    map_test(compare_capnp, filenames);
}

