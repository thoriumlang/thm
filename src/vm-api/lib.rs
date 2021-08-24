use std::time::Duration;

pub mod op;

/// registers count
pub const REG_COUNT: usize = 32;

/// Max identifier of general purpose registers
pub const MAX_REGISTER: usize = REG_COUNT - 1;

pub const REG_PC: usize = MAX_REGISTER + 1;
pub const REG_SP: usize = MAX_REGISTER + 2;
pub const REG_CS: usize = MAX_REGISTER + 3;

pub const INTERRUPTS_COUNT: usize = 256;
pub const INT_API: u8 = 0;
pub const INT_CLOCK: u8 = 1;

pub const CLOCK_SPEED: Duration = Duration::from_micros(1);

// fixme use CpuWord instead of u32?
type CpuWord = u32;

/// Maximum address
pub const MAX_ADDRESS: usize = CpuWord::MAX as usize;

/// stack length, in elements count
// fixme use CpuWord?
pub const STACK_LEN: usize = 1024;

/// Stack size, in bytes
pub const STACK_SIZE: usize = STACK_LEN * 4;

/// Min valid stack address
// fixme use CpuWord?
pub const STACK_MIN_ADDRESS: usize = 0;

/// Max valid stack address
// fixme use CpuWord?
pub const STACK_MAX_ADDRESS: usize = STACK_SIZE - 1;

/// Minimum RAM size, in bytes
// fixme use CpuWord?
pub const MIN_RAM_SIZE: usize = STACK_SIZE;

/// ROM size, in bytes
// fixme use CpuWord?
pub const ROM_SIZE: usize = 32 * 1024 * 1024; // 32 MB

/// ROM start address
// fixme use CpuWord?
pub const ROM_START: usize = MAX_ADDRESS - ROM_SIZE + 1;

pub const IV_SIZE: usize = INTERRUPTS_COUNT * 4;
pub const IV_START: usize = ROM_START - IV_SIZE;

// fixme make it better
//pub const WIDTH: usize = 800;
//pub const HEIGHT: usize = 600;
pub const WIDTH: usize = 320;
pub const HEIGHT: usize = 200;
pub const PIXEL_DEPTH: usize = 4; // todo we only need 3 bytes (rgb)

pub const VIDEO_BUFFER_SIZE: usize = WIDTH * HEIGHT * PIXEL_DEPTH;
pub const VIDEO_BUFFER_1: usize = IV_START - VIDEO_BUFFER_SIZE;
pub const VIDEO_BUFFER_0: usize = VIDEO_BUFFER_1 - VIDEO_BUFFER_SIZE;
// pub const VIDEO_START: usize = VIDEO_BUFFER_1 - 1024;
pub const VIDEO_START: usize = 0xfdc56000;
pub const VIDEO_END: usize = VIDEO_BUFFER_1 + VIDEO_BUFFER_SIZE - 1;

pub const REST_API_SIZE: usize = 1024;
pub const REST_API_START: usize = VIDEO_START - REST_API_SIZE;

// todo review casts

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_memory_large_enough() {
        assert_eq!(true, MIN_RAM_SIZE >= STACK_LEN * 4);
    }

    #[test]
    fn test_rom_boundaries() {
        assert_eq!(MAX_ADDRESS, ROM_START + ROM_SIZE - 1);
    }
}