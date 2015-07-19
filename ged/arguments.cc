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
const char help_s[] =
R"((COMMAND|OPTION)+

  Commands:

    -read-asn,-ra FILENAME   can be specified multiple times
                             after the first file, duplicate NTs are skipped

    -dt                      derive tags, must be called before print-closure
                             etc.
    -set-axiom,-sa NAME      override the default axiom (the first symbol)
    -tsort,-t                topological sort the non-terminals
    -erase-unreachable,-e    erase unreachable symbols
    -print-unreachable,-pu   print unreachable symbols
    -print-primitive,-ppt    print primitive tags
    -print-translate,-pt     print tag translations
    -print-closure,-pc TAG   print tag closure
    -print-asn,-pa FILE      print grammar in ASN.1 format
    -print-rng,-pr FILE      print grammar as RelaxNG grammar
    -print-xsd,-px FILE      print grammar as XML Schema (XSD)
    -print-dot,-pd FILE      print grammar as directed graph

  Options:

    --help,-help,-h          print this screen
    --version,-version       print version
    -no-term                 don't add terminals (i.e. UNIVERSAL tags)
                             when reading an ASN.1 grammar
                             must precede the read command
    -no-init                 don't init grammar links
    -class INT               override the default class (1 -> APPLICATION)
                             is used when computing the tag closure
    -ns NAMESPACE            set the target namespace (cf. -px/-pr)
                             by default, no namespace is declared
    -key NT FILENAME         include XSD key definitions from FILENAME
                             in the definition of element NT
                             (use before -px)
    -cs,-cons FILENAME       read additional constraints from a ZSV file
                             (ZSV is like CSV but uses a 0-byte as delimiter),
                             Supported are patterns, enumerations, length
                             and value restrictions.
                             Use before -px or -pr.

The current ASN.1 parser just understands a subset of ASN.1 ('Mini-ASN.1'),
but it is enough to parse many real world grammars (cf. README for details).
The algorithms currently assume all-implicit style ASN.1 grammars.

It is also quite liberal in what it accepts - e.g. certain ASN.1 lexing
rules are not enforced, e.g. identifiers may contain underscore characters.
The input file also need not feature a module header. This simplifies e.g.
the concatenation of different grammar snippets.

Examples:

Print all the tag aliases of the well known fundamental GSMA types:

    $ ged -ra TAP0312.asn1 -e -dt \
       -pc 'BCDString' -pc 'OCTET STRING' -pc 'INTEGER' -x

One can compare returned super set with the set of all primitive tags:

    $ ged -ra TAP0312.asn1 -e -dt -ppt

They should match.

Concate two grammar snippets:

    $ ged -ra RAP0105.asn1 -ra TAP0312.asn1 -e -dt -tsort -pa

Translate a ASN.1 file into a RelaxNG Schema (for XSD use `-px`):

    $ ged -ra TAP0312.asn1 -e -pr TAP0312.rng

Create XSD from an ASN.1 file using additional key constraints
and type restrictions:

    $ ged -ra TAP0312.asn1 -key TransferBatch tap_3_12_keys.xml \
        -cs tap_3_12_constraints.zsv -e -px tap_3_12.xsd

2015, Georg Sauthoff <mail@georg.so>

)";

#include "arguments.hh"

#include <stdlib.h>
#include <string.h>
#include <memory>
#include <deque>
#include <fstream>
#include <iostream>

#include <boost/lexical_cast.hpp>

#include <ixxx/util.hh>

#include <asn1/mini_parser.hh>
#include <asn1/grammar.hh>
#include <xml/xsd.hh>
#include <xml/rng.hh>
#include <graph/dot.hh>

#include <grammar.hh>
#include <tsort.hh>
#include <version.hh>

using namespace std;

namespace ged {

  class Arguments {
    private:
      void parse(int argc, char **argv);

      std::ostream &stream(int &i);
      void print_help();

      int argc_{0};
      char **argv_{nullptr};

      std::string argv0_;

      std::deque<std::unique_ptr<std::ofstream> > streams_;

      bool add_asn_terminals_ {true};
      bool init_links_ {true};
      bool first_grammar_ {true};
      bool links_uninitialized_ {true};
      unsigned klasse_ {1};
      std::string namespace_;
      std::deque<std::pair<std::string, std::string> > key_defs_;
      std::string constraints_filename_;

    public:
      Arguments(int argc, char **argv);

  };

  void evaluate_arguments(int argc, char **argv)
  {
    Arguments a(argc, argv);
  }

  void print_version()
  {
    cout << "ged " << grammar::version::str << '\n';
  }

  void Arguments::print_help()
  {
    cout << "call: " << argv0_ << ' ' << help_s << '\n';
    print_version();
  }

  ostream &Arguments::stream(int &i)
  {
    if (i + 1 < argc_ && *argv_[i+1] != '-') {
      ++i;
      const char *s = argv_[i];
      streams_.push_back(
          make_unique<ofstream>(s, ios_base::out | ios_base::binary));
      streams_.back()->exceptions(std::ifstream::failbit);
      return *streams_.back().get();
    }
    return cout;
  }

