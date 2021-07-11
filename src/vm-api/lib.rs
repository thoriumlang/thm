pub mod op;

/// registers count
pub const REG_COUNT: usize = 32;

/// Max identifier of general purpose registers
pub const MAX_REGISTER: usize = REG_COUNT - 1;

pub const REG_PC: usize = 255;
pub const REG_SP: usize = 254;
pub const REG_CS: usize = 253;

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

pub const VIDEO_BUFFER_1: usize = 0xFD69F800;

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