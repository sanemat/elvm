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
    emit_line("\"ADD\"");
    break;

  case SUB:
    emit_line("\"SUB\"");
    break;

  case LOAD:
    emit_line("\"LOAD\"");
    break;

  case STORE:
    emit_line("\"STORE\"");
    break;

  case PUTC:
    emit_line("print %s.chr", src_str(inst));
    break;

  case GETC:
    emit_line("\"GETC\"");
    break;

  case EXIT:
    emit_line("exit 0");
    break;

  case DUMP:
    emit_line("\"DUMP\"");
    break;

  case EQ:
    emit_line("\"EQ\"");
    break;

  case NE:
    emit_line("\"NE\"");
    break;

  case LT:
    emit_line("\"LT\"");
    break;

  case GT:
    emit_line("\"GT\"");
    break;

  case LE:
    emit_line("\"LE\"");
    break;

  case GE:
    emit_line("\"GE\"");
    break;

  case JEQ:
    emit_line("\"JEQ\"");
    break;

  case JNE:
    emit_line("\"JNE\"");
    break;

  case JLT:
    emit_line("\"JLT\"");
    break;

  case JGT:
    emit_line("\"JGT\"");
    break;

  case JLE:
    emit_line("\"JLE\"");
    break;

  case JGE:
    emit_line("\"JGE\"");
    break;

  case JMP:
    emit_line("@pc = %s - 1", value_str(&inst->jmp));
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
  emit_line("end");
  dec_indent();
  emit_line("end");
}
