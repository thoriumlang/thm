use std::sync::{Arc, RwLock};

use minifb::{Key, Window, WindowOptions};

use vmlib::{HEIGHT, PIXEL_DEPTH, VIDEO_BUFFER_0, VIDEO_BUFFER_1, VIDEO_BUFFER_SIZE, VIDEO_START, WIDTH};

use crate::memory::Memory;

pub struct Video {
    memory: Arc<RwLock<Memory>>,
}

impl Video {
    pub fn new(memory: Arc<RwLock<Memory>>) -> Video {
        {
            let mut memory = memory.write().unwrap();
            let _ = memory.set_bytes(VIDEO_START as u32, &vec![0x00, 0x00, 0x00, 0x00]);
            let _ = memory.set_bytes((VIDEO_START + 4) as u32, &(WIDTH as u32).to_be_bytes());
            let _ = memory.set_bytes((VIDEO_START + 8) as u32, &(HEIGHT as u32).to_be_bytes());
            let _ = memory.set_bytes((VIDEO_START + 12) as u32, &(PIXEL_DEPTH as u32).to_be_bytes());
            let _ = memory.set_bytes((VIDEO_START + 16) as u32, &(VIDEO_BUFFER_SIZE as u32).to_be_bytes());
            let _ = memory.set_bytes((VIDEO_START + 20) as u32, &(VIDEO_BUFFER_0 as u32).to_be_bytes());
            let _ = memory.set_bytes((VIDEO_START + 24) as u32, &(VIDEO_BUFFER_1 as u32).to_be_bytes());
        }
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
        let mut memory_map: Vec<u8> = vec![0; VIDEO_BUFFER_SIZE];
        // invalid buffer idx so we copy it the first time we loop
        let mut current_buffer_index = 2u8;

        while window.is_open() && !window.is_key_down(Key::Escape) {
            let needs_update: bool;
            {
                let memory = self.memory.read().unwrap();
                let buffer_index = memory.get(VIDEO_START as u32).unwrap();
                needs_update = current_buffer_index != buffer_index;
                if needs_update {
                    let buffer_address = match buffer_index {
                        0 => VIDEO_BUFFER_0,
                        1 => VIDEO_BUFFER_1,
                        _ => panic!("invalid buffer index"),
                    } as u32;
                    memory_map.copy_from_slice(memory.get_bytes(buffer_address, VIDEO_BUFFER_SIZE as u32).unwrap().as_slice());
                    current_buffer_index = buffer_index;
                }
            }

            if needs_update {
                for pixel_idx in 0..buffer.len() {
                    let mut pixel_byte: u32 = 0;
                    for pixel_byte_idx in 0..4 {
                        pixel_byte = pixel_byte << 8;
                        pixel_byte |= memory_map[pixel_idx * 4 + pixel_byte_idx] as u32;
                    }
                    buffer[pixel_idx] = pixel_byte;
                }
            }

            window
                .update_with_buffer(&buffer, WIDTH, HEIGHT)
                .unwrap(); // fixme?
        }
    }
}