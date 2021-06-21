extern crate vmlib;

use vmlib::{MEMORY_SIZE, REG_COUNT, STACK_SIZE};
use vmlib::op::Op;

pub struct Flags {
    zero: bool,
    negative: bool,
}

pub struct CPU {
    // FIXME remove pub
    pub registers: [i32; REG_COUNT],
    /// program pointer (aka ip)
    pc: usize,
    // FIXME remove pub
    pub program: Vec<u8>,
    flags: Flags,
    /// stack pointer
    sp: usize,
    pub memory: [u8; MEMORY_SIZE],
}

impl CPU {
    pub fn new() -> CPU {
        CPU {
            registers: [0; REG_COUNT],
            pc: 0,
            program: vec![],
            flags: Flags {
                zero: true,
                negative: false,
            },
            sp: STACK_SIZE,
            memory: [0; MEMORY_SIZE],
        }
    }

    pub fn run(&mut self) {
        loop {
            match Self::decode_opcode(self.fetch_opcode()) {
                Op::NOP => { continue; }
                Op::HALT => {
                    return;
                }
                Op::PANIC => {
                    println!("Panic!");
                    return;
                }
                Op::LOAD => {
                    let r = self.fetch_1byte() as usize;
                    let immediate = self.fetch_4bytes() as i32;
                    self.registers[r] = immediate;
                    self.flags.zero = self.registers[r] == 0;
                    self.flags.negative = self.registers[r] < 0;
                }
                Op::MOV => {
                    let r0 = self.fetch_1byte() as usize;
                    let r1 = self.fetch_1byte() as usize;
                    self.registers[r0] = self.registers[r1];
                    self.flags.zero = self.registers[r0] == 0;
                    self.flags.negative = self.registers[r0] < 0;
                }
                Op::ADD => {
                    let r0 = self.fetch_1byte() as usize;
                    let r1 = self.fetch_1byte() as usize;
                    self.registers[r0] += self.registers[r1];
                    self.flags.zero = self.registers[r0] == 0;
                    self.flags.negative = self.registers[r0] < 0;
                }
                Op::CMP => {
                    let r0 = self.fetch_1byte() as usize;
                    let r1 = self.fetch_1byte() as usize;
                    self.flags.zero = self.registers[r0] == self.registers[r1];
                    self.flags.negative = self.registers[r0] < self.registers[r1];
                }
                Op::JMP => {
                    self.pc = self.fetch_4bytes() as usize;
                }
                Op::JE => {
                    if self.flags.zero {
                        self.pc = self.fetch_4bytes() as usize;
                    } else {
                        self.skip_4bytes();
                    }
                }
                Op::JNE => {
                    if !self.flags.zero {
                        self.pc = self.fetch_4bytes() as usize;
                    } else {
                        self.skip_4bytes();
                    }
                }
                Op::INC => {
                    let r = self.fetch_1byte() as usize;
                    self.registers[r] += 1;
                    self.flags.zero = self.registers[r] == 0;
                    self.flags.negative = self.registers[r] < 0;
                }
                Op::DEC => {
                    let r = self.fetch_1byte() as usize;
                    self.registers[r] -= 1;
                    self.flags.zero = self.registers[r] == 0;
                    self.flags.negative = self.registers[r] < 0;
                }
                Op::PUSH => {
                    // we map r = 0x12345678 like to:
                    // sp = 78, sp-1 = 56, sp-2 = 34 sp-3 = 12
                    let r = self.fetch_1byte() as usize;
                    for byte in self.registers[r].to_le_bytes().iter() {
                        self.sp -= 1;
                        self.memory[self.sp] = *byte;
                    }
                }
                Op::POP => {
                    // we map sp = 78, sp-1 = 56, sp-2 = 34 sp-3 = 12 like to:
                    // r = 0x12345678
                    let r = self.fetch_1byte() as usize;
                    let mut bytes: [u8; 4] = [0; 4];
                    for i in 0..4 {
                        bytes[i] = self.memory[self.sp];
                        self.sp += 1;
                    }
                    self.registers[r] = i32::from_be_bytes(bytes);
                    self.flags.zero = self.registers[r] == 0;
                    self.flags.negative = self.registers[r] < 0;
                }
            }
        }
    }

    fn fetch_opcode(&mut self) -> u8 {
        if self.pc >= self.program.len() {
            return Op::PANIC.bytecode();
        }

        let opcode = self.program[self.pc];
        self.pc += 1;

        return opcode;
    }

    fn decode_opcode(opcode: u8) -> Op {
        Op::from(opcode)
    }

    fn fetch_1byte(&mut self) -> u8 {
        let byte = self.program[self.pc];
        self.pc += 1;
        return byte;
    }

    fn fetch_4bytes(&mut self) -> u32 {
        let result = ((self.program[self.pc] as u32) << 24)
            | ((self.program[self.pc + 1] as u32) << 16)
            | ((self.program[self.pc + 2] as u32) << 8)
            | (self.program[self.pc + 3] as u32);
        self.pc += 4;
        return result;
    }

