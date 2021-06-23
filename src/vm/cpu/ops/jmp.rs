use crate::cpu::CPU;

impl CPU {
    pub(in super::super) fn op_jmp(&mut self) {
        self.pc = self.fetch_4bytes() as usize;
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_jmp() {
        let mut cpu = CPU::new();

        cpu.program = vec![
            // JMP #12 (i.e. LOAD 0, #2)
            Op::JMP.bytecode(), 0x00, 0x00, 0x00, 0x0C,
            // LOAD 0, #1
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::HALT.bytecode(),
            // LOAD 0, #2
            Op::LOAD.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::HALT.bytecode()
        ];
        cpu.run();
        assert_eq!(cpu.registers[0], 2);
    }
}