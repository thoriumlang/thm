use std::convert::TryInto;
use std::sync::Arc;

use minifb::{Key, Window, WindowOptions};

use vmlib::{HEIGHT, PIXEL_DEPTH, VIDEO_BUFFER_0, VIDEO_BUFFER_1, VIDEO_BUFFER_SIZE, VIDEO_START, WIDTH};

use crate::memory::Memory;

pub struct Video {
    memory: Arc<Memory>,
}

impl Video {
    pub fn new(memory: Arc<Memory>) -> Video {
        let _ = memory.set_bytes(VIDEO_START as u32, &vec![0x00, 0x00, 0x00, 0x00]);
        let _ = memory.set_bytes((VIDEO_START + 0x04) as u32, &(WIDTH as u32).to_be_bytes());
        let _ = memory.set_bytes((VIDEO_START + 0x08) as u32, &(HEIGHT as u32).to_be_bytes());
        let _ = memory.set_bytes((VIDEO_START + 0x0c) as u32, &(PIXEL_DEPTH as u32).to_be_bytes());
        let _ = memory.set_bytes((VIDEO_START + 0x10) as u32, &(VIDEO_BUFFER_SIZE as u32).to_be_bytes());
        let _ = memory.set_bytes((VIDEO_START + 0x14) as u32, &(VIDEO_BUFFER_0 as u32).to_be_bytes());
        let _ = memory.set_bytes((VIDEO_START + 0x18) as u32, &(VIDEO_BUFFER_1 as u32).to_be_bytes());
        Video {
            memory
        }
    }

    pub fn start(&mut self) {
        let mut window = match Window::new("thm", WIDTH, HEIGHT, WindowOptions::default()) {
            Ok(win) => win,
            Err(err) => {
                panic!("Unable to create window {}", err);
            }
        };

        // Limit to max ~60 fps update rate
        window.limit_update_rate(Some(std::time::Duration::from_micros(16600)));

        let mut buffer: Vec<u32> = vec![0; WIDTH * HEIGHT];
        let mut current_buffer_index = 2u8;

        while window.is_open() && !window.is_key_down(Key::Escape) {
            let needs_update: bool;
            {
                let buffer_index = self.memory.get(VIDEO_START as u32).unwrap();
                needs_update = current_buffer_index != buffer_index;
                if needs_update {
                    current_buffer_index = buffer_index;
                }
            }

            if needs_update {
                let buffer_start = match current_buffer_index {
                    0 => VIDEO_BUFFER_0,
                    1 => VIDEO_BUFFER_1,
                    _ => panic!("invalid buffer index"),
                };
                for pixel_idx in 0..buffer.len() {
                    buffer[pixel_idx] = u32::from_be_bytes(
                        self.memory.get_bytes((buffer_start + pixel_idx * 4) as u32, 4)
                            .unwrap()
                            .try_into()
                            .unwrap()
                    );
                }
            }

            window
                .update_with_buffer(&buffer, WIDTH, HEIGHT)
                .unwrap(); // fixme?
        }
    }
}