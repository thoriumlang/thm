use vm::VM;
use vmlib::opcodes::Opcode;

mod vm;

fn main() {
    let mut vm = VM::new();

    // fibonacci(5) in r3
    vm.program = vec![
        /*0*/  u8::from(Opcode::LOAD), 0x00, 0x00, 0x00, 0x00, 0x05, // r0 = 5
        /*6*/  u8::from(Opcode::LOAD), 0x01, 0x00, 0x00, 0x00, 0x00, // r1 = 0
        /*12*/ u8::from(Opcode::LOAD), 0x02, 0x00, 0x00, 0x00, 0x01, // r2 = 1

        /*18*/ u8::from(Opcode::LOAD), 0x03, 0x00, 0x00, 0x00, 0x00, // r3 = 0
        /*24*/ u8::from(Opcode::LOAD), 0x04, 0x00, 0x00, 0x00, 0x01, // r4 = 1

        /*30*/ u8::from(Opcode::CMP), 0x00, 0x01,                    // r0 == r1?
        /*33*/ u8::from(Opcode::JE), 0x00, 0x00, 0x00, 0x37,         // if r0 == r1 -> jump 55

        /*38*/ u8::from(Opcode::MOV), 0x05, 0x03,                    // r5 = r3
        /*41*/ u8::from(Opcode::MOV), 0x03, 0x04,                    // r3 = r4
        /*44*/ u8::from(Opcode::ADD), 0x04, 0x05,                    // r4 += r5
        /*47*/ u8::from(Opcode::ADD), 0x01, 0x02,                    // r1 += r2
        /*50*/ u8::from(Opcode::JMP), 0x00, 0x00, 0x00, 0x1E,        // jump 30

        /*55*/ u8::from(Opcode::HALT)
    ];
    vm.run();

    println!("fibo(5): {}", vm.registers[3])
}
