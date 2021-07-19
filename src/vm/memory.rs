use std::cmp::Ordering;
use std::fmt::{Display, Formatter};
use std::ops::RangeInclusive;
use std::sync::{Arc, RwLock, RwLockWriteGuard};

pub struct MemoryZone {
    name: String,
    bytes: Vec<u8>,
    access: Access,
    range: RangeInclusive<usize>,
}

impl MemoryZone {
    pub fn new(name: String, range: RangeInclusive<usize>, access: Access) -> MemoryZone {
        MemoryZone {
            name,
            bytes: vec![0; *range.end() - *range.start() + 1],
            access,
            range,
        }
    }

    pub fn new_with_size(name: String, from: usize, size: usize, access: Access) -> MemoryZone {
        Self::new(
            name,
            from..=from + size - 1,
            access,
        )
    }


    pub fn new_with_data(name: String, from: usize, size: usize, bytes: Vec<u8>, access: Access) -> MemoryZone {
        MemoryZone {
            name,
            bytes,
            access,
            range: from..=from + size - 1,
        }
    }

    pub fn from(&self) -> usize { *self.range.start() }

    pub fn access(&self) -> &Access { &self.access }

    pub fn get(&self, index: usize) -> Option<u8> {
        if index < *self.range.start() || index > *self.range.end() {
            return None;
        }
        Some(self.bytes[index - *self.range.start()])
    }

    pub fn get_bytes(&self, from: usize, to: usize) -> Option<Vec<u8>> {
        if from < *self.range.start() || to > *self.range.end() {
            return None;
        }
        Some(self.bytes[(from - *self.range.start())..=(to - *self.range.start())].to_vec())
    }

    pub fn get_bytes_abs(&self, from: usize, to: usize) -> Option<Vec<u8>> {
        self.get_bytes(*self.range.start() + from, *self.range.start() + to)
    }

    pub fn set(&mut self, address: usize, value: u8) -> bool {
        if address < *self.range.start() || address > *self.range.end() {
            return false;
        }
        self.bytes[address - *self.range.start()] = value;
        true
    }

    pub fn set_bytes(&mut self, from: usize, bytes: &[u8]) -> bool {
        for (i, b) in bytes.iter().enumerate() {
            if !self.set(from + i, *b) {
                return false;
            }
        }
        true
    }
}

/// Holds the memory maps and allow to store / load values
pub struct Memory {
    zones: Vec<Arc<RwLock<MemoryZone>>>,
}

#[derive(Debug, PartialEq, Clone)]
pub enum Access {
    RW,
    R,
}

impl Display for Access {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            &Access::RW => write!(f, "RW"),
            &Access::R => write!(f, "R"),
        }
    }
}

#[derive(Debug, PartialEq)]
// todo refactor me!
pub struct Zone {
    name: String,
    from: u32,
    to: u32,
    access: Access,
}

impl Display for Zone {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(f, "zone {}\t{}\t{:#010x} - {:#010x} ({} Bytes)", self.access, self.name, self.from, self.to, self.to - self.from + 1)
    }
}

impl Memory {
    pub fn new(zones: Vec<Arc<RwLock<MemoryZone>>>) -> std::result::Result<Memory, String> {
        let mut sorted_zones = Vec::from(zones);
        sorted_zones.sort_by(|z1, z2| {
            if z1.read().unwrap().range.start() < z2.read().unwrap().range.start() {
                return Ordering::Less;
            } else if z1.read().unwrap().range.start() > z2.read().unwrap().range.start() {
                return Ordering::Greater;
            }
            return Ordering::Equal;
        });

        for (i, zone) in sorted_zones.iter().enumerate().skip(1) {
            let zone = zone.read().unwrap();
            if *zone.range.start() <= *sorted_zones[i - 1].read().unwrap().range.end() {
                return Err(format!("Zone {} overlap with previous zone {}", zone.name, sorted_zones[i - 1].read().unwrap().name).to_string());
            }
        }

        let res = Memory {
            zones: sorted_zones,
        };

        // todo set total ram size in 0x00000000

        return Ok(res);
    }

    /// Sets the memory location to the given value
    #[must_use]
    pub fn set(&mut self, address: u32, value: u8) -> bool {
        let address = address as usize;

        match self.zone_write(address) {
            Some(mut z) => match z.access() {
                Access::R => false,
                Access::RW => {
                    z.set(address, value);
                    true
                }
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
        match self.zone_write(address) {
            Some(z) => {
                Some(z.get(address).unwrap())
            }
            _ => {
                println!("Tried to read from {:#010x}", address);
                None
            }
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
    fn zone_write(&self, address: usize) -> Option<RwLockWriteGuard<MemoryZone>> {
        for zone in &self.zones {
            let zone_w = zone.write().unwrap();
            if *zone_w.range.start() <= address && *zone_w.range.end() >= address {
                return Some(zone_w);
            }
        }
        return None;
    }

    pub fn zones(&self) -> Vec<Box<Zone>> {
        let mut zones = Vec::new();
        for zone in &self.zones {
            zones.push(Box::new(Zone {
                name: String::from(&zone.read().unwrap().name),
                from: *zone.read().unwrap().range.start() as u32,
                to: *zone.read().unwrap().range.end() as u32,
                access: zone.read().unwrap().access.clone(),
            }));
        }
        return zones;
    }

    pub fn dump(&self, start: u32, end: u32) {
        println!("Dump of {:#010x} - {:#010x}", start, end);
        for address in start..=end {
            if address % 16 == 0 {
                print!("{:08x}  ", address as u32)
            }
            print!("{:02x} ", self.get(address).unwrap());
            if address % 8 == 7 {
                print!(" ");
            }
            if address % 16 == 15 {
                println!()
            }
        }
        println!()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_set_unmapped() {
        let mut mem = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=0, Access::RW)))]).unwrap();
        assert_eq!(false, mem.set(2, 1));
    }

    #[test]
    fn test_set_read_only() {
        let mut mem = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::R)))]).unwrap();
        assert_eq!(false, mem.set(0, 1));
    }

    #[test]
    fn test_set_and_get() {
        let mut mem = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        let _ = mem.set(0, 1);
        assert_eq!(Some(1), mem.get(0));
        assert_eq!(Some(0), mem.get(1));
    }

    #[test]
    fn test_get_unmapped() {
        let mem = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        assert_eq!(None, mem.get(32));
    }

    #[test]
    fn test_get_default_0() {
        let mem = Memory::new(vec![Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW)))]).unwrap();
        assert_eq!(Some(0), mem.get(0));
    }

    #[test]
    fn test_overlap() {
        let mem = Memory::new(vec![
            Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW))),
            Arc::new(RwLock::new(MemoryZone::new("".into(), 31..=31, Access::RW))),
        ]);
        assert_eq!(true, mem.is_err());
    }

    #[test]
    fn test_no_overlap() {
        let mem = Memory::new(vec![
            Arc::new(RwLock::new(MemoryZone::new("".into(), 0..=31, Access::RW))),
            Arc::new(RwLock::new(MemoryZone::new("".into(), 32..=32, Access::RW))),
        ]);
        assert_eq!(true, mem.is_ok());
    }
}