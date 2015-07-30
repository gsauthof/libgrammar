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
#include "tsort.hh"

#include "grammar.hh"


#include <boost/algorithm/string.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>

#include <algorithm>
#include <vector>
#include <iterator>

using Graph    = boost::adjacency_list<
                   boost::vecS, boost::vecS, boost::directedS,
                   boost::property<
                     boost::vertex_color_t,
                     boost::default_color_type
                   >
                 > ;
using Vertex   = boost::graph_traits<Graph>::vertex_descriptor;
using Edge     = std::pair<size_t, size_t>;
using Ordering = std::vector<Vertex>;

using namespace std;

namespace grammar {


  void tsort(Grammar &g)
  {
    vector<Symbol::NT*> nts(g.nts().begin(), g.nts().end());
    auto Cmp = [](const Symbol::NT *a, const Symbol::NT *b)
        { return a->name() < b->name(); };
    sort(nts.begin(), nts.end(), Cmp);
    vector<Edge> edges;
    edges.reserve(nts.size() * 2);
    size_t i = 0;
    for (auto nt : nts) {
      for (auto &ref : nt->rule().refs()) {
        Symbol::NT *nt = dynamic_cast<Symbol::NT*>(&ref.symbol());
        if (nt) {
          auto p = equal_range(nts.begin(), nts.end(), nt, Cmp);
          edges.emplace_back(i, p.first - nts.begin());
          // or reverse for reverse topologicla ordering ...
          //edges.emplace_back(p.first - nts.begin(), i);
        }
      }
      ++i;
    }
    Graph graph(edges.begin(), edges.end(), nts.size());
    Ordering ordering;
    ordering.reserve(nts.size());
    topological_sort(graph, back_inserter(ordering));
    deque<Symbol::NT*> new_nts;
    for (auto i = ordering.rbegin(); i != ordering.rend(); ++i) {
    //for (auto i = ordering.begin(); i != ordering.end(); ++i) {
      new_nts.push_back(nts[*i]);
    }
    if (new_nts.front() != &g.axiom()) {
      auto i = find(new_nts.begin(), new_nts.end(), &g.axiom());
      if (i != new_nts.end()) {
        swap(*i, *new_nts.begin());
      }
    }
    g.set_nts(std::move(new_nts));
  }

}
