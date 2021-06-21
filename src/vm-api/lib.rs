pub mod op;

/// registers count
pub const REG_COUNT: usize = 32;

// fixme use CpuWord instead of u32?
type CpuWord = u32;

/// Maximum address
pub const MAX_ADDRESS: usize = CpuWord::MAX as usize;

/// stack size, in elements count (each element is 4 bytes)
// fixme use CpuWord?
pub const STACK_SIZE: usize = 1024;

/// RAM size, in bytes
// fixme use CpuWord?
pub const RAM_SIZE: usize = STACK_SIZE * 4;

/// ROM size, in bytes
// fixme use CpuWord?
pub const ROM_SIZE: usize = 32 * 1024 * 1024; // 32 MB

/// ROM start address
// fixme use CpuWord?
pub const ROM_START: usize = MAX_ADDRESS - ROM_SIZE + 1;

// todo review casts

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_memory_large_enough() {
        assert_eq!(true, RAM_SIZE >= STACK_SIZE * 4);
    }

    #[test]
    fn test_rom_boundaries() {
        assert_eq!(MAX_ADDRESS, ROM_START + ROM_SIZE - 1);
    }
}