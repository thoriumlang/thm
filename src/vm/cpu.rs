extern crate vmlib;

use vmlib::{MIN_RAM_SIZE, REG_COUNT, ROM_START};
use vmlib::op::Op;

use crate::memory::Memory;

mod ops;

pub struct Opts {
    pub print_op: bool,
}

pub struct Flags {
    zero: bool,
    negative: bool,
}

struct Meta {
    steps: u64,
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
    memory: Memory,
    running: bool,
    opts: Opts,
    meta: Meta,
}

impl CPU {
    pub fn new() -> CPU {
        Self::new_custom_memory(Memory::new(MIN_RAM_SIZE as u32, vec![]))
    }

    pub fn new_custom_memory(memory: Memory) -> CPU {
        CPU {
            registers: [0; REG_COUNT],
            pc: ROM_START as u32,
            sp: 0,
            cs: 0,
            flags: Flags {
                zero: true,
                negative: false,
            },
            memory,
            running: false,
            opts: Opts { print_op: false },
            meta: Meta {
                steps: 0,
            }
        }
    }

    pub fn set_opts(&mut self, opts: Opts) {
        self.opts = opts;
    }

    pub fn start(&mut self) {
        self.running = true;
    }

    pub fn step(&mut self) -> bool {
        if !self.running {
            return false;
        }
        match self.fetch_opcode() {
            None => { let _ = self.op_panic("Cannot fetch op"); }
            Some(bytecode) => match match Self::decode_opcode(bytecode) {
                Op::Nop => self.op_nop(),
                Op::Halt => self.op_halt(),
                Op::Panic => self.op_panic("read PANIC op"),
                Op::MovRR => self.op_mov_rr(),
                Op::MovRI => self.op_mov_ri(),
                Op::Add => self.op_add(),
                Op::Cmp => self.op_cmp(),
                Op::Inc => self.op_inc(),
                Op::Dec => self.op_dec(),
                Op::Push => self.op_push(),
                Op::Pop => self.op_pop(),
                Op::Ja => self.op_ja(),
                Op::Jreq => self.op_jreq(),
                Op::Jrne => self.op_jrne(),
                Op::Jr => self.op_jr(),
                Op::Stor => self.op_stor(),
            } {
                Ok(_) => (),
                Err(err) => self.op_panic(err).unwrap(),
            }
        }
        self.meta.steps += 1;
        self.running
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
        println!("          z={}         n={}", Self::bool_to_u8(self.flags.zero), Self::bool_to_u8(self.flags.negative));
        println!("          pc          sp           cs          ");
        println!("          {:08x}    {:08x}     {:08x}", self.pc, self.sp, self.cs)
    }

    #[inline]
    fn bool_to_u8(b: bool) -> u8 {
        match b {
            true => 1,
            false => 0,
        }
    }

    fn fetch_opcode(&mut self) -> Option<u8> {
        self.fetch_1byte()
    }

    fn decode_opcode(opcode: u8) -> Op {
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
        }
        self.pc += 4;
        return Some(result);
    }

    pub fn read_register(&self, r: u8) -> i32 {
        self.registers[r as usize]
    }

    pub fn read_memory(&self, from: u32, size: u32) -> Option<Vec<u8>> {
        let mut vec: Vec<u8> = Vec::with_capacity(size as usize);
        for i in from..(from + size) {
            match self.memory.get(i) {
                None => return None,
                Some(v) => vec.push(v)
            }
        }
        Some(vec)
    }

    #[inline]
    pub fn get_steps_count(&self) -> u64 {
        self.meta.steps
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
            Op::Push.bytecode(), 0x00,
            Op::Pop.bytecode(), 0x01,
            Op::Halt.bytecode()
        ]);
        cpu.pc = 0;
        cpu.sp = (MIN_RAM_SIZE - 1) as u32;
        cpu.start();
        while cpu.step() {}
        assert_eq!(cpu.registers[0], cpu.registers[1], "{} != {}", cpu.registers[0], cpu.registers[1]);
    }
}
