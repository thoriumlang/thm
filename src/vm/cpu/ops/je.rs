use crate::cpu::CPU;

impl CPU {
    pub(in super::super) fn op_je(&mut self) {
        if self.flags.zero {
            self.pc = self.fetch_4bytes() as usize;
        } else {
            self.skip_4bytes();
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_je_zero() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // JE #12 (i.e. LOAD 0, #2)
            Op::JE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        cpu.flags.zero = true;
        cpu.run();
        assert_eq!(cpu.registers[0], 2);
    }

    #[test]
    fn test_je_nonzero() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // JE #12 (i.e. LOAD 0, #2)
            Op::JE.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        cpu.flags.zero = false;
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
    }
}