use vmlib::{ROM_SIZE, ROM_START};

/// Holds teh memory maps and allow to store / load values
pub struct MemoryMap {
    memory_size: usize, // a usize to make it easier to work with Vec internally
    /// RAM goes from 0x00000000 to min(MAX_ADDRESS, ROM_START)
    ram: Vec<u8>,
    /// ROM goes from ROM_START to MAX_ADDRESS
    rom: Vec<u8>,
}

#[derive(Debug, PartialEq)]
enum Location {
    UNMAPPED,
    RAM,
    ROM,
}

impl MemoryMap {
    pub fn new(memory_size: u32) -> MemoryMap {
        let memory_size = memory_size as usize;
        MemoryMap {
            memory_size,
            ram: vec![0; memory_size],
            rom: vec![0; ROM_SIZE],
        }
    }

    /// Sets the memory location to the given value
    pub fn set(&mut self, address: u32, value: u8) -> bool {
        let address = address as usize;

        match self.location(address) {
            Location::RAM => {
                self.ram[address as usize] = value;
                true
            }
            _ => false,
        }
    }

    /// Gets the value stored at memory location
    pub fn get(&self, address: u32) -> Option<u8> {
        let address = address as usize;

        match self.location(address) {
            Location::RAM => Some(self.ram[address]),
            Location::ROM => Some(self.rom[address - ROM_START]),
            _ => None,
        }
    }

    #[inline]
    fn location(&self, address: usize) -> Location {
        if address >= ROM_START && address <= ROM_START + ROM_SIZE {
            return Location::ROM;
        }
        if address < self.memory_size {
            return Location::RAM;
        }
        return Location::UNMAPPED;
    }
}


#[cfg(test)]
mod tests {
    use super::*;
    use vmlib::MAX_ADDRESS;

    #[test]
    fn test_get_unmapped() {
        let mem = MemoryMap::new(0);
        assert_eq!(None, mem.get(0));
    }

    #[test]
    fn test_set_unmapped() {
        let mut mem = MemoryMap::new(0);
        assert_eq!(false, mem.set(0, 1));
    }

    #[test]
    fn test_get_ram() {
        let mem = MemoryMap::new(2);
        assert_eq!(Some(0), mem.get(0));
        assert_eq!(Some(0), mem.get(1));
    }

    #[test]
    fn test_set_ram() {
        let mut mem = MemoryMap::new(2);
        assert_eq!(true, mem.set(0, 1));
        assert_eq!(true, mem.set(1, 1));
    }

    #[test]
    fn test_get_rom() {
        let mem = MemoryMap::new(0);
        assert_eq!(Some(0), mem.get(ROM_START as u32), "rom[start] != 0");
        assert_eq!(Some(0), mem.get(MAX_ADDRESS as u32), "rom[end] != 0");
    }

    #[test]
    fn test_set_rom() {
        let mut mem = MemoryMap::new(0);
        assert_eq!(false, mem.set(ROM_START as u32, 1));
        assert_eq!(false, mem.set((ROM_START + ROM_SIZE) as u32, 1));
    }

    #[test]
    fn test_rom_hides_ram() {
        let mut mem = MemoryMap::new(MAX_ADDRESS as u32);
        assert_eq!(false, mem.set(ROM_START as u32, 1));
    }
}