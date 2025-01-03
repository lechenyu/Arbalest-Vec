// RUN: %libomptarget-compile-generic && %libomptarget-run-generic 2>&1 | %fcheck-generic --check-prefixes CHECK
// REQUIRES: ompt

#include "callback-emi.h"
#include "omp.h"

#define N 2

int main() {
  printf("host_num = %" PRIu32 "\n", omp_get_initial_device());
  int a[N] = {0};

  // target 1 (target enter data)
  #pragma omp target data map(tofrom: a[0:N])
  {

    // target 2 (target)
    #pragma omp target map(a[0: N])
    {
      for (int i = 0; i < N; i++) {
        a[i] = 1;
      }
    }
    print_fuzzy_address(1);

  // target 3 (target exit data)
  }
  print_fuzzy_address(2);

  // CHECK-NOT: {{^}}0: Could not register callback
  // CHECK: 0: NULL_POINTER=[[NULL:.*$]]
  
  // CHECK: {{^}}[[MASTER_ID:[0-9]+]]: ompt_event_thread_begin: thread_type=ompt_thread_initial=1, thread_id=[[MASTER_ID]]
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_initial_task_begin
  // CHECK-SAME: parallel_id=[[PARALLEL_ID:[0-9]+]], task_id=[[INITIAL_TASK_ID:[0-9]+]], actual_parallelism=1, index=1, flags=1
  // CHECK: host_num = [[HOST_NUM:[0-9]+]]


  /** target 1 (target enter data) **/

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_emi_begin
  // CHECK-SAME: task_id=[[INITIAL_TASK_ID]], target_task_id=0, target_id=[[TARGET_ID_1:[0-9]+]], device_num=[[DEVICE_NUM:[0-9]+]]
  // CHECK-SAME: kind=ompt_target_enter_data, codeptr_ra=[[TARGET_RETURN_ADDRESS_1:0x[0-f]+]]
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op_emi_begin
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_1:[0-9]+]], optype=ompt_target_data_alloc, src_addr=[[SRC_ADDR:0x[0-f]+]]
  // CHECK-SAME: src_device_num=[[HOST_NUM]], dest_addr=(nil), dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op_emi_end
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_1]], optype=ompt_target_data_alloc, src_addr=[[SRC_ADDR]]
  // CHECK-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[DEST_ADDR:0x[0-f]+]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], device_mem_flag=ompt_device_mem_flag_alloc|ompt_device_mem_flag_associate
  // CHECK-SAME: host_base_addr=[[HOST_BASE_ADDR:0x[0-f]+]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // CHECK-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], device_mem_flag=ompt_device_mem_flag_to
  // CHECK-SAME: host_base_addr=[[HOST_BASE_ADDR]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // CHECK-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op_emi_begin
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_2:[0-9]+]], optype=ompt_target_data_transfer_to_device, src_addr=[[SRC_ADDR]]
  // CHECK-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[DEST_ADDR]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op_emi_end
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_1]], host_op_id=[[HOST_OP_ID_2]], optype=ompt_target_data_transfer_to_device, src_addr=[[SRC_ADDR]]
  // CHECK-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[DEST_ADDR]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_map_emi
  // CHECK-SAME: target_id=[[TARGET_ID_1]], nitems=1, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]
  // CHECK-NEXT: {{^}}[[MASTER_ID]]: map: host_addr=[[SRC_ADDR]], device_addr=[[DEST_ADDR]], bytes=8, mapping_flag=ompt_target_map_flag_to
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_emi_end
  // CHECK-SAME: task_id=[[INITIAL_TASK_ID]], target_task_id=0, target_id=[[TARGET_ID_1]], device_num=[[DEVICE_NUM]]
  // CHECK-SAME: kind=ompt_target_enter_data, codeptr_ra=[[TARGET_RETURN_ADDRESS_1]]
  

  /** target 2 (target) **/

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_emi_begin
  // CHECK-SAME: task_id=[[INITIAL_TASK_ID]], target_task_id=0, target_id=[[TARGET_ID_2:[0-9]+]], device_num=[[DEVICE_NUM]]
  // CHECK-SAME: kind=ompt_target, codeptr_ra=[[TARGET_RETURN_ADDRESS_2:0x[0-f]+]]{{[0-f][0-f]}}
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_map_emi
  // CHECK-SAME: target_id=[[TARGET_ID_2]], nitems=1, codeptr_ra=[[TARGET_RETURN_ADDRESS_2]]{{[0-f][0-f]}}
  // CHECK-NEXT: {{^}}[[MASTER_ID]]: map: host_addr=[[SRC_ADDR]], device_addr=[[DEST_ADDR]], bytes=8, mapping_flag=ompt_target_map_flag_to|ompt_target_map_flag_from

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_submit_emi_begin
  // CHECK-SAME: target_id=[[TARGET_ID_2]], host_op_id=[[HOST_OP_ID_3:[0-9]+]], requested_num_teams=1

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_submit_emi_end
  // CHECK-SAME: target_id=[[TARGET_ID_2]], host_op_id=[[HOST_OP_ID_3]], requested_num_teams=1

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_emi_end
  // CHECK-SAME: task_id=[[INITIAL_TASK_ID]], target_task_id=0, target_id=[[TARGET_ID_2]], device_num=[[DEVICE_NUM]]
  // CHECK-SAME: kind=ompt_target, codeptr_ra=[[TARGET_RETURN_ADDRESS_2]]{{[0-f][0-f]}}
  // CHECK: {{^}}[[MASTER_ID]]: fuzzy_address={{.*}}[[TARGET_RETURN_ADDRESS_2]]{{[0-f][0-f]}}
  

  /** target 3 (target exit data) **/
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_emi_begin
  // CHECK-SAME: task_id=[[INITIAL_TASK_ID]], target_task_id=0, target_id=[[TARGET_ID_3:[0-9]+]], device_num=[[DEVICE_NUM]]
  // CHECK-SAME: kind=ompt_target_exit_data, codeptr_ra=[[TARGET_RETURN_ADDRESS_3:0x[0-f]+]]{{[0-f][0-f]}}
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_3]], device_mem_flag=ompt_device_mem_flag_from
  // CHECK-SAME: host_base_addr=[[HOST_BASE_ADDR]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // CHECK-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op_emi_begin
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_3]], host_op_id=[[HOST_OP_ID_4:[0-9]+]], optype=ompt_target_data_transfer_from_device, src_addr=[[DEST_ADDR]]
  // CHECK-SAME: src_device_num=[[DEVICE_NUM]], dest_addr=[[SRC_ADDR]], dest_device_num=[[HOST_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op_emi_end
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_3]], host_op_id=[[HOST_OP_ID_4]], optype=ompt_target_data_transfer_from_device, src_addr=[[DEST_ADDR]]
  // CHECK-SAME: src_device_num=[[DEVICE_NUM]], dest_addr=[[SRC_ADDR]], dest_device_num=[[HOST_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op_emi_begin
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_3]], host_op_id=[[HOST_OP_ID_5:[0-9]+]], optype=ompt_target_data_delete, src_addr=[[SRC_ADDR]]
  // CHECK-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[DEST_ADDR]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_data_op_emi_end
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_3]], host_op_id=[[HOST_OP_ID_5]], optype=ompt_target_data_delete, src_addr=[[SRC_ADDR]]
  // CHECK-SAME: src_device_num=[[HOST_NUM]], dest_addr=[[DEST_ADDR]], dest_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_device_mem
  // CHECK-SAME: target_task_id=0, target_id=[[TARGET_ID_3]], device_mem_flag=ompt_device_mem_flag_release|ompt_device_mem_flag_disassociate
  // CHECK-SAME: host_base_addr=[[HOST_BASE_ADDR]], host_addr=[[SRC_ADDR]], host_device_num=[[HOST_NUM]]
  // CHECK-SAME: target_addr=[[DEST_ADDR]], target_device_num=[[DEVICE_NUM]], bytes=8, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}

  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_map_emi
  // CHECK-SAME: target_id=[[TARGET_ID_3]], nitems=1, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}
  // CHECK-NEXT: {{^}}[[MASTER_ID]]: map: host_addr=[[SRC_ADDR]], device_addr=[[DEST_ADDR]], bytes=8, mapping_flag=ompt_target_map_flag_from
  
  // CHECK: {{^}}[[MASTER_ID]]: ompt_event_target_emi_end
  // CHECK-SAME: task_id=[[INITIAL_TASK_ID]], target_task_id=0, target_id=[[TARGET_ID_3]], device_num=[[DEVICE_NUM]]
  // CHECK-SAME: kind=ompt_target_exit_data, codeptr_ra=[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}
  // CHECK: {{^}}[[MASTER_ID]]: fuzzy_address={{.*}}[[TARGET_RETURN_ADDRESS_3]]{{[0-f][0-f]}}

  return 0;
}