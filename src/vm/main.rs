use std::fs::OpenOptions;
use std::io::Read;

use clap::{App, Arg, ArgMatches, crate_authors, crate_version};

use cpu::CPU;
use vmlib::{MIN_RAM_SIZE, STACK_LEN, STACK_MAX_ADDRESS, STACK_SIZE};
use vmlib::op::Op;

use crate::memory_map::MemoryMap;

mod cpu;
mod memory_map;

fn main() {
    let matches = parse_opts();
    let rom_file = matches.value_of("rom").unwrap();
    let program_file = matches.value_of("image").unwrap();

    let mut rom = vec![];
    OpenOptions::new()
        .create(false)
        .read(true)
        .open(rom_file)
        .unwrap()
        .read_to_end(&mut rom).unwrap();

    let mut program: Vec<u8> = vec![];
    OpenOptions::new()
        .create(false)
        .read(true)
        .open(program_file)
        .unwrap()
        .read_to_end(&mut program).unwrap();

    // todo move to vm args
    const RAM_SIZE: usize = MIN_RAM_SIZE + 60;
    if RAM_SIZE < MIN_RAM_SIZE {
        panic!("Not enough RAM: {} < {}", RAM_SIZE, MIN_RAM_SIZE);
    }

    let mut memory_map = MemoryMap::new(RAM_SIZE as u32, rom);

    // fixme this is a hack to start executing whatever comes after the stack...
    let mut init = vec![Op::LOAD.bytecode(), 30];
    init.append(&mut ((STACK_MAX_ADDRESS + 1) as u32).to_be_bytes().to_vec());
    init.append(&mut vec![Op::LOAD.bytecode(), 31, 0, 0, 0, 0, Op::JA.bytecode(), 30, 31]);
    if !memory_map.set_bytes(0, init.as_slice()) { panic!("cannot set") };
    // fixme end of hack

    // copy program
    program.iter().enumerate().for_each(|(i, b)| {
        if !memory_map.set((STACK_MAX_ADDRESS + 1 + i) as u32, *b) { panic!("cannot set") }
    });

    println!("RAM size: {} Bytes", RAM_SIZE);
    println!("Stack:    {} Bytes ({} 32-bits words)", STACK_SIZE, STACK_LEN);
    println!("          {:#010x} - {:#010x}", 0, STACK_MAX_ADDRESS);
    println!("Free:     {} Bytes", RAM_SIZE - STACK_SIZE);
    println!("          {:#010x} - {:#010x}", STACK_MAX_ADDRESS + 1, RAM_SIZE - 1);
    memory_map.zones().iter().for_each(|z| println!("{}", z));
    // memory_map.dump(0, 32);
    // memory_map.dump((STACK_MAX_ADDRESS as u32) + 1, ROM_START as u32);

    let mut cpu = CPU::new_custom_memory(memory_map);
    cpu.registers[0] = 15;
    cpu.run();

    println!("f({}): {}", cpu.registers[0], cpu.registers[3])
}

fn parse_opts<'a>() -> ArgMatches<'a> {
    App::new("Thorium VM")
        .version(crate_version!())
        .author(crate_authors!())
        .about("The Thorium VM")
        .arg(
            Arg::with_name("rom")
                .help("Sets rom to load")
                .required(true)
                .index(1),
        )
        .arg(
            Arg::with_name("image")
                .help("Sets rom to load")
                .required(true)
                .index(2),
        )
        .get_matches()
}
