use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_panic(&mut self, message: &str) -> ops::Result {
        if self.opts.print_op {
            println!("{:03}\tPANIC", self.meta.steps);
        }
        let _ = self.op_halt();
        println!("Panic at {:#010x}: {}", self.pc, message);
        Ok(())
    }
}