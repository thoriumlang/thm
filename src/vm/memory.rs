use std::fmt::{Display, Formatter};

use vmlib::{ROM_SIZE, ROM_START};

use crate::memory::Location::ROM;

/// Holds the memory maps and allow to store / load values
pub struct Memory {
    // a usize to make it easier to work with Vec internally
    memory_size: usize,
    /// RAM goes from 0x00000000 to min(MAX_ADDRESS, ROM_START)
    ram: Vec<u8>,
    /// ROM goes from ROM_START to MAX_ADDRESS
    rom: Vec<u8>,
}

#[derive(Debug, PartialEq)]
pub enum Location {
    UNMAPPED,
    RAM,
    ROM,
}

#[derive(Debug, PartialEq)]
pub struct Zone {
    from: u32,
    to: u32,
    kind: Location,
}

impl Display for Zone {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(f, "zone {:?}: {:#010x} - {:#010x} ({} Bytes)", self.kind, self.from, self.to, self.to - self.from + 1)
    }
}

impl Memory {
    pub fn new(memory_size: u32, rom: Vec<u8>) -> Memory {
        let memory_size = memory_size as usize;

        let mut ram = vec![0; memory_size];
        if memory_size >= 4 { // todo find a cleaner way... like absolute min ram size?
            Self::write_memory_size(memory_size as u32, &mut ram);
        }

        Memory {
            memory_size,
            ram,
            rom,
        }
    }

    fn write_memory_size(memory_size: u32, memory: &mut Vec<u8>) {
        for (i, b) in memory_size.to_be_bytes().iter().enumerate() {
            memory[i] = *b;
        }
    }

    /// Sets the memory location to the given value
    #[must_use]
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

    #[must_use]
    pub fn set_bytes(&mut self, address: u32, value: &[u8]) -> bool {
        for (i, b) in value.iter().enumerate() {
            if !self.set(address + i as u32, *b) {
                return false;
            }
        }
        true
    }

    /// Gets the value stored at memory location
    pub fn get(&self, address: u32) -> Option<u8> {
        let address = address as usize;

        match self.location(address) {
            Location::RAM => Some(self.ram[address]),
            Location::ROM => Some({
                if self.rom.len() > address - ROM_START {
                    self.rom[address - ROM_START]
                } else {
                    0
                }
            }),
            _ => None,
        }
    }

    pub fn get_bytes(&self, from: u32, size: u32) -> Option<Vec<u8>> {
        let mut vec: Vec<u8> = Vec::with_capacity(size as usize);
        for i in from..(from + size) {
            match self.get(i) {
                None => return None,
                Some(v) => vec.push(v)
            }
        }
        Some(vec)
    }

    #[inline]
    fn location(&self, address: usize) -> Location {
        if address >= ROM_START && address < ROM_START + ROM_SIZE {
            return Location::ROM;
        }
        if address < self.memory_size {
            return Location::RAM;
        }
        return Location::UNMAPPED;
    }

    pub fn zones(&self) -> Vec<Zone> {
        vec![
            Zone {
                from: 0,
                to: (self.memory_size - 1) as u32,
                kind: Location::RAM,
            },
            Zone {
                from: ROM_START as u32,
                to: (ROM_START + ROM_SIZE - 1) as u32,
                kind: ROM,
            }
        ]
    }

    pub fn dump(&self, start: u32, end: u32) {
        println!("Dump of {:#010x} - {:#010x}", start, end);
        for (i, b) in self.ram.iter().enumerate().skip(start as usize).take((end - start) as usize) {
            if i % 16 == 0 {
                print!("{:08x}  ", i as u32)
            }
            print!("{:02x} ", b);
            if i % 8 == 7 {
                print!(" ");
            }
            if i % 16 == 15 {
                println!()
            }
        }
        println!()
    }
}

#[cfg(test)]
mod tests {
    use vmlib::MAX_ADDRESS;

    use super::*;

    #[test]
    fn test_get_unmapped() {
        let mem = Memory::new(0, vec![0; ROM_SIZE]);
        assert_eq!(None, mem.get(0));
    }

    #[test]
    fn test_set_unmapped() {
        let mut mem = Memory::new(0, vec![0; ROM_SIZE]);
        assert_eq!(false, mem.set(0, 1));
    }

    #[test]
    fn test_get_ram() {
        let mem = Memory::new(2, vec![0; ROM_SIZE]);
        assert_eq!(Some(0), mem.get(0));
        assert_eq!(Some(0), mem.get(1));
    }

    #[test]
    fn test_set_ram() {
        let mut mem = Memory::new(2, vec![0; ROM_SIZE]);
        assert_eq!(true, mem.set(0, 1));
        assert_eq!(true, mem.set(1, 1));
    }

    #[test]
    fn test_get_rom() {
        let mem = Memory::new(0, vec![1; ROM_SIZE]);
        assert_eq!(Some(1), mem.get(ROM_START as u32), "rom[start] != 1");
        assert_eq!(Some(1), mem.get(MAX_ADDRESS as u32), "rom[end] != 1");
    }

    #[test]
    fn test_get_rom_default_0() {
        let mem = Memory::new(0, vec![1; 2]);
        assert_eq!(Some(1), mem.get(ROM_START as u32), "rom[start] != 1");
        assert_eq!(Some(1), mem.get((ROM_START + 1) as u32), "rom[start+1] != 1");
        assert_eq!(Some(0), mem.get((ROM_START + 2) as u32), "rom[start+2] != 1");
        assert_eq!(Some(0), mem.get(MAX_ADDRESS as u32), "rom[end] != 1");
    }

    #[test]
    fn test_set_rom() {
        let mut mem = Memory::new(0, vec![0; ROM_SIZE]);
        assert_eq!(false, mem.set(ROM_START as u32, 1));
        assert_eq!(false, mem.set((ROM_START + ROM_SIZE) as u32, 1));
    }

    #[test]
    fn test_rom_hides_ram() {
        let mut mem = Memory::new(MAX_ADDRESS as u32, vec![0; ROM_SIZE]);
        assert_eq!(false, mem.set(ROM_START as u32, 1));
    }
}