#include "base/world_ext.h"

#include <cassert>

unsigned WorldExt::Size() const { return map.Size(); }

unsigned WorldExt::Index(int x, int y) const { return map.Index(x, y); }

bool WorldExt::Solved() const { return unwrapped.Empty(); }

void WorldExt::BuildDSForSet() {
  for (unsigned u : ds_update_set.List()) {
    for (unsigned t : g.Edges(u)) {
      if ((t > u) && unwrapped.HasKey(t)) {
        ds.Union(u, t);
      }
    }
  }
  ds_update_set.Clear();
}

void WorldExt::Update() {
  auto& q = map.wraps_history;
  for (; !q.empty(); q.pop()) {
    unsigned index = q.front();
    if (unwrapped.HasKey(index)) {
      unwrapped.Remove(index);
      ds_update_required.Insert(ds.Find(index));
    }
  }
}

void WorldExt::BuildDS() {
  ds_update_set = unwrapped;
  BuildDSForSet();
}

void WorldExt::UpdateDS() {
  if (ds_update_required.Empty()) return;
  assert(ds_update_set.Empty());
  for (unsigned u : unwrapped.List()) {
    if (ds_update_required.HasKey(ds.Find(u))) ds_update_set.Insert(u);
  }
  ds.unions -= (ds_update_set.Size() - ds_update_required.Size());
  for (unsigned u : ds_update_set.List()) {
    ds.p[u] = u;
    ds.rank[u] = 0;
    ds.vsize[u] = 1;
  }
  BuildDSForSet();
}

void WorldExt::Init(const std::string& desc) {
  World::Init(desc);
  unsigned size = Size();
  map.save_wraps = true;
  g.Resize(size);
  unwrapped.Clear();
  unwrapped.Resize(size);
  for (int x = 0; x < map.xsize; ++x) {
    for (int y = 0; y < map.ysize; ++y) {
      if (map.ValidToMove(x, y)) {
        unsigned index = Index(x, y);
        if (map.ValidToMove(x + 1, y)) g.AddEdge(index, Index(x + 1, y));
        if (map.ValidToMove(x, y + 1)) g.AddEdge(index, Index(x, y + 1));
        if (!map[index].Wrapped()) unwrapped.Insert(index);
      }
    }
  }
  ds.Init(size);
  ds_update_set.Resize(size);
  ds_update_required.Resize(size);
  BuildDS();
}

void WorldExt::Apply(const Action& action) {
  World::Apply(action);
  Update();
}

void WorldExt::ApplyC(unsigned index, const Action& action) {
  World::ApplyC(index, action);
  Update();
}

void WorldExt::ApplyC(const ActionsList& actions) {
  World::ApplyC(actions);
  Update();
}
