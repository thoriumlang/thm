use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_inc(&mut self, memory: &mut Memory) -> ops::Result {
        let r = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("inc: cannot fetch r")?;

        self.registers[r] += 1;
        self.update_flags(self.registers[r]);

        if self.opts.print_op {
            println!("{:03}\tINC  r{}", self.meta.steps, r);
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;
    use std::sync::{Arc, RwLock};
    use crate::memory::{MemoryZone, Access};

    #[test]
    fn test_inc() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::Inc.bytecode(), 0x00,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 0;
        cpu.flags.zero = true;
        cpu.flags.negative = false;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_inc_zero() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::Inc.bytecode(), 0x00,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = -1;
        cpu.flags.zero = false;
        cpu.flags.negative = true;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0);
        assert_eq!(true, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_inc_negative() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::Inc.bytecode(), 0x00,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = -2;
        cpu.flags.zero = false;
        cpu.flags.negative = true;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], -1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(true, cpu.flags.negative);
    }
}