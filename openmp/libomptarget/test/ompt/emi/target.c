// RUN: %libomptarget-compile-generic -DNOWAIT=0 && %libomptarget-run-generic 2>&1 | %fcheck-generic --check-prefixes CHECK,SYNC
// RUN: %libomptarget-compile-generic -DNOWAIT=1 && %libomptarget-run-generic 2>&1 | %fcheck-generic --check-prefixes CHECK,ASYNC
// REQUIRES: ompt

#include "callback-emi.h"

int main() {
  #pragma omp target NOWAIT_CLAUSE
  {
    //print_ids(0);
    printf("hello\n");
  }
  print_fuzzy_address(1);
#if NOWAIT
  #pragma omp taskwait
#endif

  // CHECK-NOT: {{^}}0: Could not register callback
  // CHECK: 0: NULL_POINTER=[[NULL:.*$]]

  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_thread_begin: thread_type=ompt_thread_initial=1, thread_id=[[MASTER_ID]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_initial_task_begin
  // CHECK-SAME: parallel_id=[[PARALLEL_ID:[0-9]+]], task_id=[[INITIAL_TASK_ID:[0-9]+]], actual_parallelism=1, index=1, flags=1

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_emi_begin
  // SYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_task_id=0, target_id=[[TARGET_ID_1:[0-9]+]], device_num=[[DEVICE_NUM:[0-9]+]]
  // SYNC-SAME: kind=ompt_target, codeptr_ra=[[TARGET_RETURN_ADDRESS:0x[0-f]+]]{{[0-f][0-f]}}

  // ASYNC: {{^}}[[MASTER_ID]]: ompt_event_task_create
  // ASYNC-SAME: parent_task_id=[[INITIAL_TASK_ID]], parent_task_frame.exit=(nil), parent_task_frame.reenter=0x{{[0-f]+}}
  // ASYNC-SAME: new_task_id=[[TARGET_TASK_ID:[0-9]+]], codeptr_ra=[[TARGET_RETURN_ADDRESS:0x[0-f]+]]{{[0-f][0-f]}}
  // ASYNC-SAME: task_type=ompt_task_explicit|ompt_task_target


  // ASYNC-DAG: {{^}}[[THREAD_ID:[0-9]+]]: ompt_event_target_emi_begin: task_id=[[INITIAL_TASK_ID]], target_task_id=[[TARGET_TASK_ID]], target_id=[[TARGET_ID_1:[0-9]+]], device_num=[[DEVICE_NUM:[0-9]+]], kind=ompt_target_nowait, codeptr_ra=(nil)

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_submit_emi_begin
  // SYNC-SAME: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID:[0-9]+]], requested_num_teams=1

  // ASYNC-DAG: {{^}}[[THREAD_ID]]: ompt_event_target_submit_emi_begin: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID:[0-9]+]], requested_num_teams=1

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_submit_emi_end
  // SYNC-SAME: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID]], requested_num_teams=1

  // ASYNC-DAG: {{^}}[[THREAD_ID]]: ompt_event_target_submit_emi_end: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID]], requested_num_teams=1

  // COM: {{^}}[[MASTER_ID]]: task level 0
  // COM: parallel_id=[[PARALLEL_ID]], task_id=[[INITIAL_TASK_ID]]

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_emi_end
  // SYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_task_id=0, target_id=[[TARGET_ID_1]], device_num=[[DEVICE_NUM]]
  // SYNC-SAME: kind=ompt_target, codeptr_ra=[[TARGET_RETURN_ADDRESS]]{{[0-f][0-f]}}
  // SYNC: {{^}}[[MASTER_ID]]: fuzzy_address={{.*}}[[TARGET_RETURN_ADDRESS]]{{[0-f][0-f]}}

  // ASYNC-DAG: {{^}}[[THREAD_ID]]: ompt_event_target_emi_end: task_id=[[INITIAL_TASK_ID]], target_task_id=[[TARGET_TASK_ID]], target_id=[[TARGET_ID_1]], device_num=[[DEVICE_NUM]], kind=ompt_target_nowait, codeptr_ra=(nil)
  // ASYNC-DAG: {{^}}[[MASTER_ID]]: fuzzy_address={{.*}}[[TARGET_RETURN_ADDRESS]]{{[0-f][0-f]}}

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_initial_task_end
  // CHECK-SAME: parallel_id=[[PARALLEL_ID]], task_id=[[INITIAL_TASK_ID]], actual_parallelism=0, index=1
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_thread_end: thread_id=[[MASTER_ID]]
  return 0;
}