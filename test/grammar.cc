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
#include <sstream>
#include <grammar/grammar.hh>
#include <grammar/tsort.hh>
#include <grammar/asn1/grammar.hh>

using namespace std;



  using namespace grammar;


    TEST_CASE("grammar" "basic links", "[grammar][links]")
    {
      Grammar g;
      auto root = make_unique<Symbol::NT>("root");
      auto s = make_unique<Rule::Set>();
      s->push(Reference("OCTET STRING"));
      root->set_rule(std::move(s));
      g.push(std::move(root));
      g.push(make_unique<Symbol::Terminal>("OCTET STRING"));
      REQUIRE_THROWS_AS(
          g.name_to_nt("root").rule().refs().front()->symbol(),
          grammar::Runtime_Error);
      g.init_links();
      CHECK(
          g.name_to_nt("root").rule().refs().front()->symbol().name() ==
          "OCTET STRING");
    }

    TEST_CASE("grammar" "links missing_symbol", "[grammar][links]")
    {
      Grammar g;
      auto root = make_unique<Symbol::NT>("root");
      auto s = make_unique<Rule::Set>();
      s->push(Reference("OCTET STRING"));
      root->set_rule(std::move(s));
      g.push(std::move(root));
      REQUIRE_THROWS_AS(g.init_links(), grammar::Runtime_Error);
    }

    TEST_CASE("grammar" "derive tags is_set", "[grammar][derive]")
    {
      Grammar g;
      auto root = make_unique<Symbol::NT>("root");
      auto s = make_unique<Rule::Set>();
      s->push(Reference("OCTET STRING"));
      root->set_rule(std::move(s));
      g.push(std::move(root));
      g.push(make_unique<Symbol::Terminal>("OCTET STRING",
            Coordinates(23u, 1u)));
      g.init_links();
      CHECK(g.name_to_nt("root").coord().tag() == 0u);
      CHECK(g.name_to_nt("root").coord().klasse() == 0u);
      derive_tags(g);
      CHECK(g.name_to_nt("root").coord().tag() == 17u);
      CHECK(g.name_to_nt("root").coord().klasse() == 0u);
    }

    TEST_CASE("grammar" "basic derive tags", "[grammar][derive]")
    {
      Grammar g;
      auto root = make_unique<Symbol::NT>("root");
      auto s = make_unique<Rule::Link>();
      s->push(Reference("OCTET STRING"));
      root->set_rule(std::move(s));
      g.push(std::move(root));
      g.push(make_unique<Symbol::Terminal>("OCTET STRING",
            Coordinates(23u, 1u)));
      g.init_links();
      CHECK(g.name_to_nt("root").coord().tag() == 0u);
      CHECK(g.name_to_nt("root").coord().klasse() == 0u);
      derive_tags(g);
      CHECK(g.name_to_nt("root").coord().tag() == 23u);
      CHECK(g.name_to_nt("root").coord().klasse() == 1u);
    }

    TEST_CASE("grammar" "derive tags loop", "[grammar][derive]")
    {
      Grammar g;
      auto root = make_unique<Symbol::NT>("root");
      root->set_rule(make_unique<Rule::Link>("other"));
      g.push(std::move(root));
      auto other = make_unique<Symbol::NT>("other");
      other->set_rule(make_unique<Rule::Link>("root"));
      g.push(std::move(other));
      g.init_links();
      derive_tags(g);
      CHECK(true);
    }

    TEST_CASE("grammar" "derive tags in_constructed", "[grammar][derive]")
    {
      Grammar g;
      auto root = make_unique<Symbol::NT>("root");
      auto s = make_unique<Rule::Choice>();
      s->push(Reference("INTEGER"));
      s->push(Reference("OCTET STRING"));
      root->set_rule(std::move(s));
      g.push(std::move(root));
      g.push(make_unique<Symbol::Terminal>("OCTET STRING", Coordinates(4u, 0u)));
      g.push(make_unique<Symbol::Terminal>("INTEGER", Coordinates(2u, 0u)));
      g.init_links();
      CHECK(g.name_to_nt("root").coord().tag() == 0u);
      CHECK(g.name_to_nt("root").coord().klasse() == 0u);
      derive_tags(g);
      CHECK(g.name_to_nt("root").coord().tag() == 0u);
      CHECK(g.name_to_nt("root").coord().klasse() == 0u);
      CHECK(g.name_to_nt("root").rule().refs().front()->coord().tag() == 2u);
      CHECK(g.name_to_nt("root").rule().refs().front()->coord().klasse() == 0u);
      CHECK(g.name_to_nt("root").rule().refs().back()->coord().tag() == 4u);
      CHECK(g.name_to_nt("root").rule().refs().back()->coord().klasse() == 0u);
    }


    TEST_CASE("grammar" "tag translate print", "[grammar][translate]")
    {
      Grammar g;
      g.push(make_unique<Symbol::NT>("foo", Coordinates(23, 1)));
      g.push(make_unique<Symbol::NT>("bar", Coordinates(24, 0)));
      g.push(make_unique<Symbol::NT>("baz", Coordinates(13, 1)));
      g.name_to_nt("foo").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("bar").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("baz").set_rule(make_unique<Rule::Link>("blah"));
      g.name_to_nt("baz").rule().refs().front()->set_coord(Coordinates(6, 1));
      ostringstream o;
      print_tag_translations(o, g, 1);
      const char ref[] =
"6,blah\n"
"13,baz\n"
"23,foo\n"
;
      CHECK(o.str() == ref);
    }

    TEST_CASE("grammar" "tag closure print", "[grammar][closure]")
    {
      Grammar g;
      g.push(make_unique<Symbol::NT>("root"));
      auto c = make_unique<Rule::Sequence>();
      c->push(Reference("foo"));
      c->push(Reference("bar"));
      c->push(Reference("baz"));
      g.name_to_nt("root").set_rule(std::move(c));
      g.push(make_unique<Symbol::NT>("foo", Coordinates(23, 1)));
      g.push(make_unique<Symbol::NT>("bar", Coordinates(24, 1)));
      g.push(make_unique<Symbol::NT>("baz", Coordinates(13, 1)));
      g.push(make_unique<Symbol::NT>("blah"));
      g.push(make_unique<Symbol::NT>("blub"));
      g.name_to_nt("foo").set_rule(make_unique<Rule::Link>("OCTET STRING"));
      g.name_to_nt("bar").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("baz").set_rule(make_unique<Rule::Link>("blah"));
      g.name_to_nt("blah").set_rule(make_unique<Rule::Link>("blub"));
      g.name_to_nt("blub").set_rule(make_unique<Rule::Link>("INTEGER"));
      grammar::asn1::add_terminals(g);
      g.init_links();
      derive_tags(g);

      ostringstream o;
      print_tag_closure(o, g, "INTEGER" ,1);
      const char ref[] = "INTEGER,13,24\n";
      CHECK(o.str() == ref);
    }

    TEST_CASE("grammar" "tag closure empty", "[grammar][closure]")
    {
      // empty because foo and baz have to parents
      Grammar g;
      g.push(make_unique<Symbol::NT>("foo", Coordinates(23, 1)));
      g.push(make_unique<Symbol::NT>("bar", Coordinates(24, 1)));
      g.push(make_unique<Symbol::NT>("baz", Coordinates(13, 1)));
      g.push(make_unique<Symbol::NT>("blah"));
      g.push(make_unique<Symbol::NT>("blub"));
      g.name_to_nt("foo").set_rule(make_unique<Rule::Link>("OCTET STRING"));
      g.name_to_nt("bar").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("baz").set_rule(make_unique<Rule::Link>("blah"));
      g.name_to_nt("blah").set_rule(make_unique<Rule::Link>("blub"));
      g.name_to_nt("blub").set_rule(make_unique<Rule::Link>("INTEGER"));
      grammar::asn1::add_terminals(g);
      g.init_links();
      derive_tags(g);

      ostringstream o;
      print_tag_closure(o, g, "INTEGER" ,1);
      const char ref[] = "INTEGER\n";
      CHECK(o.str() == ref);
    }

    TEST_CASE("grammar" "tag closure loop", "[grammar][closure]")
    {
      Grammar g;
      g.push(make_unique<Symbol::NT>("root"));
      auto c = make_unique<Rule::Sequence>();
      c->push(Reference("foo"));
      c->push(Reference("bar"));
      c->push(Reference("baz"));
      g.name_to_nt("root").set_rule(std::move(c));
      g.push(make_unique<Symbol::NT>("foo", Coordinates(23, 1)));
      g.push(make_unique<Symbol::NT>("bar", Coordinates(24, 1)));
      g.push(make_unique<Symbol::NT>("baz", Coordinates(13, 1)));
      g.push(make_unique<Symbol::NT>("blah"));
      g.push(make_unique<Symbol::NT>("blub"));
      g.name_to_nt("foo").set_rule(make_unique<Rule::Link>("OCTET STRING"));
      g.name_to_nt("bar").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("baz").set_rule(make_unique<Rule::Link>("blah"));
      g.name_to_nt("blah").set_rule(make_unique<Rule::Link>("blub"));
      g.name_to_nt("blub").set_rule(make_unique<Rule::Link>("baz"));
      grammar::asn1::add_terminals(g);
      g.init_links();
      derive_tags(g);

      ostringstream o;
      print_tag_closure(o, g, "baz" ,1);
      const char ref[] = "baz,13\n";
      CHECK(o.str() == ref);
    }

     TEST_CASE("grammar" "duplicate_symbol", "[grammar][misc]")
     {
       Grammar g;
       g.push(make_unique<Symbol::NT>("foo"));
       REQUIRE_THROWS_AS(g.push(make_unique<Symbol::NT>("foo")),
           grammar::Parse_Error);
       REQUIRE_THROWS_AS(g.push(make_unique<Symbol::Terminal>("foo")),
           grammar::Parse_Error);
       g.push(make_unique<Symbol::Terminal>("OCTET STRING"));
       REQUIRE_THROWS_AS(g.push(make_unique<Symbol::Terminal>("OCTET STRING")),
           grammar::Parse_Error);
     }

     TEST_CASE("grammar" "eff_symbol", "[grammar][misc]")
     {
      Grammar g;
      g.push(make_unique<Symbol::NT>("root"));
      auto c = make_unique<Rule::Sequence>();
      c->push(Reference("foo"));
      c->push(Reference("bar"));
      c->push(Reference("baz"));
      g.name_to_nt("root").set_rule(std::move(c));
      g.push(make_unique<Symbol::NT>("foo"));
      g.push(make_unique<Symbol::NT>("bar", Coordinates(24, 1)));
      g.push(make_unique<Symbol::NT>("baz"));
      g.push(make_unique<Symbol::NT>("blah"));
      g.push(make_unique<Symbol::NT>("blub"));
      g.name_to_nt("foo").set_rule(make_unique<Rule::Link>("OCTET STRING"));
      g.name_to_nt("bar").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("baz").set_rule(make_unique<Rule::Link>("blah"));
      g.name_to_nt("blah").set_rule(make_unique<Rule::Link>("blub"));
      g.name_to_nt("blub").set_rule(make_unique<Rule::Link>("INTEGER"));
      grammar::asn1::add_terminals(g);
      g.init_links();
      CHECK(
          effective_symbol(*g.name_to_nt("root").rule().refs().at(0)).name() ==
          "OCTET STRING");
      CHECK(
          effective_symbol(*g.name_to_nt("root").rule().refs().at(1)).name() ==
          "INTEGER");
      CHECK(
          effective_symbol(*g.name_to_nt("root").rule().refs().at(2)).name() ==
          "INTEGER");
     }

     TEST_CASE("grammar" "unreachable", "[grammar][misc]")
     {
      Grammar g;
      g.push(make_unique<Symbol::NT>("root"));
      auto c = make_unique<Rule::Sequence>();
      c->push(Reference("foo"));
      c->push(Reference("bar"));
      c->push(Reference("baz"));
      g.name_to_nt("root").set_rule(std::move(c));
      g.push(make_unique<Symbol::NT>("foo"));
      g.push(make_unique<Symbol::NT>("bar", Coordinates(24, 1)));
      g.push(make_unique<Symbol::NT>("baz"));
      g.push(make_unique<Symbol::NT>("blah"));
      g.push(make_unique<Symbol::NT>("blub"));
      g.name_to_nt("foo").set_rule(make_unique<Rule::Link>("OCTET STRING"));
      g.name_to_nt("bar").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("baz").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("blah").set_rule(make_unique<Rule::Link>("blub"));
      g.name_to_nt("blub").set_rule(make_unique<Rule::Link>("BOOLEAN"));
      grammar::asn1::add_terminals(g);
      g.init_links();
      auto r = unreachable_symbols(g);
      CHECK(r.count("foo") == 0u);
      CHECK(r.count("bar") == 0u);
      CHECK(r.count("baz") == 0u);
      CHECK(r.count("INTEGER") == 0u);
      CHECK(r.count("OCTET STRING") == 0u);
      CHECK(r.count("BOOLEAN") == 1u);
      CHECK(r.count("NULL") == 1u);
      CHECK(r.count("blah") == 1u);
      CHECK(r.count("blub") == 1u);
     }

     TEST_CASE("grammar" "erase_unreachable", "[grammar][misc]")
     {
      Grammar g;
      g.push(make_unique<Symbol::NT>("root"));
      auto c = make_unique<Rule::Sequence>();
      c->push(Reference("foo"));
      c->push(Reference("bar"));
      c->push(Reference("baz"));
      g.name_to_nt("root").set_rule(std::move(c));
      g.push(make_unique<Symbol::NT>("foo"));
      g.push(make_unique<Symbol::NT>("bar", Coordinates(24, 1)));
      g.push(make_unique<Symbol::NT>("baz"));
      g.push(make_unique<Symbol::NT>("blah"));
      g.push(make_unique<Symbol::NT>("blub"));
      g.name_to_nt("foo").set_rule(make_unique<Rule::Link>("OCTET STRING"));
      g.name_to_nt("bar").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("baz").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("blah").set_rule(make_unique<Rule::Link>("blub"));
      g.name_to_nt("blub").set_rule(make_unique<Rule::Link>("BOOLEAN"));
      grammar::asn1::add_terminals(g);
      g.init_links();
      CHECK(g.symbols().size() == 41u);
      CHECK(g.nts().size() == 6u);
      CHECK(g.terminals().size() == 35u);
      erase_unreachable_symbols(g);
      CHECK(g.symbols().size() == 6u);
      CHECK(g.nts().size() == 4u);
      CHECK(g.terminals().size() == 2u);
     }

     TEST_CASE("grammar" "topological_sort", "[grammar][misc]")
     {
      Grammar g;
      g.push(make_unique<Symbol::NT>("foo"));
      g.push(make_unique<Symbol::NT>("bar", Coordinates(24, 1)));
      g.push(make_unique<Symbol::NT>("baz"));
      g.push(make_unique<Symbol::NT>("blah"));
      g.push(make_unique<Symbol::NT>("blub"));
      g.push(make_unique<Symbol::NT>("root"));
      auto c = make_unique<Rule::Sequence>();
      c->push(Reference("foo"));
      c->push(Reference("bar"));
      c->push(Reference("baz"));
      g.name_to_nt("root").set_rule(std::move(c));
      g.name_to_nt("foo").set_rule(make_unique<Rule::Link>("OCTET STRING"));
      g.name_to_nt("bar").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("baz").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("blah").set_rule(make_unique<Rule::Link>("blub"));
      g.name_to_nt("blub").set_rule(make_unique<Rule::Link>("BOOLEAN"));
      g.set_axiom("root");
      grammar::asn1::add_terminals(g);
      g.init_links();
      CHECK(g.symbols().size() == 41u);
      CHECK(g.nts().size() == 6u);
      CHECK(g.terminals().size() == 35u);
      tsort(g);
      CHECK(g.symbols().size() == 41u);
      CHECK(g.nts().size() == 6u);
      CHECK(g.terminals().size() == 35u);
      CHECK(g.nts().front()->name() == "root");
     }

     TEST_CASE("grammar" "topological_sort_axiom_first", "[grammar][misc]")
     {
      Grammar g;
      g.push(make_unique<Symbol::NT>("foo"));
      g.push(make_unique<Symbol::NT>("bar", Coordinates(24, 1)));
      g.push(make_unique<Symbol::NT>("baz"));
      g.push(make_unique<Symbol::NT>("blah"));
      g.push(make_unique<Symbol::NT>("blub"));
      g.push(make_unique<Symbol::NT>("root"));
      auto c = make_unique<Rule::Sequence>();
      c->push(Reference("foo"));
      c->push(Reference("bar"));
      c->push(Reference("baz"));
      g.name_to_nt("root").set_rule(std::move(c));
      g.name_to_nt("foo").set_rule(make_unique<Rule::Link>("OCTET STRING"));
      g.name_to_nt("bar").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("baz").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("blah").set_rule(make_unique<Rule::Link>("blub"));
      g.name_to_nt("blub").set_rule(make_unique<Rule::Link>("BOOLEAN"));
      //g.set_axiom("root"); // axiom is foo
      grammar::asn1::add_terminals(g);
      g.init_links();
      CHECK(g.symbols().size() == 41u);
      CHECK(g.nts().size() == 6u);
      CHECK(g.terminals().size() == 35u);
      tsort(g);
      CHECK(g.symbols().size() == 41u);
      CHECK(g.nts().size() == 6u);
      CHECK(g.terminals().size() == 35u);
      CHECK(g.nts().front()->name() == "foo");
     }

