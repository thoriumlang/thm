use crate::cpu::{CPU, ops};
use super::super::vmlib::MAX_REGISTER;
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_inc(&mut self, memory: &mut Memory) -> ops::Result {
        let r = match self.fetch_1byte(memory) {
            None => return Err("Cannot fetch r"),
            Some(byte) => match byte as usize {
                0..=MAX_REGISTER => byte as usize,
                _ => return Err("r is not a valid op register")
            },
        } as usize;
        self.registers[r] += 1;
        self.flags.zero = self.registers[r] == 0;
        self.flags.negative = self.registers[r] < 0;

        if self.opts.print_op {
            println!("{:03}\tINC  r{}", self.meta.steps, r);
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
    fn test_inc() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::Inc.bytecode(), 0x00,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 0;
        cpu.flags.zero = true;
        cpu.flags.negative = false;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_inc_zero() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::Inc.bytecode(), 0x00,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = -1;
        cpu.flags.zero = false;
        cpu.flags.negative = true;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0);
        assert_eq!(true, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_inc_negative() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::Inc.bytecode(), 0x00,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = -2;
        cpu.flags.zero = false;
        cpu.flags.negative = true;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], -1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(true, cpu.flags.negative);
    }
}