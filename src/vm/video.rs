use std::convert::TryInto;
use std::sync::{Arc, RwLock};

use minifb::{Key, Window, WindowOptions};

use vmlib::{HEIGHT, PIXEL_DEPTH, VIDEO_BUFFER_SIZE, VIDEO_START, WIDTH};

use crate::memory::MemoryZone;

pub struct Video {
    meta: Arc<RwLock<MemoryZone>>,
    buffer_0: Arc<RwLock<MemoryZone>>,
    buffer_1: Arc<RwLock<MemoryZone>>,
}

impl Video {
    pub fn new(meta: Arc<RwLock<MemoryZone>>, buffer_0: Arc<RwLock<MemoryZone>>, buffer_1: Arc<RwLock<MemoryZone>>) -> Video {
        {
            let mut meta = meta.write().unwrap();
            let _ = meta.set_bytes(VIDEO_START, &vec![0x00, 0x00, 0x00, 0x00]);
            let _ = meta.set_bytes(VIDEO_START + 0x04, &(WIDTH as u32).to_be_bytes());
            let _ = meta.set_bytes(VIDEO_START + 0x08, &(HEIGHT as u32).to_be_bytes());
            let _ = meta.set_bytes(VIDEO_START + 0x0c, &(PIXEL_DEPTH as u32).to_be_bytes());
            let _ = meta.set_bytes(VIDEO_START + 0x10, &(VIDEO_BUFFER_SIZE as u32).to_be_bytes());
            let _ = meta.set_bytes(VIDEO_START + 0x14, &(buffer_0.read().unwrap().from() as u32).to_be_bytes());
            let _ = meta.set_bytes(VIDEO_START + 0x18, &(buffer_1.read().unwrap().from() as u32).to_be_bytes());
        }
        Video {
            meta,
            buffer_0,
            buffer_1
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
                let memory = self.meta.read().unwrap();
                let buffer_index = memory.get(VIDEO_START).unwrap();
                needs_update = current_buffer_index != buffer_index;
                if needs_update {
                    current_buffer_index = buffer_index;
                }
            }

            if needs_update {
                let source = match current_buffer_index {
                    0 => &self.buffer_0,
                    1 => &self.buffer_1,
                    _ => panic!("invalid buffer index"),
                }.read().unwrap();
                for pixel_idx in 0..buffer.len() {
                    buffer[pixel_idx] = u32::from_be_bytes(
                        source.get_bytes_abs(pixel_idx * 4, pixel_idx * 4 + 3)
                            .unwrap()
                            .as_slice()
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