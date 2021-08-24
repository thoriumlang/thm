use std::cell::UnsafeCell;
use std::cmp::Ordering;
use std::fmt::{Display, Formatter};
use std::ops::RangeInclusive;
use std::sync::Arc;

struct Bytes {
    bytes: UnsafeCell<Vec<u8>>,
}

impl Bytes {
    fn new(bytes: Vec<u8>) -> Bytes {
        Bytes {
            bytes: UnsafeCell::new(bytes)
        }
    }

    pub fn set(&self, index: usize, value: u8) {
        unsafe {
            (*self.bytes.get())[index] = value;
        }
    }

    pub fn set_range(&self, index: usize, value: &[u8]) {
        let mut_data = unsafe {
            &mut (*self.bytes.get())
        };
        mut_data.split_at_mut(index).1.split_at_mut(value.len()).0.copy_from_slice(value);
    }

    pub fn get(&self, index: usize) -> &u8 {
        unsafe {
            &(*self.bytes.get())[index]
        }
    }

    pub fn get_range(&self, index: RangeInclusive<usize>) -> &[u8] {
        unsafe {
            &(*self.bytes.get()).split_at(*index.start()).1.split_at(*index.end() - *index.start() + 1).0
        }
    }
}

unsafe impl Sync for Bytes {}

unsafe impl Send for Bytes {}

pub struct MemoryZone {
    name: String,
    bytes: Bytes,
    access: Access,
    range: RangeInclusive<usize>,
}

impl MemoryZone {
    pub fn new(name: String, range: RangeInclusive<usize>, access: Access) -> MemoryZone {
        MemoryZone {
            name,
            bytes: Bytes::new(vec![0; *range.end() - *range.start() + 1]),
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
            bytes: Bytes::new(bytes),
            access,
            range: from..=from + size - 1,
        }
    }
}

/// Holds the memory maps and allow to store / load values
pub struct Memory {
    zones: Vec<Arc<MemoryZone>>,
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
    pub fn new(zones: Vec<Arc<MemoryZone>>) -> std::result::Result<Memory, String> {
        let mut sorted_zones = Vec::from(zones);
        sorted_zones.sort_by(|z1, z2| {
            if z1.range.start() < z2.range.start() {
                return Ordering::Less;
            } else if z1.range.start() > z2.range.start() {
                return Ordering::Greater;
            }
            return Ordering::Equal;
        });

        for (i, zone) in sorted_zones.iter().enumerate().skip(1) {
            if *zone.range.start() <= *sorted_zones[i - 1].range.end() {
                return Err(format!("Zone {} overlap with previous zone {}", zone.name, sorted_zones[i - 1].name).to_string());
            }
        }

        sorted_zones.reverse();
        let res = Memory {
            zones: sorted_zones,
        };

        // todo set total ram size in 0x00000000

        return Ok(res);
    }

    #[must_use]
    pub fn set_bytes(&self, address: u32, value: &[u8]) -> bool {
        let address = address as usize;
        match self.zone_write(address) {
            Some((offset, zone)) => {
                zone.set_range(address - offset, value);
                true
            }
            _ => false,
        }
    }

    #[must_use]
    // todo remove
    pub fn set(&self, address: u32, value: u8) -> bool {
        self.set_bytes(address, &[value])
    }

    /// Gets the value stored at memory location
    pub fn get(&self, address: u32) -> Option<u8> {
        let address = address as usize;
        match self.zone_read(address) {
            Some((offset, zone)) => Some(*zone.get(address - *offset)),
            _ => None
        }
    }

    pub fn get_bytes(&self, from: u32, size: u32) -> Option<&[u8]> {
        let from = from as usize;
        let size = size as usize;
        match self.zone_read(from) {
            Some((offset, zone)) => Some(zone.get_range((from - *offset)..=(from + size - 1 - *offset))),
            _ => None,
        }
    }

    #[inline]
    fn zone_read(&self, address: usize) -> Option<(&usize, &Bytes)> {
        for z in &self.zones {
            if z.range.contains(&address) {
                return Some((z.range.start(), &z.bytes));
            }
        }
        None
    }

    #[inline]
    fn zone_write(&self, address: usize) -> Option<(&usize, &Bytes)> {
        for z in &self.zones {
            if z.range.contains(&address) && z.access == Access::RW {
                return Some((z.range.start(), &z.bytes));
            }
        }
        None
    }

    pub fn zones(&self) -> Vec<Box<Zone>> {
        let mut zones = Vec::new();
        for zone in &self.zones {
            zones.push(Box::new(Zone {
                name: String::from(&zone.name),
                from: *zone.range.start() as u32,
                to: *zone.range.end() as u32,
                access: zone.access.clone(),
            }));
        }
        zones.sort_by(|a, b| a.from.cmp(&b.from));
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
        let mem = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=0, Access::RW))]).unwrap();
        assert_eq!(false, mem.set(2, 1));
    }

    #[test]
    fn test_set_read_only() {
        let mem = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=31, Access::R))]).unwrap();
        assert_eq!(false, mem.set(0, 1));
    }

    #[test]
    fn test_set_and_get() {
        let mem = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW))]).unwrap();
        let _ = mem.set(0, 1);
        assert_eq!(Some(1), mem.get(0));
        assert_eq!(Some(0), mem.get(1));
    }

    #[test]
    fn test_get_unmapped() {
        let mem = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW))]).unwrap();
        assert_eq!(None, mem.get(32));
    }

    #[test]
    fn test_get_default_0() {
        let mem = Memory::new(vec![Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW))]).unwrap();
        assert_eq!(Some(0), mem.get(0));
    }

    #[test]
    fn test_overlap() {
        let mem = Memory::new(vec![
            Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW)),
            Arc::new(MemoryZone::new("".into(), 31..=31, Access::RW)),
        ]);
        assert_eq!(true, mem.is_err());
    }

    #[test]
    fn test_no_overlap() {
        let mem = Memory::new(vec![
            Arc::new(MemoryZone::new("".into(), 0..=31, Access::RW)),
            Arc::new(MemoryZone::new("".into(), 32..=32, Access::RW)),
        ]);
        assert_eq!(true, mem.is_ok());
    }
}