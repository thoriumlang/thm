use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_panic(&mut self, message: &str) -> ops::Result {
        if self.opts.print_op {
            println!("PANIC");
        }
        let _ = self.op_halt();
        panic!("Panic at {:#010x}: {}", self.pc, message);
    }
}