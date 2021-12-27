// types: word, byte
// pointer: prefix with '@'
// function decl: 'fn'
// const decl: 'const'
// variable decl: 'var'
// 'public' to export symbol (any) - prefixed to with the symbol's definition
// define extern symbol: 'extern'
// define non-cacheable symbols: 'volatile'

// address of variable: @var
// value at address:    $var
// ascii-z quotes: "", escape: \"
// ascii quotes: '', escape \'
// array access: .expr: a.0, a.(0+4) ([expr] might be implemented later)

// operators:
// +
// -
// *
// /
// &
// |
// ^
// ~
// &&
// ||
// !
// ==
// !=
// >
// >=
// <
// <=
// ::      cast
// @ address of
// $ dereference

// comment
/* multiline */

// raw asm section
asm {
    MOV r0, 1
}

// struct: 'struct'
// union: 'union'
// enum: 'enum'
// bitflag: 'bitflag' -- max values count: word_t's size (i.e. 32)

struct my_struct {
    field1: type,
};
union my_union {
    value1: type,
}
enum my_enum {
    value1 = 0,
    value2,
}
bitflag my_bits {
    flag1 = 1,
    flag2,
}

public fn main(argc: word, argv: @@byte): word {
}
fn local() { ... }

const my_const: word = xxx;
var my_var: byte = xxx;
var my_var_ptr: @byte = @my_var // my_var_ptr is now a pointer to my_var

@my_const::@byte.1 // casts @my_const to @byte and take first element

word i = 0;
while (i < 10) {
  i = i + 1;
  break;
  ...
}

// may implement for as: for (word i = 0; i < 10; i = i + 1) { ... } <=> word i = 0; while (i < 10) { i = i + 1; if (!(i<10)) { break; } ... }
// may implement loop as: loop { ... } <=> while (true) { ... }

if (expr) { ... } else { if (expr) { ... } }
if (expr) { ... } else if (expr) { ... } else { ... } <=> if (expr) { ... } else { if (expr) { ... } }

// may implement unless as: unless (expr) <=> if (!expr)
// may implement switch as: switch (expr) { (expr_a), (expr_b) { ... } (expr_c) { ... } () { ... } } <=> if ((expr == expr_a) || (expr == expr_b)) { ... } else if (expr == expr_c) { ... } else { ... }

// make the blinking screen

extern const __int_vsync: word;
extern const __int_timer: word;
extern const __int_keyboard: word;
extern const __idt_start: word;
extern const __keyboard_out: word;
extern const __video_meta: word;
extern const __video_buffer1: word;
extern const __video_buffer2: word;
extern const __video_buffer_size: word;

const seconds      : word = 3
const color_red    : word = 0x000000ff
const color_blue   : word = 0x00ff0000
const color_black  : word = 0x00000000
const color_white  : word = 0x00ffffff

var vsync_flag   : word = 0
var active       : word = 0
var prev_key     : word = 0

fn main() {
    setup_vsync_int_handler();
    setup_timer_int_handler();
    setup_keyboard_int_handler();
}

fn setup_vsync_int_handler() {

}

fn setup_timer_int_handler() {

}

fn setup_keyboard_int_handler() {

}
