use std::fs::OpenOptions;
use std::io::{Read, Write};
use std::str::FromStr;

use clap::{App, Arg, ArgMatches, crate_authors, crate_version};

use cpu::CPU;
use vmlib::{MIN_RAM_SIZE, ROM_START, STACK_LEN, STACK_MAX_ADDRESS, STACK_SIZE};
use vmlib::op::Op;

use crate::memory_map::MemoryMap;

mod cpu;
mod memory_map;

fn main() {
    let opts = parse_opts();
    let rom_file = opts.value_of("rom").unwrap();
    let program_file = opts.value_of("image").unwrap();

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

    let ram_size = opts.value_of("ram")
        .map(|s| usize::from_str(s).unwrap_or(MIN_RAM_SIZE + 128))
        .unwrap_or(MIN_RAM_SIZE + 128);

    if ram_size < MIN_RAM_SIZE + program.len() {
        panic!("Not enough RAM: {} < {}", ram_size, MIN_RAM_SIZE + program.len());
    }

    let mut memory_map = MemoryMap::new(ram_size as u32, rom);

    // fixme this is a hack to start executing whatever comes after the stack...
    //  this should go to the rom
    let mut init = vec![Op::MOVI.bytecode(), 30];
    init.append(&mut ((STACK_MAX_ADDRESS + 1) as u32).to_be_bytes().to_vec());
    init.append(&mut vec![Op::MOVI.bytecode(), 31, 0, 0, 0, 0, Op::JA.bytecode(), 30, 31, 255]);
    if !memory_map.set_bytes(0, init.as_slice()) { panic!("cannot set {} bytes from 0", init.len()) };
    // fixme end of hack

    // copy program
    program.iter().enumerate().for_each(|(i, b)| {
        if !memory_map.set((STACK_MAX_ADDRESS + 1 + i) as u32, *b) { panic!("cannot set {}", STACK_MAX_ADDRESS + 1 + i) }
    });

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
    }else {
        cpu.run();
    }

    println!("f({}): {}", cpu.registers[0], cpu.registers[3]);
    println!("f(16): 987");
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
