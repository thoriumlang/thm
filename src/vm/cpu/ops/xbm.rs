use std::time::{Duration, Instant};

use crate::cpu::{CPU, ops};
use crate::memory::Memory;

impl CPU {
    pub(in super::super) fn op_xbm(&mut self, memory: &Memory) -> ops::Result {
        let i = self.fetch_byte(memory)
            .ok_or("xbm: cannot fetch i")? as usize;

        if self.opts.print_op {
            println!("{:03}\tXBM  {}", self.meta.steps, i);
        }

        let prev = self.meta.bench[i];
        self.meta.bench[i] = Instant::now();

        let delta = Self::scale(self.meta.bench[i].duration_since(prev));
        println!("bench {}: {} {}", i, delta.0, delta.1);

        Ok(())
    }

    fn scale(d: Duration) -> (u128, String) {
        if d.as_nanos() < 10000 {
            (d.as_nanos(), "ns".into())
        } else if d.as_micros() < 10000 {
            (d.as_micros(), "us".into())
        } else if d.as_millis() < 10000 {
            (d.as_millis(), "ms".into())
        } else {
            (d.as_secs() as u128, "sec".into())
        }
    }
}