use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_dec(&mut self) -> ops::Result {
        let r = match self.fetch_1byte() {
            None => return Err("Cannot fetch r"),
            Some(byte) => byte
        } as usize;
        self.registers[r] -= 1;
        self.flags.zero = self.registers[r] == 0;
        self.flags.negative = self.registers[r] < 0;

        if self.opts.print_op {
            println!("{:03}\tDEC  r{}", self.meta.steps, r);
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_dec() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(0, &[
            Op::Dec.bytecode(), 0x00,
        ]);
        cpu.registers[0] = 2;
        cpu.flags.zero = false;
        cpu.flags.negative = false;
        cpu.pc = 0;
        cpu.start();
        cpu.step();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_dec_zero() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(0, &[
            Op::Dec.bytecode(), 0x00,
        ]);
        cpu.registers[0] = 1;
        cpu.flags.zero = false;
        cpu.flags.negative = false;
        cpu.pc = 0;
        cpu.start();
        cpu.step();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(true, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_dec_negative() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(0, &[
            Op::Dec.bytecode(), 0x00,
        ]);
        cpu.registers[0] = 0;
        cpu.flags.zero = true;
        cpu.flags.negative = false;
        cpu.pc = 0;
        cpu.start();
        cpu.step();
        assert_eq!(cpu.registers[0], -1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(true, cpu.flags.negative);
    }
}