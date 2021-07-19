use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_jreq(&mut self, memory: &mut Memory) -> ops::Result {
        let target = self.fetch_word(memory)
            .ok_or("jreq: cannot fetch target")? + self.cs;

        if self.flags.zero {
            self.pc = target;
            if self.opts.print_op {
                println!("{:03}\tJREQ {:#010x}", self.meta.steps, target);
            }
        } else {
            if self.opts.print_op {
                println!("{:03}\tNOP  (JREQ {:#010x})", self.meta.steps, target);
            }
        }

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
    fn test_jreq_zero() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::Jreq.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            Op::MovRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::Halt.bytecode(),
            Op::MovRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::Halt.bytecode()
        ]);

        let mut cpu = CPU::new();
        cpu.flags.zero = true;
        cpu.cs = 0;
        cpu.pc = 0;
        cpu.start();

        while cpu.step(&mut memory) {}

        assert_eq!(cpu.registers[0], 2);
    }

    #[test]
    fn test_jreq_nonzero() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::Jreq.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            Op::MovRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::Halt.bytecode(),
            Op::MovRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::Halt.bytecode()
        ]);

        let mut cpu = CPU::new();
        cpu.flags.zero = false;
        cpu.cs = 0;
        cpu.pc = 0;
        cpu.start();

        while cpu.step(&mut memory) {}

        assert_eq!(cpu.registers[0], 1);
    }
}