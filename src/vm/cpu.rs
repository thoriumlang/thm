extern crate vmlib;

use vmlib::{RAM_SIZE, REG_COUNT, STACK_SIZE};
use vmlib::op::Op;

use crate::memory_map::MemoryMap;

mod ops;

pub struct Flags {
    zero: bool,
    negative: bool,
}

pub struct CPU {
    // FIXME remove pub
    pub registers: [i32; REG_COUNT],
    /// program pointer (aka ip)
    pc: usize,
    // FIXME move to memory!
    pub program: Vec<u8>,
    flags: Flags,
    /// stack pointer
    sp: u32,
    memory: MemoryMap,
}

impl CPU {
    pub fn new() -> CPU {
        Self::new_custom_memory(MemoryMap::new(RAM_SIZE as u32))
    }

    pub fn new_custom_memory(memory: MemoryMap) -> CPU {
        CPU {
            registers: [0; REG_COUNT],
            pc: 0,
            program: vec![],
            flags: Flags {
                zero: true,
                negative: false,
            },
            sp: STACK_SIZE as u32,
            memory,
        }
    }

    pub fn run(&mut self) {
        loop {
            // TODO: extract implementation to methods
            match Self::decode_opcode(self.fetch_opcode()) {
                Op::NOP => continue,
                Op::HALT => return,
                Op::PANIC => self.op_panic(),
                Op::LOAD => self.op_load(),
                Op::MOV => self.op_mov(),
                Op::ADD => self.op_add(),
                Op::CMP => self.op_cmp(),
                Op::JMP => self.op_jmp(),
                Op::JE => self.op_je(),
                Op::JNE => self.op_jne(),
                Op::INC => self.op_inc(),
                Op::DEC => self.op_dec(),
                Op::PUSH => self.op_push(),
                Op::POP => self.op_pop(),
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
    fn test_push_pop() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 0x01020304;
        cpu.program = vec![
            // PUSH r0
            Op::PUSH.bytecode(), 0x00,
            Op::POP.bytecode(), 0x01,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], cpu.registers[1], "{} != {}", cpu.registers[0], cpu.registers[1]);
    }
}
