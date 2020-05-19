#include <ir/ir.h>
#include <target/util.h>

static void init_state_rb(Data* data) {
  for (int i = 0; i < 7; i++) {
    emit_line("%s = 0", reg_names[i]);
  }
  emit_line("mem = [0] * (1 << 24)");
  for (int mp = 0; data; data = data->next, mp++) {
    if (data->v) {
      emit_line("mem[%d] = %d", mp, data->v);
    }
  }
}

void target_rb(Module* module) {
  init_state_rb(module->data);
}
