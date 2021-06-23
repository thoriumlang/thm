use crate::cpu::CPU;

impl CPU {
    pub(in super::super) fn op_cmp(&mut self) {
        let r0 = self.fetch_1byte() as usize;
        let r1 = self.fetch_1byte() as usize;
        self.flags.zero = self.registers[r0] == self.registers[r1];
        self.flags.negative = self.registers[r0] < self.registers[r1];
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_cmp_eq() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 1;
        cpu.registers[1] = 1;
        cpu.program = vec![
            Op::CMP.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.registers[1], 1);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_cmp_lt() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 1;
        cpu.registers[1] = 2;
        cpu.program = vec![
            Op::CMP.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 1);
        assert_eq!(cpu.registers[1], 2);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, true);
    }

    #[test]
    fn test_cmp_gt() {
        let mut cpu = CPU::new();

        cpu.registers[0] = 2;
        cpu.registers[1] = 1;
        cpu.program = vec![
            Op::CMP.bytecode(), 0x00, 0x01,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 2);
        assert_eq!(cpu.registers[1], 1);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }
}