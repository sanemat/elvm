#include <ir/ir.h>
#include <target/util.h>

static void rb_init_state(Data* data) {
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

static void rb_emit_func_prologue(int func_id) {
    emit_line("# prologue, func_id: %d", func_id);
}

static void rb_emit_func_epilogue(void) {
    emit_line("# epilogue");
}

static void rb_emit_pc_change(int pc) {
    emit_line("# pc: %d", pc);
}

static void rb_emit_inst(Inst* inst) {
    switch (inst->op) {
    default:
        emit_line("# op");
    }
}

void target_rb(Module* module) {
  rb_init_state(module->data);

  int num_funcs = emit_chunked_main_loop(module->text,
                                           rb_emit_func_prologue,
                                           rb_emit_func_epilogue,
                                           rb_emit_pc_change,
                                           rb_emit_inst);
  emit_line("# num_funcs: %d", num_funcs);

}
