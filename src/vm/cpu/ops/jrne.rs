use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_jrne(&mut self) -> ops::Result {
        let target = match self.fetch_4bytes() {
            None => return Err("Cannot fetch target"),
            Some(bytes) => bytes,
        } + self.cs;

        if !self.flags.zero {
            self.pc = target;
        }

        if self.opts.print_op {
            println!("JRNE {:#010x}", target);
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_jrne_zero() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(0, &[
            Op::JRNE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            Op::MOVI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            Op::MOVI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ]);
        cpu.flags.zero = false;
        cpu.cs = 0;
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 2);
    }

    #[test]
    fn test_jrne_nonzero() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(0, &[
            Op::JRNE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            Op::MOVI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            Op::MOVI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ]);
        cpu.flags.zero = true;
        cpu.cs = 0;
        cpu.pc = 0;
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
    }
}