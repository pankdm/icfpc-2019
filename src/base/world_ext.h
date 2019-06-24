#pragma once

#include "base/action.h"
#include "base/world.h"
#include "common/disjoint_set.h"
#include "common/graph/graph.h"
#include "common/unsigned_set.h"
#include <string>
#include <vector>

class WorldExt : public World {
 protected:
  UndirectedGraph g;
  UnsignedSet unwrapped;
  DisjointSet ds;

  UnsignedSet ds_update_set;
  UnsignedSet ds_update_required;

 public:
  unsigned Size() const;
  unsigned Index(int x, int y) const;
  bool Solved() const;
  const UndirectedGraph& G() const;
  const std::vector<unsigned>& GEdges(unsigned u) const;
  const std::vector<unsigned>& UList() const;
  bool Unwrapped(unsigned index) const;
  bool Unwrapped(int x, int y) const;
  const UnsignedSet& UnwrappedSet() const;

 protected:
  void BuildDSForSet();
  void Update();

 public:
  void BuildDS();
  void UpdateDS();
  bool UpdateDSRequired() const;

  unsigned DSFind(unsigned u);
  unsigned DSSize(unsigned u);

 public:
  void Init(const std::string& desc);
  void Apply(const Action& action);
  void ApplyC(unsigned index, const Action& action);
  void ApplyC(const ActionsList& actions);
};