  Arguments::Arguments(int argc, char **argv)
    :
      argc_(argc),
      argv_(argv)
  {
    if (argc_)
      argv0_ = *argv_;
    parse(argc_, argv_);
  }

  void Arguments::parse(int argc, char **argv)
  {
    if (argc < 2)
      throw Argument_Error("No arguments given");
    grammar::Grammar g;
    for (int i = 1; i < argc; ++i) {
      for (auto & s : streams_) s.get()->close(); streams_.clear();
      char *s = argv[i];
      if (!strcmp(s, "-read-asn") || !strcmp(s, "-ra")) {
        ++i;
        if (i >= argc)
          throw Argument_Error("filename argument is missing");
        ixxx::util::Mapped_File f(argv[i]);
        if (!first_grammar_)
          g.set_ignore_duplicates(true);
        grammar::asn1::mini::Parser parser(std::move(g));
        parser.set_filename(argv[i]);
        parser.read(f.s_begin(), f.s_end());
        g = parser.grammar();
        first_grammar_ = false;
      } else {
        if (!constraints_filename_.empty()) {
          grammar::add_constraints(g, constraints_filename_);
          constraints_filename_.clear();
        }
        if (links_uninitialized_) {
          if (add_asn_terminals_)
            grammar::asn1::add_terminals(g);
          if (init_links_)
            g.init_links();
          links_uninitialized_ = false;
        }

        if (!strcmp(s, "-dt")) {
          derive_tags(g);
        } else if (!strcmp(s, "-tsort") || !strcmp(s, "-t")) {
          tsort(g);
        } else if (!strcmp(s, "-set-axiom") || !strcmp(s, "-sa")) {
          ++i;
          if (i >= argc)
            throw Argument_Error("set-axiom argument is missing");
          g.set_axiom(argv[i]);
        } else if (!strcmp(s, "-erase-unreachable") || !strcmp(s, "-e")) {
          erase_unreachable_symbols(g);
        } else if (!strcmp(s, "-print-unreachable") || !strcmp(s, "-pu")) {
          print_unreachable_symbols(stream(i), g);
        } else if (!strcmp(s, "-print-translate") || !strcmp(s, "-pt")) {
          print_tag_translations(stream(i), g, klasse_);
        } else if (!strcmp(s, "-print-primitive") || !strcmp(s, "-ppt")) {
          print_primitive_tags(stream(i), g, klasse_);
        } else if (!strcmp(s, "-print-closure") || !strcmp(s, "-pc")) {
          ++i;
          if (i >= argc)
            throw Argument_Error("tag argument is missing");
          print_tag_closure(stream(i), g, argv[i], klasse_);
        } else if (!strcmp(s, "-print-asn") || !strcmp(s, "-pa")) {
          grammar::asn1::Printer p(stream(i));
          p << g;
        } else if (!strcmp(s, "-print-rng") || !strcmp(s, "-pr")) {
          grammar::xml::rng::Printer p(stream(i));
          p.set_comment(argc, argv);
          p.set_target_namespace(namespace_);
          p << g;
        } else if (!strcmp(s, "-print-xsd") || !strcmp(s, "-px")) {
          grammar::xml::xsd::Printer p(stream(i));
          p.set_comment(argc, argv);
          p.set_target_namespace(namespace_);
          for (auto &x : key_defs_)
            p.read_key_def(x.first, x.second);
          p << g;
        } else if (!strcmp(s, "-print-dot") || !strcmp(s, "-pd")) {
          grammar::graph::dot::Printer p(stream(i));
          p << g;
        } else if (!strcmp(s, "-no-term")) {
          add_asn_terminals_ = false;
        } else if (!strcmp(s, "-no-init")) {
          init_links_ = false;
        } else if (!strcmp(s, "-klasse") || !strcmp(s, "-k")) {
          ++i;
          if (i >= argc)
            throw Argument_Error("class argument is missing");
          klasse_ = boost::lexical_cast<unsigned>(argv[i]);
        } else if (!strcmp(s, "-ns")) {
          ++i;
          if (i >= argc)
            throw Argument_Error("namespace argument is missing");
          namespace_ = argv[i];
        } else if (!strcmp(s, "-key")) {
          ++i;
          if (i >= argc)
            throw Argument_Error("nt argument is missing");
          string nt(argv[i]);
          ++i;
          if (i >= argc)
            throw Argument_Error("filename argument is missing");
          string filename(argv[i]);
          key_defs_.emplace_back(nt, filename);
        } else if(!strcmp(s, "-cs") || !strcmp(s, "-cons")) {
          ++i;
          if (i >= argc)
            throw Argument_Error("constraints filename argument is missing");
          constraints_filename_ = argv[i];
        } else if (!strcmp(s, "--version") || !strcmp(s, "-version"))  {
          print_version();
          exit(0);
        } else if (!strcmp(s, "--help") || !strcmp(s, "-help")
            || !strcmp(s, "-h")) {
          print_help();
          exit(0);
        } else {
          throw Argument_Error("Unknown argument: " + string(s));
        }
      }
    }
  }

}
