use std::cell::UnsafeCell;

use vmlib::INTERRUPTS_COUNT;

pub struct PIC {
    lines: UnsafeCell<[bool; INTERRUPTS_COUNT]>,
    masked: UnsafeCell<[bool; INTERRUPTS_COUNT]>,
}

unsafe impl Sync for PIC {}

unsafe impl Send for PIC {}

impl PIC {
    pub fn new() -> PIC {
        PIC {
            lines: UnsafeCell::new([false; INTERRUPTS_COUNT]),
            masked: UnsafeCell::new([false; INTERRUPTS_COUNT]),
        }
    }

    pub fn trigger(&self, int: u8) {
        unsafe {
            (*self.lines.get())[int as usize] = true;
        }
    }

    #[inline]
    fn reset(&self, int: u8) {
        unsafe {
            (*self.lines.get())[int as usize] = false;
        }
    }

    pub fn poll(&self) -> Option<u8> {
        let lines = unsafe {
            *self.lines.get()
        };
        let masked = unsafe {
            *self.masked.get()
        };
        for (int, active) in lines.iter().enumerate() {
            if *active && !masked[int] {
                self.mask(int as u8);
                self.reset(int as u8);
                return Some(int as u8);
            }
        }
        None
    }

    pub fn mask(&self, int: u8) {
        unsafe {
            (*self.masked.get())[int as usize] = true;
        }
    }

    pub fn unmask(&self, int: u8) {
        unsafe {
            (*self.masked.get())[int as usize] = false;
        }
    }
}