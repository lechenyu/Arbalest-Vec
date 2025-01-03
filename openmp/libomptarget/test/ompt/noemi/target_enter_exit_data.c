// RUN: %libomptarget-compile-generic -DNOWAIT=0 && %libomptarget-run-generic 2>&1 | %fcheck-generic --check-prefixes CHECK,SYNC
// RUN: %libomptarget-compile-generic -DNOWAIT=1 && %libomptarget-run-generic 2>&1 | %fcheck-generic --check-prefixes CHECK,ASYNC
// REQUIRES: ompt

#include "callback-noemi.h"
#include "omp.h"

#define N 2

int main() {
  printf("host_num = %" PRIu32 "\n", omp_get_initial_device());
  int a[N] = {0};

  // target 1 (target enter data)
  #pragma omp target enter data map(to: a[0:N]) NOWAIT_CLAUSE
  print_fuzzy_address(1);
#if NOWAIT
  #pragma omp taskwait
#endif

  // target 2 (target)
  #pragma omp target map(a[0: N]) NOWAIT_CLAUSE
  {
    for (int i = 0; i < N; i++) {
      a[i] = 1;
    }
  }
  print_fuzzy_address(2);
#if NOWAIT
  #pragma omp taskwait
#endif

  // target 3 (target exit data)
  #pragma omp target exit data map(from: a[0: N]) NOWAIT_CLAUSE
  print_fuzzy_address(3);
#if NOWAIT
  #pragma omp taskwait
#endif

  // CHECK-NOT: {{^}}0: Could not register callback
  // CHECK: 0: NULL_POINTER=[[NULL:.*$]]

  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_thread_begin: thread_type=ompt_thread_initial=1, thread_id=[[MASTER_ID]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_initial_task_begin
  // CHECK-SAME: parallel_id=[[PARALLEL_ID:[0-9]+]], task_id=[[INITIAL_TASK_ID:[0-9]+]], actual_parallelism=1, index=1, flags=1
  // CHECK: host_num = [[HOST_NUM:[0-9]+]]


  /** target 1 (target enter data) **/

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_begin
  // SYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_1:[0-9]+]], device_num=[[DEVICE_NUM:[0-9]+]]
  // SYNC-SAME: kind=ompt_target_enter_data, codeptr_ra=[[TARGET_RETURN_ADDRESS_1:0x[0-f]+]]{{[0-f][0-f]}}
  
  // ASYNC: {{^}}[[MASTER_ID]]: ompt_event_task_create
  // ASYNC-SAME: parent_task_id=[[INITIAL_TASK_ID]], parent_task_frame.exit=(nil), parent_task_frame.reenter=0x{{[0-f]+}}
  // ASYNC-SAME: new_task_id=[[TARGET_TASK_ID_1:[0-9]+]], codeptr_ra=0x{{[0-f]+}}
  // ASYNC-SAME: task_type=ompt_task_explicit|ompt_task_target
  // ASYNC: {{^}}[[THREAD_ID_1:[0-9]+]]: ompt_event_target_begin
  // ASYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_1:[0-9]+]], device_num=[[DEVICE_NUM:[0-9]+]]
  // ASYNC-SAME: kind=ompt_target_enter_data_nowait, codeptr_ra=(nil)
  
  /** offload a synchronously **/
  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_data_op
  // SYNC-SAME: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_1:[0-9]+]], optype=ompt_target_data_alloc, src_addr=[[SRC_ADDR:0x[0-f]+]]
  // SYNC-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[DEST_ADDR:0x[0-f]+]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // SYNC-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], device_mem_flag=ompt_device_mem_flag_alloc|ompt_device_mem_flag_associate
  // SYNC-SAME: host_base_addr=[[HOST_BASE_ADDR:0x[0-f]+]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // SYNC-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}
  
  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // SYNC-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], device_mem_flag=ompt_device_mem_flag_to
  // SYNC-SAME: host_base_addr=[[HOST_BASE_ADDR]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // SYNC-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_data_op
  // SYNC-SAME: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_2:[0-9]+]], optype=ompt_target_data_transfer_to_device, src_addr=[[SRC_ADDR]]
  // SYNC-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[DEST_ADDR]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}

  /** offload a asynchronously **/
  // ASYNC: {{^}}[[THREAD_ID_1]]: ompt_event_target_data_op
  // ASYNC-SAME: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_1:[0-9]+]], optype=ompt_target_data_alloc, src_addr=[[SRC_ADDR:0x[0-f]+]]
  // ASYNC-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[DEST_ADDR:0x[0-f]+]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=(nil)

  // ASYNC: {{^}}[[THREAD_ID_1]]: ompt_event_device_mem
  // ASYNC-SAME: target_task_id=[[TARGET_TASK_ID_1]], target_id=[[TARGET_ID_1]], device_mem_flag=ompt_device_mem_flag_alloc|ompt_device_mem_flag_associate
  // ASYNC-SAME: host_base_addr=[[HOST_BASE_ADDR:0x[0-f]+]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // ASYNC-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=(nil)
  
  // ASYNC: {{^}}[[THREAD_ID_1]]: ompt_event_device_mem
  // ASYNC-SAME: target_task_id=[[TARGET_TASK_ID_1]], target_id=[[TARGET_ID_1]], device_mem_flag=ompt_device_mem_flag_to
  // ASYNC-SAME: host_base_addr=[[HOST_BASE_ADDR]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // ASYNC-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=(nil)

  // ASYNC: {{^}}[[THREAD_ID_1]]: ompt_event_target_data_op
  // ASYNC-SAME: target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_2:[0-9]+]], optype=ompt_target_data_transfer_to_device, src_addr=[[SRC_ADDR]]
  // ASYNC-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[DEST_ADDR]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=(nil)


  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_map
  // SYNC-SAME: target_id=[[TARGET_ID_1]], nitems=1, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}
  // SYNC-NEXT: {{^}}[[MASTER_ID]]: map: host_addr=[[SRC_ADDR]], device_addr=[[DEST_ADDR]], bytes=8, mapping_flag=ompt_target_map_flag_to

  // ASYNC: {{^}}[[THREAD_ID_1]]: ompt_event_target_map
  // ASYNC-SAME: target_id=[[TARGET_ID_1]], nitems=1, codeptr_ra=(nil)
  // ASYNC-NEXT: {{^}}[[THREAD_ID_1]]: map: host_addr=[[SRC_ADDR]], device_addr=[[DEST_ADDR]], bytes=8, mapping_flag=ompt_target_map_flag_to

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_end
  // SYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_1]], device_num=[[DEVICE_NUM]]
  // SYNC-SAME: kind=ompt_target_enter_data, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}
  // SYNC: {{^}}[[MASTER_ID]]: fuzzy_address={{.*}}[[TARGET_RETURN_ADDRESS_1]]{{[0-f][0-f]}}

  // ASYNC: {{^}}[[THREAD_ID_1]]: ompt_event_target_end
  // ASYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_1]], device_num=[[DEVICE_NUM]]
  // ASYNC-SAME: kind=ompt_target_enter_data_nowait, codeptr_ra=(nil)


  /** target 2 (target) **/

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_begin
  // SYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_2:[0-9]+]], device_num=[[DEVICE_NUM]]
  // SYNC-SAME: kind=ompt_target, codeptr_ra=[[TARGET_RETURN_ADDRESS_2:0x[0-f]+]]{{[0-f][0-f]}}

  // ASYNC: {{^}}[[MASTER_ID]]: ompt_event_task_create
  // ASYNC-SAME: parent_task_id=[[INITIAL_TASK_ID]], parent_task_frame.exit=(nil), parent_task_frame.reenter=0x{{[0-f]+}}
  // ASYNC-SAME: new_task_id=[[TARGET_TASK_ID_2:[0-9]+]], codeptr_ra=0x{{[0-f]+}}
  // ASYNC-SAME: task_type=ompt_task_explicit|ompt_task_target
  // ASYNC: {{^}}[[THREAD_ID_2:[0-9]+]]: ompt_event_target_begin
  // ASYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_2:[0-9]+]], device_num=[[DEVICE_NUM]]
  // ASYNC-SAME: kind=ompt_target_nowait, codeptr_ra=(nil)

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_map
  // SYNC-SAME: target_id=[[TARGET_ID_2]], nitems=1, codeptr_ra=[[TARGET_RETURN_ADDRESS_2]]{{[0-f][0-f]}}
  // SYNC-NEXT: {{^}}[[MASTER_ID]]: map: host_addr=[[SRC_ADDR]], device_addr=[[DEST_ADDR]], bytes=8, mapping_flag=ompt_target_map_flag_to|ompt_target_map_flag_from

  // ASYNC: {{^}}[[THREAD_ID_2]]: ompt_event_target_map
  // ASYNC-SAME: target_id=[[TARGET_ID_2]], nitems=1, codeptr_ra=(nil)
  // ASYNC-NEXT: {{^}}[[THREAD_ID_2]]: map: host_addr=[[SRC_ADDR]], device_addr=[[DEST_ADDR]], bytes=8, mapping_flag=ompt_target_map_flag_to|ompt_target_map_flag_from

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_submit
  // SYNC-SAME: target_id=[[TARGET_ID_2]], host_op_id=[[HOST_OP_ID_3:[0-9]+]], requested_num_teams=1

  // ASYNC: {{^}}[[THREAD_ID_2]]: ompt_event_target_submit
  // ASYNC-SAME: target_id=[[TARGET_ID_2]], host_op_id=[[HOST_OP_ID_3:[0-9]+]], requested_num_teams=1

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_end
  // SYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_2]], device_num=[[DEVICE_NUM]]
  // SYNC-SAME: kind=ompt_target, codeptr_ra=[[TARGET_RETURN_ADDRESS_2]]{{[0-f][0-f]}}
  // SYNC: {{^}}[[MASTER_ID]]: fuzzy_address={{.*}}[[TARGET_RETURN_ADDRESS_2]]{{[0-f][0-f]}}

  // ASYNC: {{^}}[[THREAD_ID_2]]: ompt_event_target_end
  // ASYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_2]], device_num=[[DEVICE_NUM]]
  // ASYNC-SAME: kind=ompt_target_nowait, codeptr_ra=(nil)


  /** target 3 (target exit data) **/

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_begin
  // SYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_3:[0-9]+]], device_num=[[DEVICE_NUM]]
  // SYNC-SAME: kind=ompt_target_exit_data, codeptr_ra=[[TARGET_RETURN_ADDRESS_3:0x[0-f]+]]{{[0-f][0-f]}}

  // ASYNC: {{^}}[[MASTER_ID]]: ompt_event_task_create
  // ASYNC-SAME: parent_task_id=[[INITIAL_TASK_ID]], parent_task_frame.exit=(nil), parent_task_frame.reenter=0x{{[0-f]+}}
  // ASYNC-SAME: new_task_id=[[TARGET_TASK_ID_3:[0-9]+]], codeptr_ra=0x{{[0-f]+}}
  // ASYNC-SAME: task_type=ompt_task_explicit|ompt_task_target
  // ASYNC: {{^}}[[THREAD_ID_3:[0-9]+]]: ompt_event_target_begin
  // ASYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_3:[0-9]+]], device_num=[[DEVICE_NUM]]
  // ASYNC-SAME: kind=ompt_target_exit_data_nowait, codeptr_ra=(nil)

  /** offload a back synchronously **/
  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // SYNC-SAME: target_task_id=0, target_id=[[TARGET_ID_3]], device_mem_flag=ompt_device_mem_flag_from
  // SYNC-SAME: host_base_addr=[[HOST_BASE_ADDR]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // SYNC-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_data_op
  // SYNC-SAME: target_id=[[TARGET_ID_3]], host_op_id=[[HOST_OP_ID_4:[0-9]+]], optype=ompt_target_data_transfer_from_device, src_addr=[[DEST_ADDR]]
  // SYNC-SAME: src_device_num=[[DEVICE_NUM]], dest_addr=[[SRC_ADDR]], dest_device_num=[[HOST_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_data_op
  // SYNC-SAME: target_id=[[TARGET_ID_3]], host_op_id=[[HOST_OP_ID_5:[0-9]+]], optype=ompt_target_data_delete, src_addr=[[SRC_ADDR]]
  // SYNC-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[DEST_ADDR]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // SYNC-SAME: target_task_id=0, target_id=[[TARGET_ID_3]], device_mem_flag=ompt_device_mem_flag_release|ompt_device_mem_flag_disassociate
  // SYNC-SAME: host_base_addr=[[HOST_BASE_ADDR]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // SYNC-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}

  /** offload a back asynchronously **/
  // ASYNC: {{^}}[[THREAD_ID_3]]: ompt_event_device_mem
  // ASYNC-SAME: target_task_id=[[TARGET_TASK_ID_3]], target_id=[[TARGET_ID_3]], device_mem_flag=ompt_device_mem_flag_from
  // ASYNC-SAME: host_base_addr=[[HOST_BASE_ADDR]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // ASYNC-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=(nil)

  // ASYNC: {{^}}[[THREAD_ID_3]]: ompt_event_target_data_op
  // ASYNC-SAME: target_id=[[TARGET_ID_3]], host_op_id=[[HOST_OP_ID_4:[0-9]+]], optype=ompt_target_data_transfer_from_device, src_addr=[[DEST_ADDR]]
  // ASYNC-SAME: src_device_num=[[DEVICE_NUM]], dest_addr=[[SRC_ADDR]], dest_device_num=[[HOST_NUM]], bytes=8, codeptr_ra=(nil)

  // ASYNC: {{^}}[[THREAD_ID_3]]: ompt_event_target_data_op
  // ASYNC-SAME: target_id=[[TARGET_ID_3]], host_op_id=[[HOST_OP_ID_5:[0-9]+]], optype=ompt_target_data_delete, src_addr=[[SRC_ADDR]]
  // ASYNC-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[DEST_ADDR]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=(nil)

  // ASYNC: {{^}}[[THREAD_ID_3]]: ompt_event_device_mem
  // ASYNC-SAME: target_task_id=[[TARGET_TASK_ID_3]], target_id=[[TARGET_ID_3]], device_mem_flag=ompt_device_mem_flag_release|ompt_device_mem_flag_disassociate
  // ASYNC-SAME: host_base_addr=[[HOST_BASE_ADDR]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // ASYNC-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=(nil)


  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_map
  // SYNC-SAME: target_id=[[TARGET_ID_3]], nitems=1, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}
  // SYNC-NEXT: {{^}}[[MASTER_ID]]: map: host_addr=[[SRC_ADDR]], device_addr=[[DEST_ADDR]], bytes=8, mapping_flag=ompt_target_map_flag_from

  // ASYNC: {{^}}[[THREAD_ID_3]]: ompt_event_target_map
  // ASYNC-SAME: target_id=[[TARGET_ID_3]], nitems=1, codeptr_ra=(nil)
  // ASYNC-NEXT: {{^}}[[THREAD_ID_3]]: map: host_addr=[[SRC_ADDR]], device_addr=[[DEST_ADDR]], bytes=8, mapping_flag=ompt_target_map_flag_from

  // SYNC: {{^}}[[MASTER_ID]]: ompt_event_target_end
  // SYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_3]], device_num=[[DEVICE_NUM]]
  // SYNC-SAME: kind=ompt_target_exit_data, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}
  // SYNC: {{^}}[[MASTER_ID]]: fuzzy_address={{.*}}[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}

  // ASYNC: {{^}}[[THREAD_ID_3]]: ompt_event_target_end
  // ASYNC-SAME: task_id=[[INITIAL_TASK_ID]], target_id=[[TARGET_ID_3]], device_num=[[DEVICE_NUM]]
  // ASYNC-SAME: kind=ompt_target_exit_data_nowait, codeptr_ra=(nil)


  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_initial_task_end
  // CHECK-SAME: parallel_id=[[PARALLEL_ID]], task_id=[[INITIAL_TASK_ID]], actual_parallelism=0, index=1
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_thread_end: thread_id=[[MASTER_ID]]
  return 0;
}