extern crate vmlib;

use vmlib::{MIN_RAM_SIZE, REG_COUNT};
use vmlib::op::Op;

use crate::memory_map::MemoryMap;

use self::vmlib::{STACK_MAX_ADDRESS, ROM_START};

mod ops;

pub struct Flags {
    zero: bool,
    negative: bool,
}

pub struct CPU {
    // FIXME remove pub
    pub registers: [i32; REG_COUNT],
    /// program pointer (aka ip)
    // FIXME remove pub
    pub pc: u32,
    /// stack pointer
    // FIXME remove pub
    pub sp: u32,
    /// start of code segment
    pub cs: u32,
    flags: Flags,
    memory: MemoryMap,
    running: bool,
}

impl CPU {
    pub fn new() -> CPU {
        Self::new_custom_memory(MemoryMap::new(MIN_RAM_SIZE as u32, vec![]))
    }

    pub fn new_custom_memory(memory: MemoryMap) -> CPU {
        CPU {
            registers: [0; REG_COUNT],
            pc: ROM_START as u32,
            // fixme this should be set by rom
            sp: STACK_MAX_ADDRESS as u32,
            // fixme this should be set by rom
            cs: (STACK_MAX_ADDRESS + 1) as u32,
            flags: Flags {
                zero: true,
                negative: false,
            },
            memory,
            running: false,
        }
    }

    pub fn step(&mut self) {
        match self.fetch_opcode() {
            None => { let _ = self.op_panic("Cannot fetch op"); }
            Some(bytecode) => match match Self::decode_opcode(bytecode) {
                Op::NOP => self.op_nop(),
                Op::HALT => self.op_halt(),
                Op::PANIC => self.op_panic("op"),
                Op::MOV => self.op_mov(),
                Op::MOVI => self.op_movi(),
                Op::ADD => self.op_add(),
                Op::CMP => self.op_cmp(),
                Op::INC => self.op_inc(),
                Op::DEC => self.op_dec(),
                Op::PUSH => self.op_push(),
                Op::POP => self.op_pop(),
                Op::JA => self.op_ja(),
                Op::JREQ => self.op_jreq(),
                Op::JRNE => self.op_jrne(),
                Op::JR => self.op_jr(),
            } {
                Ok(_) => (),
                Err(err) => self.op_panic(err).unwrap(),
            }
        }
    }

    pub fn run(&mut self) {
        self.running = true;
        while self.running {
            self.step();
        }
    }

    pub fn dump(&self) {
        for r in 0..32 {
            if r % 4 == 0 {
                print!("r{:02} r{:02}   ", r, r + 3);
            }
            print!("{:08x}    ", self.registers[r]);
            if r % 2 == 1 {
                print!(" ")
            }
            if r % 4 == 3 {
                println!()
            }
        }
        println!("          pc          sp           cs          ");
        println!("          {:08x}    {:08x}     {:08x}", self.pc, self.sp, self.cs)
    }

    fn fetch_opcode(&mut self) -> Option<u8> {
        //println!("pc:{:#010x} = {:#04x}", self.pc, self.memory.get(self.pc).unwrap_or(0));
        self.fetch_1byte()
    }

    fn decode_opcode(opcode: u8) -> Op {
        // println!("decoding {:#04x} -> {:?}", opcode, Op::from(opcode));
        Op::from(opcode)
    }

    fn fetch_1byte(&mut self) -> Option<u8> {
        let byte = self.memory.get(self.pc);
        self.pc += 1;
        return byte;
    }

    fn fetch_4bytes(&mut self) -> Option<u32> {
        let mut result: u32 = 0;
        for i in 0..4 {
            result = result << 8;
            match self.memory.get(self.pc + i) {
                None => return None,
                Some(byte) => result |= byte as u32,
            }
            // println!("tmp {}: {:#010x}", i, result);
        }
        self.pc += 4;
        return Some(result);
    }

    fn skip_4bytes(&mut self) {
        self.pc += 4;
    }
}


#[cfg(test)]
mod tests {
    use super::*;
    use super::vmlib::ROM_START;

    #[test]
    fn test_create_cpu() {
        let cpu = CPU::new();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.pc, ROM_START as u32);
    }

    #[test]
    fn test_push_pop() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 0x01020304;
        let _ = cpu.memory.set_bytes(0, &[
            // PUSH r0
            Op::PUSH.bytecode(), 0x00,
            Op::POP.bytecode(), 0x01,
            Op::HALT.bytecode()
        ]);
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], cpu.registers[1], "{} != {}", cpu.registers[0], cpu.registers[1]);
    }
}
