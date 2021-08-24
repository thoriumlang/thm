use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_mul_ri(&mut self, memory: &Memory) -> ops::Result {
        let r = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("mul_ri: cannot fetch r1")?;

        let w = self.fetch_word(memory)
            .ok_or("mov_ri: cannot fetch w")? as i32;

        self.registers[r] *= w;
        self.update_flags(self.registers[r]);

        if self.opts.print_op {
            println!("{:03}\tSUB  r{}, {}", self.meta.steps, r, w);
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
    fn test_mul_ri() {
        let mut memory = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::MulRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
        ]);
        let pic = Arc::new(PIC::new());

        let mut cpu = CPU::new(pic);
        cpu.registers[0] = 3;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 6);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_mul_ri_zero() {
        let mut memory = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::MulRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x00,
        ]);
        let pic = Arc::new(PIC::new());

        let mut cpu = CPU::new(pic);
        cpu.registers[0] = 1;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_mul_ri_negative() {
        let mut memory = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW))]).unwrap();
        let _ = memory.set_bytes(0, &[
            Op::MulRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
        ]);
        let pic = Arc::new(PIC::new());

        let mut cpu = CPU::new(pic);
        cpu.registers[0] = -1;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], -1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, true);
    }
}