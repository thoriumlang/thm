use crate::cpu::{CPU, ops};

use super::super::vmlib::MAX_REGISTER;
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_push(&mut self, memory:&mut Memory) -> ops::Result {
        // we map r = 0x12345678 like to:
        // sp = 78, sp-1 = 56, sp-2 = 34 sp-3 = 12

        let r = match self.fetch_1byte(memory) {
            None => return Err("Cannot fetch r"),
            Some(byte) => match byte as usize {
                0..=MAX_REGISTER => byte as usize,
                _ => return Err("r is not a valid op register")
            },
        };

        if self.opts.print_op {
            println!("{:03}\tPUSH r{}", self.meta.steps, r);
        }

        for byte in self.registers[r].to_le_bytes().iter() {
            self.sp -= 1;
            if !memory.set(self.sp, *byte) {
                return Err("Cannot set");
            }
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
    fn test_push() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(5, &[
            Op::Push.bytecode(), 0x00,
        ]);

        let mut cpu = CPU::new();
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