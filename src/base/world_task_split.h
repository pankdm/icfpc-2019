#pragma once

#include "base/world_ext.h"
#include "common/unsigned_set.h"
#include <vector>

class WorldTaskSplit : public WorldExt {
 protected:
  std::vector<unsigned> task_id;
  std::vector<UnsignedSet> tasks;
  UnsignedSet task_update_required;

 protected:
  void BuildDSForSet();
  void Update();

 protected:
  void BuildDS();
  void UpdateDS();
  bool UpdateDSRequired() const;

 public:
  const UnsignedSet& GetTask(unsigned task_id) const;
  void UpdateTask(unsigned task_id);
  bool UpdateTaskRequired(unsigned task_id);
  UnsignedSet& UnwrappedSet();

 public:
  void SetNewTasks(const std::vector<UnsignedSet>& new_tasks);

 public:
  void Init(const std::string& desc);
  void ApplyC(unsigned index, const Action& action);
  void ApplyC(const ActionsList& actions);
};
