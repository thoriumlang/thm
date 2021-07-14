extern crate vmlib;

use std::time::Instant;

use vmlib::{REG_COUNT, ROM_START};
use vmlib::op::Op;

use crate::memory::Memory;

use self::vmlib::MAX_REGISTER;
use std::convert::TryInto;

mod ops;

pub struct Opts {
    pub print_op: bool,
}

pub struct Flags {
    running: bool,
    zero: bool,
    negative: bool,
}

struct Meta {
    steps: u64,
    bench: [Instant; 256],
}

pub struct CPU {
    registers: [i32; REG_COUNT],
    /// program pointer (aka ip)
    pc: u32,
    /// stack pointer
    sp: u32,
    /// start of code segment
    cs: u32,
    flags: Flags,
    opts: Opts,
    meta: Meta,
}

impl CPU {
    pub fn new() -> CPU {
        let now = Instant::now();
        CPU {
            registers: [0; REG_COUNT],
            pc: ROM_START as u32,
            sp: 0,
            cs: ROM_START as u32,
            flags: Flags {
                running: false,
                zero: true,
                negative: false,
            },
            opts: Opts { print_op: false },
            meta: Meta {
                steps: 0,
                bench: [now; 256],
            },
        }
    }

    pub fn set_opts(&mut self, opts: Opts) {
        self.opts = opts;
    }

    pub fn start(&mut self) {
        self.flags.running = true;
    }

    pub fn step(&mut self, memory: &mut Memory) -> bool {
        if !self.flags.running {
            return false;
        }
        match self.fetch_opcode(memory) {
            None => { let _ = self.op_panic("Cannot fetch op"); }
            Some(bytecode) => match match Self::decode_opcode(bytecode) {
                Op::Nop => self.op_nop(),
                Op::Halt => self.op_halt(),
                Op::Panic => self.op_panic("read PANIC op"),
                Op::MovRR => self.op_mov_rr(memory),
                Op::MovRI => self.op_mov_ri(memory),
                Op::AddRR => self.op_add_rr(memory),
                Op::AddRI => self.op_add_ri(memory),
                Op::SubRR => self.op_sub_rr(memory),
                Op::SubRI => self.op_sub_ri(memory),
                Op::MulRR => self.op_mul_rr(memory),
                Op::MulRI => self.op_mul_ri(memory),
                Op::Cmp => self.op_cmp(memory),
                Op::Inc => self.op_inc(memory),
                Op::Dec => self.op_dec(memory),
                Op::Push => self.op_push(memory),
                Op::Pop => self.op_pop(memory),
                Op::Ja => self.op_ja(memory),
                Op::Jreq => self.op_jreq(memory),
                Op::Jrne => self.op_jrne(memory),
                Op::Jr => self.op_jr(memory),
                Op::Stor => self.op_stor(memory),
                Op::Load => self.op_load(memory),
                Op::Call => self.op_call(memory),
                Op::Ret => self.op_ret(memory),
                Op::Xbm => self.op_xbm(memory),
            } {
                Ok(_) => (),
                Err(err) => self.op_panic(err).unwrap(),
            }
        }
        self.meta.steps += 1;
        self.flags.running
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

    fn fetch_opcode(&mut self, memory: &Memory) -> Option<u8> {
        self.fetch_byte(memory)
    }

    fn decode_opcode(opcode: u8) -> Op {
        Op::from(opcode)
    }

    fn fetch_byte(&mut self, memory: &Memory) -> Option<u8> {
        let byte = memory.get(self.pc);
        self.pc += 1;
        return byte;
    }

    fn fetch_register(&mut self, memory: &Memory, f: &dyn Fn(&usize) -> bool) -> Option<usize> {
        self.fetch_byte(memory)
            .map(|r| r as usize)
            .filter(f)
    }

    fn is_general_purpose_register(r: &usize) -> bool {
        (0..(MAX_REGISTER + 1)).contains(r)
    }

    fn fetch_word(&mut self, memory: &Memory) -> Option<u32> {
        match memory.get_bytes(self.pc, 4).and_then(|v| v.as_slice().try_into().ok()) {
            Some(bytes) => {
                self.pc += 4;
                Some(u32::from_be_bytes(bytes))
            },
            _ => None
        }
    }

    pub fn read_register(&self, r: usize) -> i32 {
        match r {
            255 => self.pc as i32,
            254 => self.sp as i32,
            253 => self.cs as i32,
            r => self.registers[r],
        }
    }

    pub fn write_register(&mut self, r: usize, val: i32) {
        match r {
            255 => self.pc = val as u32,
            254 => self.sp = val as u32,
            253 => self.cs = val as u32,
            r => self.registers[r] = val,
        }
    }

    #[inline]
    pub fn get_steps_count(&self) -> u64 {
        self.meta.steps
    }
}


#[cfg(test)]
mod tests {
    use super::*;
    use super::vmlib::{MIN_RAM_SIZE, ROM_START};

    #[test]
    fn test_create_cpu() {
        let cpu = CPU::new();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.pc, ROM_START as u32);
    }

    #[test]
    fn test_push_pop() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::Push.bytecode(), 0x00,
            Op::Pop.bytecode(), 0x01,
            Op::Halt.bytecode()
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 0x01020304;
        cpu.pc = 0;
        cpu.sp = (MIN_RAM_SIZE - 1) as u32;
        cpu.start();

        while cpu.step(&mut memory) {}

        assert_eq!(cpu.registers[0], cpu.registers[1], "{} != {}", cpu.registers[0], cpu.registers[1]);
    }
}
