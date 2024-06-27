// RUN: %libomptarget-compile-generic && %libomptarget-run-generic 2>&1 | %fcheck-generic --check-prefixes CHECK
// REQUIRES: ompt

#include <stdio.h>
#include "callback-noemi.h"

#pragma omp declare target
int a = 0;
#pragma omp end declare target

int main() {
  int *a_addr = 0;
  #pragma omp target map(tofrom: a_addr)
  {
    a_addr = &a;
  }
  print_fuzzy_address(1);
  printf("%" PRIu64 ":" _TOOL_PREFIX " a: host_addr=%p, target_addr=%p\n", ompt_get_thread_data()->value, &a, a_addr);

  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_thread_begin: thread_type=ompt_thread_initial=1, thread_id=[[MASTER_ID]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_initial_task_begin
  // CHECK-SAME: parallel_id=[[PARALLEL_ID:[0-9]+]], task_id=[[INITIAL_TASK_ID:[0-9]+]], actual_parallelism=1, index=1, flags=1

  /** target 0 (global variable initialization) **/

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_begin
  // CHECK-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_0:[0-9]+]], device_num=[[DEVICE_NUM:[0-9]+]]
  // CHECK-SAME: kind=ompt_target_enter_data, codeptr_ra=(nil)
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_0]], device_mem_flag=ompt_device_mem_flag_to|ompt_device_mem_flag_alloc|ompt_device_mem_flag_associate
  // CHECK-SAME: host_base_addr=[[A_HOST_BASE_ADDR:0x[0-f]+]], host_addr=[[A_HOST_ADDR:0x[0-f]+]], host_device_num=[[HOST_NUM:[0-9]+]]
  // CHECK-SAME: target_addr=[[A_TARGET_ADDR:0x[0-f]+]], target_device_num=[[DEVICE_NUM:[0-9]+]], bytes=4, codeptr_ra=(nil)

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_map
  // CHECK-SAME: target_id=[[TARGET_ID_0]], nitems=1, codeptr_ra=(nil)
  // CHECK-NEXT: {{^}}[[MASTER_ID]]: map: host_addr=[[A_HOST_ADDR]], device_addr=[[A_TARGET_ADDR]], bytes=4, mapping_flag=ompt_target_map_flag_to

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_end
  // CHECK-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_0]], device_num=[[DEVICE_NUM]]
  // CHECK-SAME: kind=ompt_target_enter_data, codeptr_ra=(nil)

  /** target 1 (target) **/

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_begin
  // CHECK-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_1:[0-9]+]], device_num=[[DEVICE_NUM]]
  // CHECK-SAME: kind=ompt_target, codeptr_ra=[[TARGET_RETURN_ADDRESS_1:0x[0-f]+]]{{[0-f][0-f]}}

  /** offload a_addr **/
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op
  // CHECK-SAME: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_1:[0-9]+]], optype=ompt_target_data_alloc, src_addr=[[APTR_HOST_ADDR:0x[0-f]+]]
  // CHECK-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[APTR_TARGET_ADDR:0x[0-f]+]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], device_mem_flag=ompt_device_mem_flag_alloc|ompt_device_mem_flag_associate
  // CHECK-SAME: host_base_addr=[[APTR_HOST_BASE_ADDR:0x[0-f]+]], host_addr=[[APTR_HOST_ADDR]], host_device_num=[[HOST_NUM]]
  // CHECK-SAME: target_addr=[[APTR_TARGET_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], device_mem_flag=ompt_device_mem_flag_to
  // CHECK-SAME: host_base_addr=[[APTR_HOST_BASE_ADDR]], host_addr=[[APTR_HOST_ADDR]], host_device_num=[[HOST_NUM]]
  // CHECK-SAME: target_addr=[[APTR_TARGET_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op
  // CHECK-SAME: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_2:[0-9]+]], optype=ompt_target_data_transfer_to_device, src_addr=[[APTR_HOST_ADDR]]
  // CHECK-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[APTR_TARGET_ADDR]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_map
  // CHECK-SAME: target_id=[[TARGET_ID_1]], nitems=1, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}
  // CHECK-NEXT: {{^}}[[MASTER_ID]]: map: host_addr=[[APTR_HOST_ADDR]], device_addr=[[APTR_TARGET_ADDR]], bytes=8, mapping_flag=ompt_target_map_flag_to|ompt_target_map_flag_from


  /** submit kernel **/
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_submit
  // CHECK-SAME: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_3:[0-9]+]], requested_num_teams=1

  /** offload a_addr back **/
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], device_mem_flag=ompt_device_mem_flag_from
  // CHECK-SAME: host_base_addr=[[APTR_HOST_BASE_ADDR]], host_addr=[[APTR_HOST_ADDR]], host_device_num=[[HOST_NUM]]
  // CHECK-SAME: target_addr=[[APTR_TARGET_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op
  // CHECK-SAME: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_4:[0-9]+]], optype=ompt_target_data_transfer_from_device, src_addr=[[APTR_TARGET_ADDR]]
  // CHECK-SAME: src_device_num=[[DEVICE_NUM]], dest_addr=[[APTR_HOST_ADDR]], dest_device_num=[[HOST_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op
  // CHECK-SAME: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_5:[0-9]+]], optype=ompt_target_data_delete, src_addr=[[APTR_HOST_ADDR]]
  // CHECK-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[APTR_TARGET_ADDR]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], device_mem_flag=ompt_device_mem_flag_release|ompt_device_mem_flag_disassociate
  // CHECK-SAME: host_base_addr=[[APTR_HOST_BASE_ADDR]], host_addr=[[APTR_HOST_ADDR]], host_device_num=[[HOST_NUM]]
  // CHECK-SAME: target_addr=[[APTR_TARGET_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}


  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_end
  // CHECK-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_1]], device_num=[[DEVICE_NUM]]
  // CHECK-SAME: kind=ompt_target, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}
  // CHECK: {{^}}[[MASTER_ID]]: fuzzy_address={{.*}}[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}
  
  // CHECK: {{^}}[[MASTER_ID]]: a: host_addr=[[A_HOST_ADDR]], target_addr=[[A_TARGET_ADDR]]

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_initial_task_end
  // CHECK-SAME: parallel_id=[[PARALLEL_ID]], task_id=[[INITIAL_TASK_ID]], actual_parallelism=0, index=1
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_thread_end: thread_id=[[MASTER_ID]]
  return 0;
}