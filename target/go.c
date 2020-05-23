#include <ir/ir.h>
#include <target/util.h>

static void go_init_state() {
    for (int i = 0; i < 7; i++) {
      emit_line("var %s = 0", reg_names[i]);
    }
    emit_line("var mem = make([]int, 1 << 24)");
}

static void go_init_state2(Data* data) {
  for (int mp = 0; data; data = data->next, mp++) {
    if (data->v) {
      emit_line("mem[%d] = %d", mp, data->v);
    }
  }
}

// func f0() {
// }
static void go_emit_func_prologue(int func_id) {
  emit_line("");
  emit_line("func f%d() {", func_id);
  inc_indent();
  emit_line("for %d <= pc && pc < %d {",
              func_id * CHUNKED_FUNC_SIZE,
              (func_id + 1) * CHUNKED_FUNC_SIZE);
  inc_indent();
  emit_line("switch {");
}

static void go_emit_func_epilogue(void) {
    emit_line("} // switch");
    emit_line("pc += 1");
    dec_indent();
    emit_line("} // for");
    dec_indent();
    emit_line("} // func f d");
}

static void go_emit_pc_change(int pc) {
  emit_line("case pc == %d:", pc);
}

static void go_emit_inst(Inst* inst) {
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
    emit_line("`SUB`");
    break;

  case LOAD:
    emit_line("%s = mem[%s]", reg_names[inst->dst.reg], src_str(inst));
    break;

  case STORE:
    emit_line("mem[%s] = %s", src_str(inst), reg_names[inst->dst.reg]);
    break;

  case PUTC:
    emit_line("os.Stdout.WriteString(string(%s))", src_str(inst));
    break;

  case GETC:
    emit_line("{ // GETC start");
    inc_indent();
    emit_line("buf := make([]byte, 1)");
    emit_line("n, err := os.Stdin.Read(buf[:])");
    emit_line("if n != 0 && err == nil {");
    inc_indent();
    emit_line("%s = int(buf[0])", reg_names[inst->dst.reg]);
    dec_indent();
    emit_line("} else {");
    inc_indent();
    emit_line("%s = 0", reg_names[inst->dst.reg]);
    dec_indent();
    emit_line("}");
    dec_indent();
    emit_line("} // GETC end");
    break;

  case EXIT:
    emit_line("os.Exit(0)");
    break;

  case DUMP:
    break;

  case EQ:
    emit_line("`EQ`");
    break;

  case NE:
    emit_line("`NE`");
    break;

  case LT:
    emit_line("`LT`");
    break;

  case GT:
    emit_line("`GT`");
    break;

  case LE:
    emit_line("`LE`");
    break;

  case GE:
    emit_line("`GE`");
    break;

  case JEQ:
  case JNE:
  case JLT:
  case JGT:
  case JLE:
  case JGE:
  case JMP:
    emit_line("if %s {", cmp_str(inst, "true"));
    inc_indent();
    emit_line("pc = %s - 1", value_str(&inst->jmp));
    dec_indent();
    emit_line("} // if compare");
    break;

  default:
    error("oops");
  }

  dec_indent();
}

void target_go(Module* module) {
  emit_line("package main");
  emit_line("");
  emit_line("import (");
  inc_indent();
  emit_line("\"os\"");
  dec_indent();
  emit_line(")");
  emit_line("");
  go_init_state();

  int num_funcs = emit_chunked_main_loop(module->text,
                                           go_emit_func_prologue,
                                           go_emit_func_epilogue,
                                           go_emit_pc_change,
                                           go_emit_inst);

  emit_line("");
  emit_line("func main() {");
  inc_indent();
  go_init_state2(module->data);

  emit_line("");
  emit_line("for {");
  inc_indent();
  emit_line("switch {");

  for (int i = 0; i < num_funcs; i++) {
    emit_line("case %d <= pc && pc < %d:", i * CHUNKED_FUNC_SIZE, (i + 1) * CHUNKED_FUNC_SIZE);
    inc_indent();
    emit_line("f%d()", i);
    dec_indent();
  }
  emit_line("} // switch");
  dec_indent();
  emit_line("} // for");

  dec_indent();
  emit_line("} // func main");
}
