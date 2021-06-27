use std::fs::OpenOptions;
use std::io::{Read, Write};
use std::str::FromStr;

use clap::{App, Arg, ArgMatches, crate_authors, crate_version};

use cpu::CPU;
use vmlib::{MIN_RAM_SIZE, ROM_START, STACK_LEN, STACK_MAX_ADDRESS, STACK_SIZE};

use crate::memory_map::MemoryMap;

mod cpu;
mod memory_map;

fn main() {
    let opts = parse_opts();

    let rom = load_bin(opts.value_of("rom").unwrap());
    let program: Vec<u8> = load_bin(opts.value_of("image").unwrap());
    let ram_size = opts.value_of("ram")
        .map(|s| usize::from_str(s).unwrap_or(MIN_RAM_SIZE + 128))
        .unwrap_or(MIN_RAM_SIZE + 128);

    if ram_size < MIN_RAM_SIZE + program.len() {
        panic!("Not enough RAM: {} < {}", ram_size, MIN_RAM_SIZE + program.len());
    }

    let mut memory_map = MemoryMap::new(ram_size as u32, rom);
    if !memory_map.set_bytes((STACK_MAX_ADDRESS + 1) as u32, program.as_slice()) {
        panic!("Cannot copy program to ram");
    }

    if opts.is_present("mmap") {
        println!("RAM size: {} Bytes", ram_size);
        println!("Stack:    {} Bytes ({} 32-bits words)", STACK_SIZE, STACK_LEN);
        println!("          {:#010x} - {:#010x}", 0, STACK_MAX_ADDRESS);
        println!("Free:     {} Bytes", ram_size - STACK_SIZE);
        println!("          {:#010x} - {:#010x}", STACK_MAX_ADDRESS + 1, ram_size - 1);
        memory_map.zones().iter().for_each(|z| println!("{}", z));
        memory_map.dump(0, 16);
        memory_map.dump((STACK_MAX_ADDRESS as u32) + 1, ROM_START as u32);
    }

    let mut cpu = CPU::new_custom_memory(memory_map);
    cpu.sp = STACK_MAX_ADDRESS as u32;
    cpu.cs = (STACK_MAX_ADDRESS + 1) as u32;

    cpu.registers[0] = 16;

    if opts.is_present("step") {
        cpu.set_opts(cpu::Opts {
            print_op: true,
        });
        cpu.dump();
        println!();
        while cpu.step() {
            let mut buffer = String::new();
            cpu.dump();
            print!("> ");
            std::io::stdout().flush().unwrap();
            std::io::stdin().read_line(&mut buffer).unwrap();
            buffer.truncate(0);
        }
    } else {
        cpu.run();
    }

    println!("f({}): {}", cpu.registers[0], cpu.registers[3]);
    println!("f(16): 987");
}

fn load_bin(path: &str) -> Vec<u8> {
    let mut bin = vec![];
    OpenOptions::new()
        .create(false)
        .read(true)
        .open(path)
        .unwrap()
        .read_to_end(&mut bin).unwrap();
    bin
}

fn parse_opts<'a>() -> ArgMatches<'a> {
    App::new("Thorium VM")
        .version(crate_version!())
        .author(crate_authors!())
        .about("The Thorium VM")
        .arg(
            Arg::with_name("ram")
                .short("r")
                .long("ram")
                .value_name("ram")
                .help("Amount of RAM (defaults to STACK_SIZE + 128 B)"),
        )
        .arg(
            Arg::with_name("step")
                .short("s")
                .long("step")
                .help("Execute instructions step by step"),
        )
        .arg(
            Arg::with_name("mmap")
                .short("m")
                .long("mmap")
                .help("Print memory map information before start"),
        )
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
