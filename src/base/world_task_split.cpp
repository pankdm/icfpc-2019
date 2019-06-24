#include "base/world_task_split.h"

#include "common/always_assert.h"
#include <algorithm>

void WorldTaskSplit::BuildDSForSet() {
  for (unsigned u : ds_update_set.List()) {
    ds.p[u] = u;
    ds.rank[u] = 0;
    ds.vsize[u] = 1;
  }
  for (unsigned u : ds_update_set.List()) {
    for (unsigned t : g.Edges(u)) {
      if ((t > u) && unwrapped.HasKey(t) && (task_id[u] == task_id[t])) {
        ds.Union(u, t);
      }
    }
  }
  ds_update_set.Clear();
}

void WorldTaskSplit::Update() {
  auto& q = map.wraps_history;
  for (; !q.empty(); q.pop()) {
    unsigned index = q.front();
    if (unwrapped.HasKey(index)) {
      unwrapped.Remove(index);
      ds_update_required.Insert(ds.Find(index));
      ALWAYS_ASSERT(index < task_id.size());
      unsigned task_index = task_id[index];
      ALWAYS_ASSERT(task_index < tasks.size());
      if (tasks[task_index].HasKey(index)) {
        tasks[task_index].Remove(index);
        task_update_required.Insert(task_index);
      }
    }
  }
  UpdateAllPOI();
}

void WorldTaskSplit::BuildDS() {
  ds_update_set = unwrapped;
  BuildDSForSet();
}

void WorldTaskSplit::UpdateDS() {
  if (ds_update_required.Empty()) return;
  ALWAYS_ASSERT(ds_update_set.Empty());
  for (unsigned u : unwrapped.List()) {
    if (ds_update_required.HasKey(ds.Find(u))) ds_update_set.Insert(u);
  }
  ds.unions -= (ds_update_set.Size() - ds_update_required.Size());
  ds_update_required.Clear();
  BuildDSForSet();
}

unsigned WorldTaskSplit::TotalTasks() const { return tasks.size(); }

const UnsignedSet& WorldTaskSplit::GetTask(unsigned task_id) const {
  ALWAYS_ASSERT(task_id < tasks.size());
  return tasks[task_id];
}

void WorldTaskSplit::UpdateTask(unsigned task_id) {
  UpdateDS();
  task_update_required.Remove(task_id);
}

bool WorldTaskSplit::UpdateTaskRequired(unsigned task_id) {
  return task_update_required.HasKey(task_id);
}

UnsignedSet& WorldTaskSplit::UnwrappedSet() { return unwrapped; }

void WorldTaskSplit::InitPOI() {
  items.clear();
  items.resize(unsigned(Item::UNKNOWN));
  for (unsigned index = 0; index < Size(); ++index) {
    Item item = map[index].CheckItem();
    if ((item != Item::NONE) && (item < Item::UNKNOWN)) {
      items[unsigned(item)].items.emplace_back(POI(item, index, g));
    }
  }
}

const POIList& WorldTaskSplit::GetPOIList(Item item) const {
  ALWAYS_ASSERT(item < Item::UNKNOWN);
  return items[unsigned(item)];
}

void WorldTaskSplit::UpdatePOIList(Item item) {
  ALWAYS_ASSERT(item < Item::UNKNOWN);
  items[unsigned(item)].Update(map);
}

void WorldTaskSplit::UpdateAllPOI() {
  for (unsigned i = 0; i < items.size(); ++i) {
    UpdatePOIList(Item(i));
  }
}

void WorldTaskSplit::SetNewTasks(const std::vector<UnsignedSet>& new_tasks) {
  tasks = new_tasks;
  std::fill(task_id.begin(), task_id.end(), tasks.size());
  for (unsigned i = 0; i < tasks.size(); ++i) {
    for (unsigned u : tasks[i].List()) {
      ALWAYS_ASSERT(task_id[u] == tasks.size());
      task_id[u] = i;
    }
  }
  task_update_required.Resize(tasks.size());
  for (unsigned i = 0; i < tasks.size(); ++i) task_update_required.Insert(i);
  BuildDS();
}

void WorldTaskSplit::Init(const std::string& desc) {
  WorldExt::Init(desc);
  InitPOI();
  std::vector<UnsignedSet> vtemp(1, unwrapped);
  task_id.resize(Size());
  SetNewTasks(vtemp);
}

void WorldTaskSplit::ApplyC(unsigned index, const Action& action) {
  World::ApplyC(index, action);
  Update();
}

void WorldTaskSplit::ApplyC(const ActionsList& actions) {
  World::ApplyC(actions);
  Update();
}
