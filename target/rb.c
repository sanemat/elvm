#include <ir/ir.h>
#include <target/util.h>

static const char* RB_REG_NAMES[] = {
  "@a", "@b", "@c", "@d", "@bp", "@sp", "@pc"
};

static void rb_init_state(Data* data) {
  reg_names = RB_REG_NAMES;

  for (int i = 0; i < 7; i++) {
    emit_line("%s = 0", reg_names[i]);
  }
  emit_line("@mem = [0] * (1 << 24)");
  for (int mp = 0; data; data = data->next, mp++) {
    if (data->v) {
      emit_line("@mem[%d] = %d", mp, data->v);
    }
  }
}

// def f0
// end

static void rb_emit_func_prologue(int func_id) {
    emit_line("");
    emit_line("def f%d", func_id);
    inc_indent();
    emit_line("while %d <= @pc and @pc < %d",
              func_id * CHUNKED_FUNC_SIZE,
              (func_id + 1) * CHUNKED_FUNC_SIZE);
    inc_indent();
    emit_line("case @pc");
}

static void rb_emit_func_epilogue(void) {
    emit_line("end # case @pc");
    emit_line("@pc += 1");
    dec_indent();
    emit_line("end # while");
    dec_indent();
    emit_line("end # def f d");
}

static void rb_emit_pc_change(int pc) {
    emit_line("when %d", pc);
}

static void rb_emit_inst(Inst* inst) {
  reg_names = RB_REG_NAMES;

  inc_indent();

  switch (inst->op) {
  case MOV:
    emit_line("%s = %s", reg_names[inst->dst.reg], src_str(inst));
    break;

  case ADD:
    emit_line("%s = (%s + %s) & " UINT_MAX_STR,
              reg_names[inst->dst.reg],
              reg_names[inst->dst.reg], src_str(inst));
    break;

  case SUB:
    emit_line("%s = (%s - %s) & " UINT_MAX_STR,
              reg_names[inst->dst.reg],
              reg_names[inst->dst.reg], src_str(inst));
    break;

  case LOAD:
    emit_line("%s = @mem[%s]", reg_names[inst->dst.reg], src_str(inst));
    break;

  case STORE:
    emit_line("@mem[%s] = %s", src_str(inst), reg_names[inst->dst.reg]);
    break;

  case PUTC:
    emit_line("print %s.chr", src_str(inst));
    break;

  case GETC:
    emit_line("%s = gets(1)&.ord || 0", reg_names[inst->dst.reg]);
    break;

  case EXIT:
    emit_line("exit 0");
    break;

  case DUMP:
    emit_line("\"DUMP\"");
    break;

  case EQ:
  case NE:
  case LT:
  case GT:
  case LE:
  case GE:
    emit_line("%s = (%s) ? 1 : 0",
              reg_names[inst->dst.reg], cmp_str(inst, "true"));
    break;

  case JEQ:
  case JNE:
  case JLT:
  case JGT:
  case JLE:
  case JGE:
  case JMP:
    emit_line("if %s", cmp_str(inst, "true"));
    inc_indent();
    emit_line("@pc = %s - 1", value_str(&inst->jmp));
    dec_indent();
    emit_line("end # if compare");
    break;

  default:
    error("oops");
  }

  dec_indent();
}

void target_rb(Module* module) {
  rb_init_state(module->data);

  int num_funcs = emit_chunked_main_loop(module->text,
                                           rb_emit_func_prologue,
                                           rb_emit_func_epilogue,
                                           rb_emit_pc_change,
                                           rb_emit_inst);

  emit_line("");
  emit_line("loop do");
  inc_indent();
  emit_line("case @pc");
  for (int i = 0; i < num_funcs; i++) {
    emit_line("when %d..%d", i * CHUNKED_FUNC_SIZE, (i + 1) * CHUNKED_FUNC_SIZE - 1);
    inc_indent();
    emit_line("f%d()", i);
    dec_indent();
  }
  emit_line("end # case @pc");
  dec_indent();
  emit_line("end # loop");
}
