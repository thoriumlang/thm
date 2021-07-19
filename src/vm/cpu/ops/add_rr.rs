use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_add_rr(&mut self, memory: &mut Memory) -> ops::Result {
        let r0 = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("add_rr: cannot fetch r0")?;

        let r1 = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("add_rr: cannot fetch r1")?;

        self.registers[r0] += self.registers[r1];
        self.update_flags(self.registers[r0]);

        if self.opts.print_op {
            println!("{:03}\tADD  r{}, r{}", self.meta.steps, r0, r1);
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
    fn test_add_rr() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::AddRR.bytecode(), 0x00, 0x01,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 1;
        cpu.registers[1] = 2;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 3);
        assert_eq!(cpu.registers[1], 2);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_add_rr_zero() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::AddRR.bytecode(), 0x00, 0x01,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 0;
        cpu.registers[1] = 0;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.registers[1], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }
}