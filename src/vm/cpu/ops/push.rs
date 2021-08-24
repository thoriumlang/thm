use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_push(&mut self, memory: &Memory) -> ops::Result {
        // we map r = 0x12345678 like to:
        // sp = 78, sp-1 = 56, sp-2 = 34 sp-3 = 12

        let r = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("push: cannot fetch r")?;

        if self.opts.print_op {
            println!("{:03}\tPUSH r{}", self.meta.steps, r);
        }

        self.sp -= 4;
        if !Self::store_word(memory, self.registers[r] as u32, self.sp) {
            return Err("push: cannot write memory");
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
    fn test_push() {
        let mut memory = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW))]).unwrap();
        let _ = memory.set_bytes(5, &[
            Op::Push.bytecode(), 0x00,
        ]);
        let pic = Arc::new(PIC::new());

        let mut cpu = CPU::new(pic);
        cpu.sp = 4;
        cpu.registers[0] = 0x01020304;
        cpu.flags.zero = true;
        cpu.flags.negative = true;
        cpu.pc = 5;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0x01020304, "r0 {} != 0x01020304", cpu.registers[0]);
        assert_eq!(true, cpu.flags.zero, "zero flag not set");
        assert_eq!(true, cpu.flags.negative, "negative flag not set");
        assert_eq!(0, cpu.sp, "sp {} != 0", cpu.sp);

        assert_eq!(Some(0x01), memory.get(0), "mem[0]: 1 != {:?}", memory.get(0));
        assert_eq!(Some(0x02), memory.get(1), "mem[1]: 2 != {:?}", memory.get(1));
        assert_eq!(Some(0x03), memory.get(2), "mem[2]: 3 != {:?}", memory.get(2));
        assert_eq!(Some(0x04), memory.get(3), "mem[3]: 4 != {:?}", memory.get(3));
    }
}