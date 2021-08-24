use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_sub_rr(&mut self, memory: &Memory) -> ops::Result {
        let r0 = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("sub_rr: cannot fetch r0")?;

        let r1 = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("sub_rr: cannot fetch r1")?;

        self.registers[r0] -= self.registers[r1];
        self.update_flags(self.registers[r0]);

        if self.opts.print_op {
            println!("{:03}\tSUB  r{}, r{}", self.meta.steps, r0, r1);
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use std::sync::Arc;

    use crate::cpu::Op;
    use crate::memory::{Access, MemoryZone};

    use super::*;
    use crate::interrupts::PIC;

    #[test]
    fn test_sub_rr() {
        let mut memory = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::SubRR.bytecode(), 0x00, 0x01,
        ]);
        let pic = Arc::new(PIC::new());

        let mut cpu = CPU::new(pic);
        cpu.registers[0] = 2;
        cpu.registers[1] = 1;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.registers[1], 1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_sub_rr_zero() {
        let mut memory = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::SubRR.bytecode(), 0x00, 0x01,
        ]);
        let pic = Arc::new(PIC::new());

        let mut cpu = CPU::new(pic);
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

    #[test]
    fn test_sub_rr_negative() {
        let mut memory = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::SubRR.bytecode(), 0x00, 0x01,
        ]);
        let pic = Arc::new(PIC::new());

        let mut cpu = CPU::new(pic);
        cpu.registers[0] = 0;
        cpu.registers[1] = 1;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], -1);
        assert_eq!(cpu.registers[1], 1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, true);
    }
}