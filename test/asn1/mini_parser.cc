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
#include <boost/test/unit_test.hpp>

#include <ostream>
#include <map>
#include <fstream>

#include <boost/filesystem.hpp>

#include <grammar/asn1/mini_parser.hh>
#include <grammar/asn1/grammar.hh>
#include <grammar/xml/xsd.hh>
#include <grammar/xml/rng.hh>
#include <test/test.hh>

#include <ixxx/ixxx.h>
#include <ixxx/util.hh>
#include <ixxx/util/boost.hh>


using namespace std;
namespace bf = boost::filesystem;



BOOST_AUTO_TEST_SUITE(grammar_)


  BOOST_AUTO_TEST_SUITE(asn1_)

    BOOST_AUTO_TEST_SUITE(mini_parser)

      // {{{

      using namespace grammar::asn1::mini;


      BOOST_AUTO_TEST_CASE(begin_end)
      {
        const char inp[] =
          "ABC-0123 DEFINITIONS IMPLICIT TAGS ::=\n"
          "BEGIN\n"
          "Root ::= SEQUENCE\n"
          "{\n"
          "  s OCTET STRING,\n"
          "  e Element\n"
          "}\n"
          "Element ::= SEQUENCE\n"
          "{\n"
          "  i INTEGER,\n"
          "  x OCTET STRING\n"
          "}\n"
          "END\n";
        Parser parser;
        parser.read(inp, inp + sizeof(inp)-1);
        grammar::Grammar g(parser.grammar());
        BOOST_CHECK_EQUAL(g.nts().size(), 2u);
      }

      BOOST_AUTO_TEST_CASE(normalize_ws_in_symbols)
      {
        const char inp[] =
          "ABC-0123 DEFINITIONS IMPLICIT TAGS ::=\n"
          "BEGIN\n"
          "Root ::= SEQUENCE\n"
          "{\n"
          "  s1 OCTET STRING,\n"
          "  s2 OCTET  STRING,\n"
          "  s3 OCTET   STRING,\n"
          "  s4 BIT STRING,\n"
          "  s5 BIT  STRING,\n"
          "  e Element\n"
          "}\n"
          "Element ::= SEQUENCE\n"
          "{\n"
          "  i INTEGER,\n"
          "  x OCTET STRING\n"
          "}\n"
          "END\n";
        Parser parser;
        parser.read(inp, inp + sizeof(inp)-1);
        grammar::Grammar g(parser.grammar());
        BOOST_CHECK_EQUAL(g.nts().size(), 2u);
        BOOST_CHECK_EQUAL(g.name_to_nt("Root").rule().refs().at(0).symbol_name(), "OCTET STRING");
        BOOST_CHECK_EQUAL(g.name_to_nt("Root").rule().refs().at(1).symbol_name(), "OCTET STRING");
        BOOST_CHECK_EQUAL(g.name_to_nt("Root").rule().refs().at(2).symbol_name(), "OCTET STRING");
        BOOST_CHECK_EQUAL(g.name_to_nt("Root").rule().refs().at(3).symbol_name(), "BIT STRING");
        BOOST_CHECK_EQUAL(g.name_to_nt("Root").rule().refs().at(4).symbol_name(), "BIT STRING");
      }

      BOOST_AUTO_TEST_CASE(minimal_ws)
      {
        const char inp[] =
          "ABC-0123 DEFINITIONS IMPLICIT TAGS::="
          "BEGIN\n"
          "Root::=SEQUENCE"
          "{"
          "s OCTET STRING,"
          "e Element"
          "}\n"
          "Element::=SEQUENCE"
          "{"
          "i INTEGER,"
          "x OCTET STRING"
          "}\n"
          "END";
        Parser parser;
        parser.read(inp, inp + sizeof(inp)-1);
        grammar::Grammar g(parser.grammar());
        BOOST_CHECK_EQUAL(g.nts().size(), 2u);
      }

      BOOST_AUTO_TEST_CASE(rt_error)
      {
        const char inp[] =
          "Root::=SEQUENCE"
          "{"
          "s OCTET STRING,"
          "e Element"
          "}\n"
          "Element:=SEQUENCE"
          "{"
          "i INTEGER,"
          "x OCTET STRING"
          "}\n"
          ;
        Parser parser;
        BOOST_CHECK_THROW(parser.read(inp, inp + sizeof(inp)-1), std::runtime_error);
      }

      BOOST_AUTO_TEST_CASE(dash_comments)
      {
        const char inp[] =
          "Root ::= ---- SEQUENCE\n"
          "{\n"
          // not in between identifier ...
          //"  s OCTET -- Foo Bar -- STRING,\n"
          "  s -- Foo Bar -- OCTET  STRING,\n"
          "  e Element\n"
          "}\n"
          "Element ::= SEQUENCE -- until the end\n"
          "-- blah blah\n"
          "{\n"
          "--xyz--  i INTEGER,\n"
          "  x OCTET STRING\n"
          "}\n"
          ;
        Parser parser;
        parser.read(inp, inp + sizeof(inp)-1);
        grammar::Grammar g(parser.grammar());
        BOOST_CHECK_EQUAL(g.nts().size(), 2u);
      }

      BOOST_AUTO_TEST_CASE(comment_error)
      {
        const char inp[] =
          "Root ::= SEQUENCE\n"
          "{\n"
          "  s OCTET STRING,\n"
          "  e Element\n"
          "}\n"
          // a zero length comment followed by a dash
          "Element ::= ----- SEQUENCE\n"
          "{\n"
          "  i INTEGER,\n"
          "  x OCTET STRING\n"
          "}\n"
          ;
        Parser parser;
        BOOST_CHECK_THROW(parser.read(inp, inp + sizeof(inp)-1), std::runtime_error);
      }

      BOOST_AUTO_TEST_CASE(error_msg)
      {
        const char inp[] =
          "Root ::= SEQUENCE\n"
          "{\n"
          "  s OCTET STRING,\n"
          "  e Element\n"
          "}\n"
          "Element := SEQUENCE\n"
          "{\n"
          "  i INTEGER,\n"
          "  x OCTET STRING\n"
          "}\n"
          ;
        Parser parser;
        string msg;
        try {
          parser.read(inp, inp + sizeof(inp)-1);
        } catch (const grammar::Parse_Error &e) {
          msg = e.what();
        }
        BOOST_CHECK(msg.find("\nElement := SEQUENCE\n") != msg.npos);
        BOOST_CHECK(msg.find("i INTEGER") == msg.npos);
        BOOST_CHECK(msg.find("e Element") == msg.npos);
      }

      BOOST_AUTO_TEST_CASE(location)
      {
        const char inp[] =
          "Root ::= SEQUENCE\n"
          "{\n"
          "  s OCTET STRING,\n"
          "  e Element --\n"
          "}\n"
          "Element := SEQUENCE\n" // error!
          "{\n"
          "  i INTEGER,\n"
          "  x OCTET STRING\n"
          "}\n"
          ;
        Parser parser("input.asn1");
        string msg;
        try {
          parser.read(inp, inp + sizeof(inp)-1);
        } catch (const grammar::Parse_Error &e) {
          msg = e.what();
        }
        BOOST_CHECK(msg.find("input.asn1:6:10: error: parse error") != msg.npos);
        BOOST_CHECK(msg.find("\n         ^") != msg.npos);
      }

      BOOST_AUTO_TEST_CASE(constraints)
      {
        const char inp[] =
          "Root ::= SEQUENCE\n"
          "{\n"
          "  s1 OCTET STRING SIZE(12..23),\n"
          "  s2 OCTET STRING (SIZE(12..23)),\n"
          "  s3 OCTET STRING (SIZE( 12 .. 23)),\n"
          "  s4 OCTET STRING ( SIZE(12 .. 23 ) ),\n"
          "  s5 OCTET STRING (SIZE(23)),\n"
          "  s6 OCTET STRING(SIZE(23)),\n"
          "  s7 OCTET STRING SIZE(23),\n"
          "  e Element --\n"
          "}\n"
          "Element ::= SEQUENCE\n"
          "{\n"
          "  i1 INTEGER,\n"
          "  i2 INTEGER(1..32),\n"
          "  i3 INTEGER (1..32),\n"
          "  i4 INTEGER (1 ..  32),\n"
          "  x OCTET STRING\n"
          "}\n"
          ;
        Parser parser;
        parser.read(inp, inp + sizeof(inp)-1);
        grammar::Grammar g(parser.grammar());
        BOOST_CHECK_EQUAL(g.nts().size(), 2u);
      }

      BOOST_AUTO_TEST_CASE(links)
      {
        bf::path in(test::path::in());
        in /= "asn1/record.asn1";
        ixxx::util::RO_Mapped_File f(in.generic_string());
        Parser parser;
        parser.read(f.sbegin(), f.send());
        grammar::Grammar g(parser.grammar());
        BOOST_CHECK_THROW(
            g.name_to_nt("RecordCount").rule().refs().front().symbol(),
            grammar::Runtime_Error);
        using namespace grammar;
        g.push(make_unique<Symbol::Terminal>("INTEGER"));
        g.push(make_unique<Symbol::Terminal>("OCTET STRING"));
        g.init_links();
        BOOST_CHECK_EQUAL(
            g.name_to_nt("RecordCount").rule().refs().front().symbol().name(),
            "INTEGER");
      }


      BOOST_AUTO_TEST_CASE(tag_translation)
      {
        bf::path in(test::path::in());
        in /= "asn1/record.asn1";
        ixxx::util::RO_Mapped_File f(in.generic_string());
        Parser parser;
        parser.read(f.sbegin(), f.send());
        grammar::Grammar g(parser.grammar());
        using namespace grammar;
        g.push(make_unique<Symbol::Terminal>("INTEGER"));
        g.push(make_unique<Symbol::Terminal>("OCTET STRING"));
        g.init_links();
        derive_tags(g);
        auto t = tag_translations(g, 1);
        map<uint32_t, string> s(t.begin(), t.end());
        const char ref[] = 
  "1,Document\n"
  "2,Records\n"
  "3,Record\n"
  "4,Header\n"
  "5,Footer\n"
  "123,Location\n"
  "124,From\n"
  "125,To\n"
  ;
        ostringstream o;
        for (auto &x : s) {
          o << x.first << ',' << x.second << '\n';
        }
        BOOST_CHECK_EQUAL(o.str(), ref);
      }

      BOOST_AUTO_TEST_CASE(tag_translation_uni)
      {
        bf::path in(test::path::in());
        in /= "asn1/record.asn1";
        ixxx::util::RO_Mapped_File f(in.generic_string());
        Parser parser;
        parser.read(f.sbegin(), f.send());
        grammar::Grammar g(parser.grammar());
        using namespace grammar;
        g.push(make_unique<Symbol::Terminal>("INTEGER", Coordinates(2, 0)));
        g.push(make_unique<Symbol::Terminal>("OCTET STRING", Coordinates(4, 0)));
        g.init_links();
        derive_tags(g);
        auto t = tag_translations(g, 0);
        map<uint32_t, string> s(t.begin(), t.end());
        const char ref[] = 
  "2,INTEGER\n"
  "4,OCTET STRING\n"
  ;
        ostringstream o;
        for (auto &x : s) {
          o << x.first << ',' << x.second << '\n';
        }
        BOOST_CHECK_EQUAL(o.str(), ref);
      }

      // }}}


      BOOST_AUTO_TEST_CASE(ignore_duplicates)
      {
        grammar::Grammar g;
        {
          bf::path in(test::path::in());
          in /= "asn1/first_half.asn1";
          ixxx::util::Mapped_File f(in.generic_string());
          grammar::asn1::mini::Parser parser(std::move(g));
          parser.read(f.s_begin(), f.s_end());
          g = parser.grammar();
        }
        g.set_ignore_duplicates(true);
        {
          bf::path in(test::path::in());
          in /= "asn1/second_half.asn1";
          ixxx::util::Mapped_File f(in.generic_string());
          grammar::asn1::mini::Parser parser(std::move(g));
          parser.read(f.s_begin(), f.s_end());
          g = parser.grammar();
        }
        grammar::asn1::add_terminals(g);
        g.init_links();
        BOOST_CHECK_EQUAL(g.name_to_nt("Record").rule().refs().size(), 3u);
        BOOST_CHECK_EQUAL(g.name_to_nt("Record").parents().size(), 1u);
        BOOST_CHECK_EQUAL(g.name_to_nt("Header").rule().refs().size(), 1u);
        BOOST_CHECK_EQUAL(g.nts().size(), 12u);
      }

      BOOST_AUTO_TEST_CASE(add_keys_constraints)
      {
        grammar::Grammar g;
        bf::path in(test::path::in());
        in /= "asn1/tap_3_12_strip.asn1";
        bf::path key_file(test::path::in());
        key_file /= "../../grammar/xml/tap_3_12_keys.xml";
        bf::path cs_file(test::path::in());
        cs_file /= "../../grammar/xml/tap_3_12_constraints.zsv";
        string rel_out("xsd/asn1/key_const.xml");
        bf::path out(test::path::out());
        out /= rel_out;
        BOOST_TEST_CHECKPOINT("Removing: " << out);
        bf::remove(out);
        bf::create_directories(out.parent_path());
        bf::path ref(test::path::ref());
        ref /= rel_out;
        BOOST_TEST_CHECKPOINT("Parsing: " << in );
        {
          ixxx::util::Mapped_File f(in.generic_string());
          grammar::asn1::mini::Parser parser(std::move(g));
          parser.read(f.s_begin(), f.s_end());
          g = parser.grammar();
          grammar::asn1::add_terminals(g);
          g.init_links();
        }
        BOOST_TEST_CHECKPOINT("Reading: " << key_file << ' ' << cs_file);
        {
          ofstream f(out.generic_string(), ios_base::out | ios_base::binary);
          grammar::xml::xsd::Printer p(f);
          p.set_any_attribute(false);
          p.set_comment(string());
          p.read_key_def("TransferBatch", key_file.generic_string());
          grammar::add_constraints(g, cs_file.generic_string());
          p << g;
        }
        BOOST_TEST_CHECKPOINT("Comparing: " << ref << " vs. " << out);
        {
          ixxx::util::Mapped_File a(ref.generic_string());
          ixxx::util::Mapped_File b(out.generic_string());
          BOOST_REQUIRE(bf::file_size(ref) && bf::file_size(out));
          bool are_equal = std::equal(a.begin(), a.end(), b.begin(), b.end());
          BOOST_CHECK(are_equal);
        }
      }

      BOOST_AUTO_TEST_CASE(add_rng_constraints)
      {
        grammar::Grammar g;
        bf::path in(test::path::in());
        in /= "asn1/tap_3_12_strip.asn1";
        bf::path cs_file(test::path::in());
        cs_file /= "../../grammar/xml/tap_3_12_constraints.zsv";
        string rel_out("rng/asn1/const.xml");
        bf::path out(test::path::out());
        out /= rel_out;
        BOOST_TEST_CHECKPOINT("Removing: " << out);
        bf::remove(out);
        bf::create_directories(out.parent_path());
        bf::path ref(test::path::ref());
        ref /= rel_out;
        BOOST_TEST_CHECKPOINT("Parsing: " << in );
        {
          ixxx::util::Mapped_File f(in.generic_string());
          grammar::asn1::mini::Parser parser(std::move(g));
          parser.read(f.s_begin(), f.s_end());
          g = parser.grammar();
          grammar::asn1::add_terminals(g);
          g.init_links();
        }
        BOOST_TEST_CHECKPOINT("Reading: " << cs_file);
        {
          ofstream f(out.generic_string(), ios_base::out | ios_base::binary);
          grammar::xml::xsd::Printer p(f);
          p.set_any_attribute(false);
          p.set_comment(string());
          grammar::add_constraints(g, cs_file.generic_string());
          p << g;
        }
        BOOST_TEST_CHECKPOINT("Comparing: " << ref << " vs. " << out);
        {
          ixxx::util::Mapped_File a(ref.generic_string());
          ixxx::util::Mapped_File b(out.generic_string());
          BOOST_REQUIRE(bf::file_size(ref) && bf::file_size(out));
          bool are_equal = std::equal(a.begin(), a.end(), b.begin(), b.end());
          BOOST_CHECK(are_equal);
        }
      }

      BOOST_AUTO_TEST_CASE(any_attribute)
      {
        grammar::Grammar g;
        bf::path in(test::path::in());
        in /= "asn1/tap_3_12_strip.asn1";
        string rel_out("xsd/asn1/any_attribute.xml");
        bf::path out(test::path::out());
        out /= rel_out;
        BOOST_TEST_CHECKPOINT("Removing: " << out);
        bf::remove(out);
        bf::create_directories(out.parent_path());
        bf::path ref(test::path::ref());
        ref /= rel_out;
        BOOST_TEST_CHECKPOINT("Parsing: " << in );
        {
          ixxx::util::Mapped_File f(in.generic_string());
          grammar::asn1::mini::Parser parser(std::move(g));
          parser.read(f.s_begin(), f.s_end());
          g = parser.grammar();
          grammar::asn1::add_terminals(g);
          g.init_links();
        }
        BOOST_TEST_CHECKPOINT("Writing: " << out);
        {
          ofstream f(out.generic_string(), ios_base::out | ios_base::binary);
          grammar::xml::xsd::Printer p(f);
          p.set_any_attribute(true);
          p.set_comment(string());
          p << g;
        }
        BOOST_TEST_CHECKPOINT("Comparing: " << ref << " vs. " << out);
        {
          ixxx::util::Mapped_File a(ref.generic_string());
          ixxx::util::Mapped_File b(out.generic_string());
          BOOST_REQUIRE(bf::file_size(ref) && bf::file_size(out));
          bool are_equal = std::equal(a.begin(), a.end(), b.begin(), b.end());
          BOOST_CHECK(are_equal);
        }
      }


    BOOST_AUTO_TEST_SUITE_END() // mini_parser

  BOOST_AUTO_TEST_SUITE_END() //asn1_


BOOST_AUTO_TEST_SUITE_END() // grammar_
