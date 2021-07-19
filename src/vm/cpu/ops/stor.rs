use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_stor(&mut self, memory: &mut Memory) -> ops::Result {
        // we map r = 0x12345678 like to:
        // addr = 12, addr+1 = 34, addr+2 = 56, addr+3 = 78

        let r0 = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("stor: cannot fetch r0")?;

        let r1 = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("stor: cannot fetch r1")?;

        if self.opts.print_op {
            println!("{:03}\tSTOR r{}, r{}", self.meta.steps, r0, r1);
        }

        if !Self::store_word(memory, self.registers[r1] as u32, self.registers[r0] as u32) {
            return Err("stor: cannot write memory");
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
    fn test_stor() {
        let mut memory = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = memory.set_bytes(5, &[
            Op::Stor.bytecode(), 0x01, 0x00
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 0x01020304;
        cpu.registers[1] = 0x00000000;
        cpu.pc = 5;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0x01020304, "{} != 0x01020304", cpu.registers[0]);

        assert_eq!(Some(0x01), memory.get(0), "mem[0]: 1 != {:?}", memory.get(0));
        assert_eq!(Some(0x02), memory.get(1), "mem[1]: 2 != {:?}", memory.get(1));
        assert_eq!(Some(0x03), memory.get(2), "mem[2]: 3 != {:?}", memory.get(2));
        assert_eq!(Some(0x04), memory.get(3), "mem[3]: 4 != {:?}", memory.get(3));
    }
}