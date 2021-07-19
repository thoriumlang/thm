use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_add_ri(&mut self, memory: &mut Memory) -> ops::Result {
        let r = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("add_ri: cannot fetch r")?;

        let w = self.fetch_word(memory)
            .ok_or("add_ri: cannot fetch w")? as i32;

        self.registers[r] += w;
        self.update_flags(self.registers[r]);

        if self.opts.print_op {
            println!("{:03}\tADD  r{}, {}", self.meta.steps, r, w);
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
    fn test_add_ri() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::AddRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 0;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_add_ri_zero() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::AddRW.bytecode(), 0x00, 0xff, 0xff, 0xff, 0xff,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 1;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }
}