use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_jr(&mut self) -> ops::Result {
        let target = match self.fetch_4bytes() {
            None => return Err("Cannot fetch target"),
            Some(bytes) => bytes
        } + self.cs;

        if self.opts.print_op {
            println!("{:03}\tJR   {:#010x}", self.meta.steps, target);
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
    fn test_jr() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(0, &[
            Op::Jr.bytecode(), 0x00, 0x00, 0x00, 0x0c,
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x01,
            Op::Halt.bytecode(),
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0x02,
            Op::Halt.bytecode()
        ]);
        cpu.cs = 0;
        cpu.pc = 0;
        cpu.start();
        while cpu.step() {}
        assert_eq!(cpu.registers[0], 2);
    }
}