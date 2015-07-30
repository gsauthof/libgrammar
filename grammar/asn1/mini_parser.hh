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
#include <grammar/grammar.hh>

#include <iostream>
#include <string>
#include <deque>

namespace grammar {

  namespace asn1 {

    namespace mini {


      class Parser {
        public:
          Parser(Grammar &&g, std::ostream &error_stream = std::cerr);
          Parser(std::ostream &error_stream = std::cerr);
          Parser(const std::string &filename, std::ostream &error_stream = std::cerr);
          void read(const char *begin, const char *end);
          void restart();

          grammar::Grammar grammar();
          void set_filename(const std::string &filename);
        private:
          void read_without_comments(const char *begin, const char *end);
          const char *read_section(const char *begin, const char *end);
          void throw_parse_error(const char *begin, const char *p, const char *pe, const std::string &msg);

          std::ostream &e;
          int cs {0};
          const char *p {nullptr};
          const char *eof {nullptr};
          size_t line_ {1u};
          std::string filename_ {"unknown"};
          grammar::Grammar grammar_;
          std::unique_ptr<Symbol::NT> nt_;
          std::unique_ptr<Rule::Base> rule_;
          Reference ref_;
          bool sequence_ {true};
          int klasse_ {0};
          Coordinates coord_;
          std::pair<ssize_t, ssize_t> size_;

      };

      Grammar parse_files(const std::deque<std::string> &filenames);


    }


  }
}

