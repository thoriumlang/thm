use crate::cpu::{CPU, ops};
use crate::memory::Memory;

use super::super::vmlib::MAX_REGISTER;

impl CPU {
    pub(in super::super) fn op_load(&mut self, memory: &mut Memory) -> ops::Result {
        // we map addr = 12, addr+1 = 34, addr+2 = 56, addr+3 = 78 like to:
        // r = 0x12345678

        let r0 = match self.fetch_1byte(memory) {
            None => return Err("Cannot fetch r0"),
            Some(byte) => match byte as usize {
                0..=MAX_REGISTER => byte as usize,
                _ => return Err("r0 is not a valid op register")
            },
        };

        let r1 = match self.fetch_1byte(memory) {
            None => return Err("Cannot fetch r1"),
            Some(byte) => match byte as usize {
                0..=MAX_REGISTER => byte as usize,
                _ => return Err("r1 is not a valid op register")
            },
        };

        let mut bytes: u32 = 0;
        for i in 0..4 {
            bytes = bytes << 8;
            match memory.get((self.registers[r1] + i) as u32) {
                None => return Err("Cannot fetch 4 bytes"),
                Some(byte) => bytes |= byte as u32,
            }
        }
        self.registers[r0] = bytes as i32;
        self.flags.zero = self.registers[r0] == 0;
        self.flags.negative = self.registers[r0] < 0;

        if self.opts.print_op {
            println!("{:03}\tLOAD r{}, r{}", self.meta.steps, r0, r1);
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;
    use super::super::super::vmlib::MIN_RAM_SIZE;

    #[test]
    fn test_load() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            0x01, 0x02, 0x03, 0x04,
            Op::Load.bytecode(), 0x00, 0x01
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 0x00000000;
        cpu.registers[1] = 0x00000000;
        cpu.pc = 4;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0x01020304, "{} != 0x01020304", cpu.registers[0]);
        assert_eq!(Some(0x01), memory.get(0), "mem[0]: 1 != {:?}", memory.get(0));
        assert_eq!(Some(0x02), memory.get(1), "mem[1]: 2 != {:?}", memory.get(1));
        assert_eq!(Some(0x03), memory.get(2), "mem[2]: 3 != {:?}", memory.get(2));
        assert_eq!(Some(0x04), memory.get(3), "mem[3]: 4 != {:?}", memory.get(3));
    }
}