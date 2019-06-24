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

void WorldTaskSplit::UpdateTask(unsigned task) {
  UpdateDS();
  task_update_required.Remove(task);
}

bool WorldTaskSplit::UpdateTaskRequired(unsigned task) {
  return task_update_required.HasKey(task);
}

UnsignedSet& WorldTaskSplit::UnwrappedSet() { return unwrapped; }

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
