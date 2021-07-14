use std::convert::TryInto;

use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_load(&mut self, memory: &mut Memory) -> ops::Result {
        // we map addr = 12, addr+1 = 34, addr+2 = 56, addr+3 = 78 like to:
        // r = 0x12345678

        let r0 = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("load: cannot fetch r0")?;

        let r1 = self.fetch_register(memory, &Self::is_general_purpose_register)
            .ok_or("load: cannot fetch r1")?;

        let address = self.registers[r1] as u32;
        let bytes = memory.get_bytes(address, 4)
            .ok_or("load: cannot get memory")?
            .as_slice().try_into().expect("load: did not read 4 bytes");

        self.registers[r0] = i32::from_be_bytes(bytes);
        self.flags.zero = self.registers[r0] == 0;
        self.flags.negative = self.registers[r0] < 0;

        if self.opts.print_op {
            println!("{:03}\tLOAD r{}, r{}", self.meta.steps, r0, r1);
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;
    use super::super::super::vmlib::MIN_RAM_SIZE;

    #[test]
    fn test_load() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            0x01, 0x02, 0x03, 0x04,
            Op::Load.bytecode(), 0x00, 0x01
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 0x00000000;
        cpu.registers[1] = 0x00000000;
        cpu.pc = 4;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0x01020304, "{} != 0x01020304", cpu.registers[0]);
        assert_eq!(Some(0x01), memory.get(0), "mem[0]: 1 != {:?}", memory.get(0));
        assert_eq!(Some(0x02), memory.get(1), "mem[1]: 2 != {:?}", memory.get(1));
        assert_eq!(Some(0x03), memory.get(2), "mem[2]: 3 != {:?}", memory.get(2));
        assert_eq!(Some(0x04), memory.get(3), "mem[3]: 4 != {:?}", memory.get(3));
    }
}