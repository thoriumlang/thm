use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_ja(&mut self) -> ops::Result {
        let r0 = match self.fetch_1byte() {
            None => return Err("Cannot fetch r0"),
            Some(byte) => byte,
        } as usize;
        let r1 = match self.fetch_1byte() {
            None => return Err("Cannot fetch r1"),
            Some(byte) => byte,
        } as usize;

        let target = self.registers[r0] as u32 + self.registers[r1] as u32;

        if self.opts.print_op {
            println!("{:03}\tJA   r{}, r{} ({:#010x})", self.meta.steps, r0, r1, target);
        }

        self.pc = target;

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use crate::cpu::Op;

    use super::*;

    #[test]
    fn test_ja() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(0, &[
            Op::Ja.bytecode(), 0x00, 0x01,
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::Halt.bytecode(),
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::Halt.bytecode()
        ]);
        cpu.registers[0] = 0;
        cpu.registers[1] = 10;
        cpu.cs = 1;
        cpu.pc = 0;
        cpu.start();
        while cpu.step() {}
        assert_eq!(cpu.registers[0], 2);
    }
}