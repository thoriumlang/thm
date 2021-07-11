use crate::cpu::{CPU, ops};

use super::super::vmlib::MAX_REGISTER;

impl CPU {
    pub(in super::super) fn op_mov_ri(&mut self) -> ops::Result {
        let r = match self.fetch_1byte() {
            None => return Err("Cannot fetch r0"),
            Some(byte) => match byte as usize {
                0..=MAX_REGISTER => byte as usize,
                _ => return Err("r is not a valid op register")
            },
        };
        let imm4 = match self.fetch_4bytes() {
            None => return Err("Cannot fetch imm4"),
            Some(byte) => byte,
        } as usize;

        self.registers[r] = imm4 as i32;
        self.flags.zero = self.registers[r] == 0;
        self.flags.negative = self.registers[r] < 0;

        if self.opts.print_op {
            println!("{:03}\tMOV  r{}, {:#010x}", self.meta.steps, r, imm4);
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_mov_ri() {
        let mut cpu = CPU::new();
        let _ = cpu.memory.set_bytes(0, &[
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
        ]);
        cpu.pc = 0;
        cpu.start();
        cpu.step();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_mov_ri_zero() {
        let mut cpu = CPU::new();
        cpu.registers[0] = 1;
        let _ = cpu.memory.set_bytes(0, &[
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x00,
        ]);
        cpu.pc = 0;
        cpu.start();
        cpu.step();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }
}