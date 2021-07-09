use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_call(&mut self) -> ops::Result {
        let target = match self.fetch_4bytes() {
            None => return Err("Cannot fetch addr"),
            Some(byte) => byte
        } + self.cs;

        if self.opts.print_op {
            println!("{:03}\tCALL {:#010x}", self.meta.steps, target);
        }

        // todo maybe mutualize code for call/push
        for byte in self.pc.to_le_bytes().iter() {
            self.sp -= 1;
            if !self.memory.set(self.sp, *byte) {
                return Err("Cannot set");
            }
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
    fn test_call() {
        let mut cpu = CPU::new();

        let _ = cpu.memory.set_bytes(4, &[
            Op::Call.bytecode(), 0x00, 0x00, 0x00, 0x06,
            Op::Panic.bytecode(),
            Op::MovRI.bytecode(), 0x00, 0x00, 0x00, 0x00, 0xff,
            Op::Halt.bytecode()
        ]);
        cpu.sp = 4;
        cpu.cs = cpu.sp;
        cpu.pc = cpu.cs;
        cpu.start();
        while cpu.step() {}
        assert_eq!(cpu.registers[0], 0x000000ff, "r0 {} != 0x000000ff", cpu.registers[0]);
        assert_eq!(0, cpu.sp, "sp {} != 0", cpu.sp);
        assert_eq!(Some(0x00), cpu.memory.get(0), "mem[0]: 1 != {:?}", cpu.memory.get(0));
        assert_eq!(Some(0x00), cpu.memory.get(1), "mem[1]: 2 != {:?}", cpu.memory.get(1));
        assert_eq!(Some(0x00), cpu.memory.get(2), "mem[2]: 3 != {:?}", cpu.memory.get(2));
        assert_eq!(Some(0x09), cpu.memory.get(3), "mem[3]: 4 != {:?}", cpu.memory.get(3));
    }
}