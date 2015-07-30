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

#include <grammar/grammar.hh>
#include <grammar/tsort.hh>
#include <grammar/asn1/grammar.hh>

using namespace std;


BOOST_AUTO_TEST_SUITE(grammar_)

  using namespace grammar;


  BOOST_AUTO_TEST_SUITE(grammar_)

   BOOST_AUTO_TEST_SUITE(links)

    BOOST_AUTO_TEST_CASE(basic)
    {
      Grammar g;
      auto root = make_unique<Symbol::NT>("root");
      auto s = make_unique<Rule::Set>();
      s->push(Reference("OCTET STRING"));
      root->set_rule(std::move(s));
      g.push(std::move(root));
      g.push(make_unique<Symbol::Terminal>("OCTET STRING"));
      BOOST_CHECK_THROW(
          g.name_to_nt("root").rule().refs().front().symbol(),
          grammar::Runtime_Error);
      g.init_links();
      BOOST_CHECK_EQUAL(
          g.name_to_nt("root").rule().refs().front().symbol().name(),
          "OCTET STRING");
    }

    BOOST_AUTO_TEST_CASE(missing_symbol)
    {
      Grammar g;
      auto root = make_unique<Symbol::NT>("root");
      auto s = make_unique<Rule::Set>();
      s->push(Reference("OCTET STRING"));
      root->set_rule(std::move(s));
      g.push(std::move(root));
      BOOST_CHECK_THROW(g.init_links(), grammar::Runtime_Error);
    }

   BOOST_AUTO_TEST_SUITE_END()


   BOOST_AUTO_TEST_SUITE(derive_tags_)

    BOOST_AUTO_TEST_CASE(is_set)
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
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().tag(), 0u);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().klasse(), 0u);
      derive_tags(g);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().tag(), 17u);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().klasse(), 0u);
    }

    BOOST_AUTO_TEST_CASE(basic)
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
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().tag(), 0u);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().klasse(), 0u);
      derive_tags(g);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().tag(), 23u);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().klasse(), 1u);
    }

    BOOST_AUTO_TEST_CASE(loop)
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
      BOOST_CHECK(true);
    }

    BOOST_AUTO_TEST_CASE(in_constructed)
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
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().tag(), 0u);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().klasse(), 0u);
      derive_tags(g);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().tag(), 0u);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").coord().klasse(), 0u);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").rule().refs().front().coord().tag(), 2u);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").rule().refs().front().coord().klasse(), 0u);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").rule().refs().back().coord().tag(), 4u);
      BOOST_CHECK_EQUAL(g.name_to_nt("root").rule().refs().back().coord().klasse(), 0u);
    }

   BOOST_AUTO_TEST_SUITE_END()

   BOOST_AUTO_TEST_SUITE(tag_translations_)

    BOOST_AUTO_TEST_CASE(print)
    {
      Grammar g;
      g.push(make_unique<Symbol::NT>("foo", Coordinates(23, 1)));
      g.push(make_unique<Symbol::NT>("bar", Coordinates(24, 0)));
      g.push(make_unique<Symbol::NT>("baz", Coordinates(13, 1)));
      g.name_to_nt("foo").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("bar").set_rule(make_unique<Rule::Link>("INTEGER"));
      g.name_to_nt("baz").set_rule(make_unique<Rule::Link>("blah"));
      g.name_to_nt("baz").rule().refs().front().set_coord(Coordinates(6, 1));
      ostringstream o;
      print_tag_translations(o, g, 1);
      const char ref[] =
"6,blah\n"
"13,baz\n"
"23,foo\n"
;
      BOOST_CHECK_EQUAL(o.str(), ref);
    }

   BOOST_AUTO_TEST_SUITE_END() // tag_translations_

   BOOST_AUTO_TEST_SUITE(tag_closure)

    BOOST_AUTO_TEST_CASE(print)
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
      BOOST_CHECK_EQUAL(o.str(), ref);
    }

    BOOST_AUTO_TEST_CASE(empty)
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
      BOOST_CHECK_EQUAL(o.str(), ref);
    }

    BOOST_AUTO_TEST_CASE(loop)
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
      BOOST_CHECK_EQUAL(o.str(), ref);
    }

   BOOST_AUTO_TEST_SUITE_END() // tag_closure


   BOOST_AUTO_TEST_SUITE(misc)

     BOOST_AUTO_TEST_CASE(duplicate_symbol)
     {
       Grammar g;
       g.push(make_unique<Symbol::NT>("foo"));
       BOOST_CHECK_THROW(g.push(make_unique<Symbol::NT>("foo")),
           grammar::Parse_Error);
       BOOST_CHECK_THROW(g.push(make_unique<Symbol::Terminal>("foo")),
           grammar::Parse_Error);
       g.push(make_unique<Symbol::Terminal>("OCTET STRING"));
       BOOST_CHECK_THROW(g.push(make_unique<Symbol::Terminal>("OCTET STRING")),
           grammar::Parse_Error);
     }

     BOOST_AUTO_TEST_CASE(eff_symbol)
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
      BOOST_CHECK_EQUAL(
          effective_symbol(g.name_to_nt("root").rule().refs().at(0)).name(),
          "OCTET STRING");
      BOOST_CHECK_EQUAL(
          effective_symbol(g.name_to_nt("root").rule().refs().at(1)).name(),
          "INTEGER");
      BOOST_CHECK_EQUAL(
          effective_symbol(g.name_to_nt("root").rule().refs().at(2)).name(),
          "INTEGER");
     }

     BOOST_AUTO_TEST_CASE(unreachable)
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
      BOOST_CHECK_EQUAL(r.count("foo"), 0u);
      BOOST_CHECK_EQUAL(r.count("bar"), 0u);
      BOOST_CHECK_EQUAL(r.count("baz"), 0u);
      BOOST_CHECK_EQUAL(r.count("INTEGER"), 0u);
      BOOST_CHECK_EQUAL(r.count("OCTET STRING"), 0u);
      BOOST_CHECK_EQUAL(r.count("BOOLEAN"), 1u);
      BOOST_CHECK_EQUAL(r.count("NULL"), 1u);
      BOOST_CHECK_EQUAL(r.count("blah"), 1u);
      BOOST_CHECK_EQUAL(r.count("blub"), 1u);
     }

     BOOST_AUTO_TEST_CASE(erase_unreachable)
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
      BOOST_CHECK_EQUAL(g.symbols().size(), 41u);
      BOOST_CHECK_EQUAL(g.nts().size(), 6u);
      BOOST_CHECK_EQUAL(g.terminals().size(), 35u);
      erase_unreachable_symbols(g);
      BOOST_CHECK_EQUAL(g.symbols().size(), 6u);
      BOOST_CHECK_EQUAL(g.nts().size(), 4u);
      BOOST_CHECK_EQUAL(g.terminals().size(), 2u);
     }

     BOOST_AUTO_TEST_CASE(topological_sort)
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
      BOOST_CHECK_EQUAL(g.symbols().size(), 41u);
      BOOST_CHECK_EQUAL(g.nts().size(), 6u);
      BOOST_CHECK_EQUAL(g.terminals().size(), 35u);
      tsort(g);
      BOOST_CHECK_EQUAL(g.symbols().size(), 41u);
      BOOST_CHECK_EQUAL(g.nts().size(), 6u);
      BOOST_CHECK_EQUAL(g.terminals().size(), 35u);
      BOOST_CHECK_EQUAL(g.nts().front()->name(), "root");
     }

     BOOST_AUTO_TEST_CASE(topological_sort_axiom_first)
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
      BOOST_CHECK_EQUAL(g.symbols().size(), 41u);
      BOOST_CHECK_EQUAL(g.nts().size(), 6u);
      BOOST_CHECK_EQUAL(g.terminals().size(), 35u);
      tsort(g);
      BOOST_CHECK_EQUAL(g.symbols().size(), 41u);
      BOOST_CHECK_EQUAL(g.nts().size(), 6u);
      BOOST_CHECK_EQUAL(g.terminals().size(), 35u);
      BOOST_CHECK_EQUAL(g.nts().front()->name(), "foo");
     }

   BOOST_AUTO_TEST_SUITE_END() // misc

  BOOST_AUTO_TEST_SUITE_END() // grammar_


BOOST_AUTO_TEST_SUITE_END() // grammar_

