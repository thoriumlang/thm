extern crate vmlib;

use vmlib::op::Op;

pub struct Flags {
    zero: bool,
    negative: bool,
}

pub struct VM {
    // FIXME remove pub
    pub registers: [i32; 32],
    pc: usize,
    // FIXME remove pub
    pub program: Vec<u8>,
    flags: Flags,
}

impl VM {
    pub fn new() -> VM {
        VM {
            registers: [0; 32],
            pc: 0,
            program: vec![],
            flags: Flags {
                zero: true,
                negative: false,
            },
        }
    }

    pub fn run(&mut self) {
        loop {
            // println!("regs: {:?}", self.registers);
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
    fn test_create_vm() {
        let vm = VM::new();
        assert_eq!(vm.registers[0], 0);
        assert_eq!(vm.flags.zero, true);
        assert_eq!(vm.pc, 0);
    }

    #[test]
    fn test_load() {
        let mut vm = VM::new();
        vm.program = vec![
            // LOAD 0, #16909320
            Op::LOAD.bytecode(), 0x00, 0x01, 0x02, 0x04, 0x08,
            Op::HALT.bytecode()
        ];
        vm.run();
        assert_eq!(vm.registers[0], 16909320);
        assert_eq!(vm.flags.zero, false);
        assert_eq!(vm.flags.negative, false);
    }

    #[test]
    fn test_load_zero() {
        let mut vm = VM::new();
        vm.program = vec![
            // LOAD 0, #0
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x00,
            Op::HALT.bytecode()
        ];
        vm.run();
        assert_eq!(vm.registers[0], 0);
        assert_eq!(vm.flags.zero, true);
        assert_eq!(vm.flags.negative, false);
    }

    #[test]
    fn test_mov() {
        let mut vm = VM::new();
        vm.registers[1] = 1;
        vm.program = vec![
            // LOAD 0, #1
            Op::MOV.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        vm.run();
        assert_eq!(vm.registers[0], 1);
        assert_eq!(vm.registers[1], 1);
        assert_eq!(vm.flags.zero, false);
        assert_eq!(vm.flags.negative, false);
    }

    #[test]
    fn test_mov_zero() {
        let mut vm = VM::new();
        vm.registers[0] = 1;
        vm.program = vec![
            // LOAD 0, #1
            Op::MOV.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        vm.run();
        assert_eq!(vm.registers[0], 0);
        assert_eq!(vm.registers[1], 0);
        assert_eq!(vm.flags.zero, true);
        assert_eq!(vm.flags.negative, false);
    }

    #[test]
    fn test_add() {
        let mut vm = VM::new();

        vm.registers[0] = 1;
        vm.registers[1] = 2;
        vm.program = vec![
            Op::ADD.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        vm.run();
        assert_eq!(vm.registers[0], 3);
        assert_eq!(vm.registers[1], 2);
        assert_eq!(vm.flags.zero, false);
        assert_eq!(vm.flags.negative, false);
    }

    #[test]
    fn test_add_zero() {
        let mut vm = VM::new();

        vm.registers[0] = 0;
        vm.registers[1] = 0;
        vm.program = vec![
            Op::ADD.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        vm.run();
        assert_eq!(vm.registers[0], 0);
        assert_eq!(vm.registers[1], 0);
        assert_eq!(vm.flags.zero, true);
        assert_eq!(vm.flags.negative, false);
    }

    #[test]
    fn test_cmp_eq() {
        let mut vm = VM::new();

        vm.registers[0] = 1;
        vm.registers[1] = 1;
        vm.program = vec![
            Op::CMP.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        vm.run();
        assert_eq!(vm.registers[0], 1);
        assert_eq!(vm.registers[1], 1);
        assert_eq!(vm.flags.zero, true);
        assert_eq!(vm.flags.negative, false);
    }

    #[test]
    fn test_cmp_lt() {
        let mut vm = VM::new();

        vm.registers[0] = 1;
        vm.registers[1] = 2;
        vm.program = vec![
            Op::CMP.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        vm.run();
        assert_eq!(vm.registers[0], 1);
        assert_eq!(vm.registers[1], 2);
        assert_eq!(vm.flags.zero, false);
        assert_eq!(vm.flags.negative, true);
    }

    #[test]
    fn test_cmp_gt() {
        let mut vm = VM::new();

        vm.registers[0] = 2;
        vm.registers[1] = 1;
        vm.program = vec![
            Op::CMP.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        vm.run();
        assert_eq!(vm.registers[0], 2);
        assert_eq!(vm.registers[1], 1);
        assert_eq!(vm.flags.zero, false);
        assert_eq!(vm.flags.negative, false);
    }

    #[test]
    fn test_jmp() {
        let mut vm = VM::new();

        vm.program = vec![
            // JMP #12 (i.e. LOAD 0, #2)
            Op::JMP.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        vm.run();
        assert_eq!(vm.registers[0], 2);
    }

    #[test]
    fn test_je_zero() {
        let mut vm = VM::new();

        vm.program = vec![
            // JE #12 (i.e. LOAD 0, #2)
            Op::JE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        vm.flags.zero = true;
        vm.run();
        assert_eq!(vm.registers[0], 2);
    }

    #[test]
    fn test_je_nonzero() {
        let mut vm = VM::new();

        vm.program = vec![
            // JE #12 (i.e. LOAD 0, #2)
            Op::JE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        vm.flags.zero = false;
        vm.run();
        assert_eq!(vm.registers[0], 1);
    }

    #[test]
    fn test_jne_zero() {
        let mut vm = VM::new();

        vm.program = vec![
            // JNE #12 (i.e. LOAD 0, #2)
            Op::JNE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        vm.flags.zero = false;
        vm.run();
        assert_eq!(vm.registers[0], 2);
    }

    #[test]
    fn test_jne_nonzero() {
        let mut vm = VM::new();

        vm.program = vec![
            // JNE #12 (i.e. LOAD 0, #2)
            Op::JNE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        vm.flags.zero = true;
        vm.run();
        assert_eq!(vm.registers[0], 1);
    }

    #[test]
    fn test_inc() {
        let mut vm = VM::new();

        vm.program = vec![
            // INC r0
            Op::INC.bytecode(), 0x00
        ];
        vm.registers[0] = 0;
        vm.flags.zero = true;
        vm.flags.negative = false;
        vm.run();
        assert_eq!(vm.registers[0], 1);
        assert_eq!(false, vm.flags.zero);
        assert_eq!(false, vm.flags.negative);
    }

    #[test]
    fn test_inc_zero() {
        let mut vm = VM::new();

        vm.program = vec![
            // INC r0
            Op::INC.bytecode(), 0x00
        ];
        vm.registers[0] = -1;
        vm.flags.zero = false;
        vm.flags.negative = true;
        vm.run();
        assert_eq!(vm.registers[0], 0);
        assert_eq!(true, vm.flags.zero);
        assert_eq!(false, vm.flags.negative);
    }

    #[test]
    fn test_inc_negative() {
        let mut vm = VM::new();

        vm.program = vec![
            // INC r0
            Op::INC.bytecode(), 0x00
        ];
        vm.registers[0] = -2;
        vm.flags.zero = false;
        vm.flags.negative = true;
        vm.run();
        assert_eq!(vm.registers[0], -1);
        assert_eq!(false, vm.flags.zero);
        assert_eq!(true, vm.flags.negative);
    }

    #[test]
    fn test_dec() {
        let mut vm = VM::new();

        vm.program = vec![
            // DEC r0
            Op::DEC.bytecode(), 0x00
        ];
        vm.registers[0] = 2;
        vm.flags.zero = false;
        vm.flags.negative = false;
        vm.run();
        assert_eq!(vm.registers[0], 1);
        assert_eq!(false, vm.flags.zero);
        assert_eq!(false, vm.flags.negative);
    }

    #[test]
    fn test_dec_zero() {
        let mut vm = VM::new();

        vm.program = vec![
            // DEC r0
            Op::DEC.bytecode(), 0x00
        ];
        vm.registers[0] = 1;
        vm.flags.zero = false;
        vm.flags.negative = false;
        vm.run();
        assert_eq!(vm.registers[0], 0);
        assert_eq!(true, vm.flags.zero);
        assert_eq!(false, vm.flags.negative);
    }

    #[test]
    fn test_dec_negative() {
        let mut vm = VM::new();

        vm.program = vec![
            // DEC r0
            Op::DEC.bytecode(), 0x00
        ];
        vm.registers[0] = 0;
        vm.flags.zero = true;
        vm.flags.negative = false;
        vm.run();
        assert_eq!(vm.registers[0], -1);
        assert_eq!(false, vm.flags.zero);
        assert_eq!(true, vm.flags.negative);
    }

    #[test]
    fn test_fibonacci() {
        let mut vm = VM::new();

        // fibonacci(5)
        vm.program = vec![
            /*0*/  Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x05, // r0 = 5
            /*6*/  Op::LOAD.bytecode(), 0x01, 0x00, 0x00, 0x00, 0x00, // r1 = 0
            /*12*/ Op::LOAD.bytecode(), 0x02, 0x00, 0x00, 0x00, 0x01, // r2 = 1

            /*18*/ Op::LOAD.bytecode(), 0x03, 0x00, 0x00, 0x00, 0x00, // r3 = 0
            /*24*/ Op::LOAD.bytecode(), 0x04, 0x00, 0x00, 0x00, 0x01, // r4 = 1

            /*30*/ Op::CMP.bytecode(), 0x00, 0x01,                    // r0 == r1?
            /*33*/ Op::JE.bytecode(), 0x00, 0x00, 0x00, 0x37,         // if r0 == r1 -> jump 55

            /*38*/ Op::MOV.bytecode(), 0x05, 0x03,                    // r5 = r3
            /*41*/ Op::MOV.bytecode(), 0x03, 0x04,                    // r3 = r4
            /*44*/ Op::ADD.bytecode(), 0x04, 0x05,                    // r4 += r5
            /*47*/ Op::ADD.bytecode(), 0x01, 0x02,                    // r1 += r2
            /*50*/ Op::JMP.bytecode(), 0x00, 0x00, 0x00, 0x1E,        // jump 30

            /*55*/ Op::HALT.bytecode()
        ];
        vm.run();

        assert_eq!(vm.registers[3], 5);
    }
}