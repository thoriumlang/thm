use crate::cpu::CPU;

impl CPU {
    pub(in super::super) fn op_load(&mut self) {
        let r = self.fetch_1byte() as usize;
        let immediate = self.fetch_4bytes() as i32;
        self.registers[r] = immediate;
        self.flags.zero = self.registers[r] == 0;
        self.flags.negative = self.registers[r] < 0;
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_load() {
        let mut cpu = CPU::new();
        cpu.program = vec![
            // LOAD 0, #16909320
            Op::LOAD.bytecode(), 0x00, 0x01, 0x02, 0x04, 0x08,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 16909320);
        assert_eq!(cpu.flags.zero, false);
        assert_eq!(cpu.flags.negative, false);
    }

    #[test]
    fn test_load_zero() {
        let mut cpu = CPU::new();
        cpu.program = vec![
            // LOAD 0, #0
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x00,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 0);
        assert_eq!(cpu.flags.zero, true);
        assert_eq!(cpu.flags.negative, false);
    }
}