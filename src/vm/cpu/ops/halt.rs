use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_halt(&mut self) -> ops::Result {
        if self.opts.print_op {
            println!("{:03}\tHALT", self.meta.steps);
        }
        self.flags.running = false;
        Ok(())
    }
}
