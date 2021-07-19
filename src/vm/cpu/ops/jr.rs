use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_jr(&mut self, memory: &mut Memory) -> ops::Result {
        let target = self.fetch_word(memory)
            .ok_or("jr: cannot fetch target")? + self.cs;

        if self.opts.print_op {
            println!("{:03}\tJR   {:#010x}", self.meta.steps, target);
        }

        self.pc = target;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use std::sync::{Arc, RwLock};

    use crate::cpu::Op;
    use crate::memory::{Access, MemoryZone};

    use super::*;

    #[test]
    fn test_jr() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::Jr.bytecode(), 0x00, 0x00, 0x00, 0x0c,
            Op::MovRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::Halt.bytecode(),
            Op::MovRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::Halt.bytecode()
        ]);

        let mut cpu = CPU::new();
        cpu.cs = 0;
        cpu.pc = 0;
        cpu.start();

        while cpu.step(&mut memory) {}

        assert_eq!(cpu.registers[0], 2);
    }
}