use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_pop(&mut self, memory: &mut Memory) -> ops::Result {
        // we map sp = 78, sp-1 = 56, sp-2 = 34 sp-3 = 12 like to:
        // r = 0x12345678

        let r = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("pop: cannot fetch r")?;

        let value = Self::load_word(memory, self.sp).ok_or("pop: cannot read memory")? as i32;

        self.registers[r] = value;
        self.sp += 4;
        self.update_flags(self.registers[r]);

        if self.opts.print_op {
            println!("{:03}\tPOP  r{}", self.meta.steps, r);
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
    fn test_pop() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        // pretend we pushed something before
        let _ = memory.set(0, 0x01);
        let _ = memory.set(1, 0x02);
        let _ = memory.set(2, 0x03);
        let _ = memory.set(3, 0x04);
        let _ = memory.set_bytes(4, &[
            Op::Pop.bytecode(), 0x00,
        ]);

        let mut cpu = CPU::new();
        cpu.sp = 0;
        cpu.flags.zero = true;
        cpu.flags.negative = true;
        cpu.pc = 4;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0x01020304, "reg {} != {}", cpu.registers[0], 0x01020304);
        assert_eq!(false, cpu.flags.zero, "zero flag not set");
        assert_eq!(false, cpu.flags.negative, "negative flag set");
        assert_eq!(4, cpu.sp, "sp {} != 4", cpu.sp);
    }
}