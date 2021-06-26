use crate::cpu::{CPU, ops};

impl CPU {
    pub(in super::super) fn op_halt(&mut self) -> ops::Result {
        if self.opts.print_op {
            println!("HALT");
        }
        self.running = false;
        Ok(())
    }
}
