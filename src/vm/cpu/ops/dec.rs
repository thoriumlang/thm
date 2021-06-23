use crate::cpu::CPU;

impl CPU {
    pub(in super::super) fn op_dec(&mut self) {
        let r = self.fetch_1byte() as usize;
        self.registers[r] -= 1;
        self.flags.zero = self.registers[r] == 0;
        self.flags.negative = self.registers[r] < 0;
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_dec() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // DEC r0
            Op::DEC.bytecode(), 0x00,
            Op::HALT.bytecode()
        ];
        cpu.registers[0] = 2;
        cpu.flags.zero = false;
        cpu.flags.negative = false;
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_dec_zero() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // DEC r0
            Op::DEC.bytecode(), 0x00,
            Op::HALT.bytecode()
        ];
        cpu.registers[0] = 1;
        cpu.flags.zero = false;
        cpu.flags.negative = false;
        cpu.run();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(true, cpu.flags.zero);
        assert_eq!(false, cpu.flags.negative);
    }

    #[test]
    fn test_dec_negative() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // DEC r0
            Op::DEC.bytecode(), 0x00,
            Op::HALT.bytecode()
        ];
        cpu.registers[0] = 0;
        cpu.flags.zero = true;
        cpu.flags.negative = false;
        cpu.run();
        assert_eq!(cpu.registers[0], -1);
        assert_eq!(false, cpu.flags.zero);
        assert_eq!(true, cpu.flags.negative);
    }
}