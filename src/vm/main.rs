use std::borrow::BorrowMut;
use std::fs::OpenOptions;
use std::io::Read;
use std::str::FromStr;
use std::sync::{Arc, RwLock};

use clap::{App, Arg, ArgMatches, crate_authors, crate_version};

use cpu::CPU;
use vmlib::{MIN_RAM_SIZE, REG_SP, ROM_START, STACK_LEN, STACK_MAX_ADDRESS, STACK_SIZE};

use crate::memory::Memory;
use crate::rest_api::RestApi;

mod cpu;
mod memory;
mod rest_api;

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

    let mut memory = Memory::new(ram_size as u32, rom);
    if !memory.set_bytes((STACK_MAX_ADDRESS + 1) as u32, program.as_slice()) {
        panic!("Cannot copy program to ram");
    }

    if opts.is_present("mmap") {
        println!("RAM size: {} Bytes", ram_size);
        println!("Stack:    {} Bytes ({} 32-bits words)", STACK_SIZE, STACK_LEN);
        println!("          {:#010x} - {:#010x}", 0, STACK_MAX_ADDRESS);
        println!("Free:     {} Bytes", ram_size - STACK_SIZE);
        println!("          {:#010x} - {:#010x}", STACK_MAX_ADDRESS + 1, ram_size - 1);
        memory.zones().iter().for_each(|z| println!("{}", z));
        memory.dump(0, 16);
        memory.dump((STACK_MAX_ADDRESS as u32) + 1, ROM_START as u32);
    }

    let mut cpu = CPU::new();
    cpu.write_register(REG_SP, STACK_MAX_ADDRESS as i32);
    cpu.write_register(0, opts.value_of("r0").map_or(0, |v| i32::from_str(v).unwrap_or_default()));
    cpu.start();

    if opts.is_present("step") {
        cpu.set_opts(cpu::Opts {
            print_op: true,
        });
        cpu.dump();
        println!();
    }

    let memory = Arc::new(RwLock::new(memory));
    let cpu = Arc::new(RwLock::new(cpu));
    let api = RestApi::new(cpu.clone(), memory.clone());

    if opts.is_present("step") {
        let _ = api.join();
    } else {
        loop {
            let mut cpu = cpu.write().unwrap();
            if !cpu.step(memory.write().unwrap().borrow_mut()) {
                break;
            }
        }
    }

    let cpu = cpu.read().unwrap();
    println!("f({}): {}", cpu.read_register(0), cpu.read_register(3));
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
                .default_value("STACK_SIZE + 128 Bytes")
                .help("Amount of RAM"),
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
        .arg(
            Arg::with_name("r0")
                .short("0")
                .help("Initial value of r0")
                .default_value("0")
                .required(false)
                .takes_value(true)
        )
        .get_matches()
}
