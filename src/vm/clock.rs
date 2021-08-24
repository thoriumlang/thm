use std::sync::Arc;
use crate::interrupts::PIC;
use std::thread::JoinHandle;
use std::thread;
use vmlib::{CLOCK_SPEED, INT_CLOCK};

pub struct Clock {
    pic: Arc<PIC>,
}

impl Clock {
    pub fn new(pic: Arc<PIC>) -> Clock {
        Clock { pic }
    }

    pub fn start(&self) -> JoinHandle<()> {
        let pic = self.pic.clone();
        thread::Builder::new().name("clock".into()).spawn(move || {
            loop {
                pic.trigger(INT_CLOCK);
                thread::sleep(CLOCK_SPEED);
            }
        }).unwrap()
    }
}