    fn skip_4bytes(&mut self) {
        self.pc += 4;
    }
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_create_cpu() {
        let cpu = CPU::new();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.pc, 0);
    }

    #[test]
    fn test_load() {
        let mut cpu = CPU::new();
        cpu.program = vec![
            // LOAD 0, #16909320
            Op::LOAD.bytecode(), 0x00, 0x01, 0x02, 0x04, 0x08,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 16909320);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_load_zero() {
        let mut cpu = CPU::new();
        cpu.program = vec![
            // LOAD 0, #0
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x00,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_mov() {
        let mut cpu = CPU::new();
        cpu.registers[1] = 1;
        cpu.program = vec![
            // LOAD 0, #1
            Op::MOV.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.registers[1], 1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_mov_zero() {
        let mut cpu = CPU::new();
        cpu.registers[0] = 1;
        cpu.program = vec![
            // LOAD 0, #1
            Op::MOV.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.registers[1], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_add() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 1;
        cpu.registers[1] = 2;
        cpu.program = vec![
            Op::ADD.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 3);
        assert_eq!(cpu.registers[1], 2);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_add_zero() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 0;
        cpu.registers[1] = 0;
        cpu.program = vec![
            Op::ADD.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.registers[1], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_cmp_eq() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 1;
        cpu.registers[1] = 1;
        cpu.program = vec![
            Op::CMP.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.registers[1], 1);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_cmp_lt() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 1;
        cpu.registers[1] = 2;
        cpu.program = vec![
            Op::CMP.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.registers[1], 2);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, true);
    }

    #[test]
    fn test_cmp_gt() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 2;
        cpu.registers[1] = 1;
        cpu.program = vec![
            Op::CMP.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 2);
        assert_eq!(cpu.registers[1], 1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_jmp() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // JMP #12 (i.e. LOAD 0, #2)
            Op::JMP.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 2);
    }

    #[test]
    fn test_je_zero() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // JE #12 (i.e. LOAD 0, #2)
            Op::JE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        cpu.flags.zero = true;
        cpu.run();
        assert_eq!(cpu.registers[0], 2);
    }

    #[test]
    fn test_je_nonzero() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // JE #12 (i.e. LOAD 0, #2)
            Op::JE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        cpu.flags.zero = false;
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
    }

    #[test]
    fn test_jne_zero() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // JNE #12 (i.e. LOAD 0, #2)
            Op::JNE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        cpu.flags.zero = false;
        cpu.run();
        assert_eq!(cpu.registers[0], 2);
    }

    #[test]
    fn test_jne_nonzero() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // JNE #12 (i.e. LOAD 0, #2)
            Op::JNE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        cpu.flags.zero = true;
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
    }

    #[test]
    fn test_inc() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // INC r0
            Op::INC.bytecode(), 0x00
        ];
        cpu.registers[0] = 0;
        cpu.flags.zero = true;
        cpu.flags.negative = false;
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_inc_zero() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // INC r0
            Op::INC.bytecode(), 0x00
        ];
        cpu.registers[0] = -1;
        cpu.flags.zero = false;
        cpu.flags.negative = true;
        cpu.run();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(true, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_inc_negative() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // INC r0
            Op::INC.bytecode(), 0x00
        ];
        cpu.registers[0] = -2;
        cpu.flags.zero = false;
        cpu.flags.negative = true;
        cpu.run();
        assert_eq!(cpu.registers[0], -1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(true, cpu.flags.negative);
    }

    #[test]
    fn test_dec() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // DEC r0
            Op::DEC.bytecode(), 0x00
        ];
        cpu.registers[0] = 2;
        cpu.flags.zero = false;
        cpu.flags.negative = false;
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_dec_zero() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // DEC r0
            Op::DEC.bytecode(), 0x00
        ];
        cpu.registers[0] = 1;
        cpu.flags.zero = false;
        cpu.flags.negative = false;
        cpu.run();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(true, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_dec_negative() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // DEC r0
            Op::DEC.bytecode(), 0x00
        ];
        cpu.registers[0] = 0;
        cpu.flags.zero = true;
        cpu.flags.negative = false;
        cpu.run();
        assert_eq!(cpu.registers[0], -1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(true, cpu.flags.negative);
    }

    #[test]
    fn test_push() {
        let mut cpu = CPU::new();

        cpu.sp = 4;
        cpu.registers[0] = 0x01020304;
        cpu.flags.zero = true;
        cpu.flags.negative = true;
        cpu.program = vec![
            // PUSH r0
            Op::PUSH.bytecode(), 0x00
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 0x01020304, "{} != 1", cpu.registers[0]);
        assert_eq!(true, cpu.flags.zero, "zero flag not set");
        assert_eq!(true, cpu.flags.negative, "negative flag not set");
        assert_eq!(0, cpu.sp, "sp {} != 0", cpu.sp);

        assert_eq!(0x01, cpu.memory[0], "mem[3]: 1 != {}", cpu.memory[3]);
        assert_eq!(0x02, cpu.memory[1], "mem[2]: 2 != {}", cpu.memory[2]);
        assert_eq!(0x03, cpu.memory[2], "mem[1]: 3 != {}", cpu.memory[1]);
        assert_eq!(0x04, cpu.memory[3], "mem[0]: 4 != {}", cpu.memory[1]);
    }

    #[test]
    fn test_pop() {
        let mut cpu = CPU::new();

        // pretend we pushed something before
        cpu.memory[0] = 0x01;
        cpu.memory[1] = 0x02;
        cpu.memory[2] = 0x03;
        cpu.memory[3] = 0x04;
        cpu.sp = 0;
        cpu.flags.zero = true;
        cpu.flags.negative = true;
        cpu.program = vec![
            // POP r0
            Op::POP.bytecode(), 0x00
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 0x01020304, "reg {} != {}", cpu.registers[0], 0x01020304);
        assert_eq!(false, cpu.flags.zero, "zero flag not set");
        assert_eq!(false, cpu.flags.negative, "negative flag set");
        assert_eq!(4, cpu.sp, "sp {} != 4", cpu.sp);
    }

    #[test]
    fn test_push_pop() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 0x01020304;
        cpu.program = vec![
            // PUSH r0
            Op::PUSH.bytecode(), 0x00,
            Op::POP.bytecode(), 0x01,
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], cpu.registers[1], "{} != {}", cpu.registers[0], cpu.registers[1]);
    }
}