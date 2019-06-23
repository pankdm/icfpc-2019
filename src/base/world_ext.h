#pragma once

#include "base/action.h"
#include "base/world.h"
#include "common/disjoint_set.h"
#include "common/graph/graph.h"
#include "common/unsigned_set.h"
#include <string>

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

 protected:
  void BuildDSForSet();
  void Update();

 public:
  void BuildDS();
  void UpdateDS();

 public:
  void Init(const std::string& desc);
  void Apply(const Action& action);
  void ApplyC(unsigned index, const Action& action);
  void ApplyC(const ActionsList& actions);
};
