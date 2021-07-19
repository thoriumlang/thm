use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_ret(&mut self, memory: &mut Memory) -> ops::Result {
        if self.opts.print_op {
            println!("{:03}\tRET", self.meta.steps);
        }

        let address = Self::load_word(memory, self.sp).ok_or("load: cannot read memory")? as u32;

        self.pc = address;
        self.sp += 4;

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
    fn test_ret() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(4, &[
            Op::Call.bytecode(), 0x00, 0x00, 0x00, 0x06,
            Op::Halt.bytecode(),
            Op::MovRW.bytecode(), 0x00, 0x00, 0x00, 0x00, 0xff,
            Op::Ret.bytecode(),
            Op::Panic.bytecode(),
        ]);

        let mut cpu = CPU::new();

        cpu.sp = 4;
        cpu.cs = cpu.sp;
        cpu.pc = cpu.cs;
        cpu.start();

        while cpu.step(&mut memory) {}

        assert_eq!(cpu.registers[0], 0x000000ff, "r0 {} != 0x000000ff", cpu.registers[0]);
        assert_eq!(4, cpu.sp, "sp {} != 4", cpu.sp);
    }
}