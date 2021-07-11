use crate::cpu::{CPU, ops};
use crate::memory::Memory;

use super::super::vmlib::MAX_REGISTER;

impl CPU {
    pub(in super::super) fn op_cmp(&mut self, memory: &mut Memory) -> ops::Result {
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
        self.flags.zero = self.registers[r0] == self.registers[r1];
        self.flags.negative = self.registers[r0] < self.registers[r1];

        if self.opts.print_op {
            println!("{:03}\tCMP  r{}, r{}", self.meta.steps, r0, r1);
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
    fn test_cmp_eq() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::Cmp.bytecode(), 0x00, 0x01,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 1;
        cpu.registers[1] = 1;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.registers[1], 1);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_cmp_lt() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::Cmp.bytecode(), 0x00, 0x01,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 1;
        cpu.registers[1] = 2;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.registers[1], 2);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, true);
    }

    #[test]
    fn test_cmp_gt() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::Cmp.bytecode(), 0x00, 0x01,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 2;
        cpu.registers[1] = 1;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 2);
        assert_eq!(cpu.registers[1], 1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }
}