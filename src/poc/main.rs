use std::sync::{Arc, RwLock};
use std::thread;

struct Bus {
    data: RwLock<Vec<u8>>,
}

impl Bus {
    pub fn new() -> Bus {
        Bus {
            data: RwLock::new(vec![0u8; 1024])
        }
    }

    pub fn write(&self, address: usize, value: u8) {
        println!("write {} to {}", value, address);
        self.data.write().unwrap()[address] = value
    }
}

fn main() {
    let bus = Bus::new();
    let bus = Arc::new(bus);

    let bus1 = bus.clone();
    let th1 = thread::spawn(move || {
        bus1.write(0, 1);
    });

    let bus2 = bus.clone();
    let th2 = thread::spawn(move || {
        bus2.write(515, 2);
    });

    let _ = th1.join();
    let _ = th2.join();
}