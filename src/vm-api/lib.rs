pub mod op;

pub const REG_COUNT: usize = 32;
/// stack size, in elements count (each element is 4 bytes)
pub const STACK_SIZE: usize = 1024;
/// memory size, in bytes
pub const MEMORY_SIZE: usize = STACK_SIZE * 4;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_memory_large_enough() {
        assert_eq!(true, MEMORY_SIZE >= STACK_SIZE * 4);
    }
}