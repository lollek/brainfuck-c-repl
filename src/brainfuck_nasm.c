#include "brainfuck_nasm.h"

static unsigned char stack[256] = { 0 };
static unsigned char ptr = 0;

static inline void op_put(FILE *output) {
  fprintf(output, "\tcall\t_put\n");
}

static inline void op_get(FILE *output) {
  fprintf(output, "\tcall\t_get\n");
}

static inline void op_inc(FILE *output) {
  fprintf(output, "\tinc\tbyte[ebp]\n");
}

static inline void op_dec(FILE *output) {
  fprintf(output, "\tdec\tbyte[ebp]\n");
}

static inline void op_mvr(FILE *output) {
  fprintf(output, "\tinc\tebp\n");
}

static inline void op_mvl(FILE *output) {
  fprintf(output, "\tdec\tebp\n");
}

static void op_tag(FILE *output) {
  fprintf(output, "\tcmp\tbyte[ebp],0\n"
                  "\tjz\tend%d%d\n"
                  "tag%d%d:",
          ptr, stack[ptr], ptr, stack[ptr]);
  ++ptr;
}

static void op_jmp(FILE *output) {
  fprintf(output, "\tcmp\tbyte[ebp],0\n"
                  "\tjnz\ttag%d%d\n"
                  "end%d%d:",
                  ptr -1, stack[ptr -1], ptr -1, stack[ptr -1]);
  stack[--ptr]++;
}

int brainfuck_nasm_write(FILE *output, FILE *input) {
  fprintf(output, "section .text\n"
                  "\tglobal _start\n"
  /* PUT */       "_put:\t\t\t\t; sys_write(stdout, ebp, 1)\n"
                  "\tmov\teax,4\t\t; sys_write\n"
                  "\tmov\tebx,1\t\t; stdout\n"
                  "\tmov\tecx,ebp\n"
                  "\tint\t80h\n"
                  "\tret\n"
  /* GET */       "_get:\t\t\t\t; sys_read(stdin, ebp, 1)\n"
                  "\tmov\teax,3\t\t; sys_read\n"
                  "\tmov\tebx,0\t\t; stdin\n"
                  "\tmov\tecx,ebp\n"
                  "\tint\t80h\n"
                  "\tret\n"
  /* START */     "_start:\n"
                  "\tsub\tesp,7000\t; 7000 byte stack\n"
                  "\tmov\tebp,esp\t\t; Set ptr to start of stack\n"
                  "\tmov\tedx,1\t\t; edx (for _put and _get) is always 1\n");

  int op;
  while ((op = fgetc(input)) != EOF) {
    switch (op) {
      case '.': op_put(output); break;
      case ',': op_get(output); break;
      case '+': op_inc(output); break;
      case '-': op_dec(output); break;
      case '>': op_mvr(output); break;
      case '<': op_mvl(output); break;
      case '[': op_tag(output); break;
      case ']': op_jmp(output); break;
    }
  }

  fprintf(output, "\tmov\teax,1\t\t; sys_exit(0)\n"
                  "\tmov\tebx,0\n"
                  "\tint\t80h\n");
  return 0;
}
