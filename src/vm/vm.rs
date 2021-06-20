extern crate vmlib;
use vmlib::opcodes::Opcode;

pub struct Flags {
    zero: bool,
    negative: bool,
}

pub struct VM {
    pub registers: [i32; 32], // FIXME remove pub
    pc: usize,
    pub program: Vec<u8>, // FIXME remove pub
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
                Opcode::NOP => { continue; }
                Opcode::HALT => {
                    return;
                }
                Opcode::PANIC => {
                    println!("Panic!");
                    return;
                }
                Opcode::LOAD => {
                    let r = self.fetch_1byte() as usize;
                    let immediate = self.fetch_4bytes() as i32;
                    self.registers[r] = immediate;
                    self.flags.zero = self.registers[r] == 0;
                    self.flags.negative = self.registers[r] < 0;
                }
                Opcode::MOV => {
                    let r0 = self.fetch_1byte() as usize;
                    let r1 = self.fetch_1byte() as usize;
                    self.registers[r0] = self.registers[r1];
                    self.flags.zero = self.registers[r0] == 0;
                    self.flags.negative = self.registers[r0] < 0;
                }
                Opcode::ADD => {
                    let r0 = self.fetch_1byte() as usize;
                    let r1 = self.fetch_1byte() as usize;
                    self.registers[r0] += self.registers[r1];
                    self.flags.zero = self.registers[r0] == 0;
                    self.flags.negative = self.registers[r0] < 0;
                }
                Opcode::CMP => {
                    let r0 = self.fetch_1byte() as usize;
                    let r1 = self.fetch_1byte() as usize;
                    self.flags.zero = self.registers[r0] == self.registers[r1];
                    self.flags.negative = self.registers[r0] < self.registers[r1];
                }
                Opcode::JMP => {
                    self.pc = self.fetch_4bytes() as usize;
                }
                Opcode::JE => {
                    if self.flags.zero {
                        self.pc = self.fetch_4bytes() as usize;
                    } else {
                        self.skip_4bytes();
                    }
                }
            }
        }
    }

    fn fetch_opcode(&mut self) -> u8 {
        if self.pc >= self.program.len() {
            return Opcode::PANIC.bytecode();
        }

        let opcode = self.program[self.pc];
        self.pc += 1;

        return opcode;
    }

    fn decode_opcode(opcode: u8) -> Opcode {
        Opcode::from(opcode)
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
            Opcode::LOAD.bytecode(), 0x00, 0x01, 0x02, 0x04, 0x08,
            Opcode::HALT.bytecode()
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
            Opcode::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x00,
            Opcode::HALT.bytecode()
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
            Opcode::MOV.bytecode(), 0x00, 0x01,
            Opcode::HALT.bytecode()
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
            Opcode::MOV.bytecode(), 0x00, 0x01,
            Opcode::HALT.bytecode()
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
            Opcode::ADD.bytecode(), 0x00, 0x01,
            Opcode::HALT.bytecode()
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
            Opcode::ADD.bytecode(), 0x00, 0x01,
            Opcode::HALT.bytecode()
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
            Opcode::CMP.bytecode(), 0x00, 0x01,
            Opcode::HALT.bytecode()
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
            Opcode::CMP.bytecode(), 0x00, 0x01,
            Opcode::HALT.bytecode()
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
            Opcode::CMP.bytecode(), 0x00, 0x01,
            Opcode::HALT.bytecode()
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
            Opcode::JMP.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Opcode::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Opcode::HALT.bytecode(),
            // LOAD 0, #2
            Opcode::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Opcode::HALT.bytecode()
        ];
        vm.run();
        assert_eq!(vm.registers[0], 2);
    }

    #[test]
    fn test_je_zero() {
        let mut vm = VM::new();

        vm.program = vec![
            // JE #12 (i.e. LOAD 0, #2)
            Opcode::JE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Opcode::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Opcode::HALT.bytecode(),
            // LOAD 0, #2
            Opcode::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Opcode::HALT.bytecode()
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
            Opcode::JE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Opcode::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Opcode::HALT.bytecode(),
            // LOAD 0, #2
            Opcode::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Opcode::HALT.bytecode()
        ];
        vm.flags.zero = false;
        vm.run();
        assert_eq!(vm.registers[0], 1);
    }

    #[test]
    fn test_fibonacci() {
        let mut vm = VM::new();

        // fibonacci(5)
        vm.program = vec![
            /*0*/  Opcode::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x05, // r0 = 5
            /*6*/  Opcode::LOAD.bytecode(), 0x01, 0x00, 0x00, 0x00, 0x00, // r1 = 0
            /*12*/ Opcode::LOAD.bytecode(), 0x02, 0x00, 0x00, 0x00, 0x01, // r2 = 1

            /*18*/ Opcode::LOAD.bytecode(), 0x03, 0x00, 0x00, 0x00, 0x00, // r3 = 0
            /*24*/ Opcode::LOAD.bytecode(), 0x04, 0x00, 0x00, 0x00, 0x01, // r4 = 1

            /*30*/ Opcode::CMP.bytecode(), 0x00, 0x01,                    // r0 == r1?
            /*33*/ Opcode::JE.bytecode(), 0x00, 0x00, 0x00, 0x37,         // if r0 == r1 -> jump 55

            /*38*/ Opcode::MOV.bytecode(), 0x05, 0x03,                    // r5 = r3
            /*41*/ Opcode::MOV.bytecode(), 0x03, 0x04,                    // r3 = r4
            /*44*/ Opcode::ADD.bytecode(), 0x04, 0x05,                    // r4 += r5
            /*47*/ Opcode::ADD.bytecode(), 0x01, 0x02,                    // r1 += r2
            /*50*/ Opcode::JMP.bytecode(), 0x00, 0x00, 0x00, 0x1E,        // jump 30

            /*55*/ Opcode::HALT.bytecode()
        ];
        vm.run();

        assert_eq!(vm.registers[3], 5);
    }
}