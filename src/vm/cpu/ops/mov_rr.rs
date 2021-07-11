use crate::cpu::{CPU, ops};
use crate::memory::Memory;

use super::super::vmlib::{MAX_REGISTER, REG_CS, REG_SP};

impl CPU {
    pub(in super::super) fn op_mov_rr(&mut self, memory: &mut Memory) -> ops::Result {
        let r0 = match self.fetch_1byte(memory) {
            None => return Err("Cannot fetch r0"),
            Some(byte) => byte,
        } as usize;
        let r1 = match self.fetch_1byte(memory) {
            None => return Err("Cannot fetch r1"),
            Some(byte) => byte,
        } as usize;

        let value = match r1 {
            REG_SP => self.sp as i32,
            REG_CS => self.cs as i32,
            0..=MAX_REGISTER => self.registers[r1],
            _ => return Err("r1 is not a valid source register"),
        } as i32;

        match r0 {
            REG_SP => self.sp = value as u32,
            REG_CS => self.cs = value as u32,
            0..=MAX_REGISTER => self.registers[r0] = value,
            _ => {
                println!("{}", r0);
                return Err("r0 is not a valid destination register");
            }
        }

        self.flags.zero = value == 0;
        self.flags.negative = value < 0;

        if self.opts.print_op {
            let r0 = match r0 {
                REG_SP => "sp".to_string(),
                REG_CS => "cs".to_string(),
                i => format!("r{}", i)
            };
            let r1 = match r1 {
                REG_SP => "sp".to_string(),
                REG_CS => "cs".to_string(),
                i => format!("r{}", i)
            };
            println!("{:03}\tMOV  {}, {}", self.meta.steps, r0, r1);
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
    fn test_mov_rr() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::MovRR.bytecode(), 0x00, 0x01,
        ]);

        let mut cpu = CPU::new();
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
    fn test_mov_zero() {
        let mut memory = Memory::new(MIN_RAM_SIZE as u32, vec![]);
        let _ = memory.set_bytes(0, &[
            Op::MovRR.bytecode(), 0x00, 0x01,
        ]);

        let mut cpu = CPU::new();
        cpu.registers[0] = 1;
        cpu.pc = 0;
        cpu.start();

        cpu.step(&mut memory);

        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.registers[1], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }
}