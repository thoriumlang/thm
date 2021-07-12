use std::borrow::BorrowMut;
use std::fs::OpenOptions;
use std::io::{Read, Write};
use std::str::FromStr;
use std::sync::{Arc, RwLock};
use std::thread;

use clap::{App, Arg, ArgMatches, crate_authors, crate_version, SubCommand};

use cpu::CPU;
use vmlib::INTERRUPT_START;
use vmlib::MAX_ADDRESS;
use vmlib::MIN_RAM_SIZE;
use vmlib::REG_SP;
use vmlib::ROM_SIZE;
use vmlib::ROM_START;
use vmlib::STACK_LEN;
use vmlib::STACK_MAX_ADDRESS;
use vmlib::STACK_SIZE;
use vmlib::VIDEO_BUFFER_0;
use vmlib::VIDEO_BUFFER_1;
use vmlib::VIDEO_START;

use crate::memory::Memory;
use crate::rest_api::RestApi;
use crate::video::Video;

mod cpu;
mod memory;
mod rest_api;
mod video;

fn main() {
    let opts = parse_opts();
    if let Some(opts) = opts.subcommand_matches("run") {
        run(opts);
    } else if let Some(opts) = opts.subcommand_matches("meta") {
        meta(opts);
    }
}

fn run(opts: &ArgMatches) {
    let rom = load_bin(opts.value_of("rom").unwrap());
    let program: Vec<u8> = load_bin(opts.value_of("image").unwrap());
    let ram_size = opts.value_of("ram")
        .map(|s| usize::from_str(s).unwrap_or(MIN_RAM_SIZE + 256))
        .unwrap_or(MIN_RAM_SIZE + 256);

    if ram_size < MIN_RAM_SIZE + program.len() {
        panic!("Not enough RAM: {} < {}", ram_size, MIN_RAM_SIZE + program.len());
    }

    let mut memory = Memory::new(ram_size as u32, rom);
    if !memory.set_bytes((STACK_MAX_ADDRESS + 1) as u32, program.as_slice()) {
        panic!("Cannot copy program to ram");
    }

    if opts.is_present("mmap") {
        println!("RAM size:  \t{} Bytes", ram_size);
        println!("Stack:     \t{:#010x} - {:#010x} ({} Bytes) ({} 32-bits words)", 0, STACK_MAX_ADDRESS, STACK_SIZE, STACK_LEN);
        println!("Free:      \t{:#010x} - {:#010x} ({} Bytes)", STACK_MAX_ADDRESS + 1, ram_size - 1, ram_size - STACK_SIZE);
        println!("Video meta \t{:#010x} - {:#010x} ({} Bytes)", VIDEO_START, VIDEO_BUFFER_0 - 1, VIDEO_BUFFER_0 - VIDEO_START);
        println!("VBuffer0   \t{:#010x} - {:#010x} ({} Bytes)", VIDEO_BUFFER_0, VIDEO_BUFFER_1 - 1, VIDEO_BUFFER_1 - VIDEO_BUFFER_0);
        println!("VBuffer1   \t{:#010x} - {:#010x} ({} Bytes)", VIDEO_BUFFER_1, INTERRUPT_START - 1, INTERRUPT_START - VIDEO_BUFFER_1);
        println!("Interrupts \t{:#010x} - {:#010x} ({} Bytes)", INTERRUPT_START, ROM_START - 1, ROM_START - INTERRUPT_START);
        println!("ROM:       \t{:#010x} - {:#010x} ({} Bytes)", ROM_START, MAX_ADDRESS, ROM_SIZE);
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

    let executor_thread = match opts.is_present("step") {
        true => api,
        false => {
            let cpu = cpu.clone();
            let memory = memory.clone();
            thread::Builder::new().name("cpu".into()).spawn(move || {
                loop {
                    let mut cpu = cpu.write().unwrap();
                    let mut memory = memory.write().unwrap();
                    if !cpu.step(memory.borrow_mut()) {
                        break;
                    }
                }
                let cpu = cpu.read().unwrap();
                println!("f({}): {}", cpu.read_register(0), cpu.read_register(3));
            }).unwrap()
        }
    };

    if !opts.is_present("no-screen") {
        Video::new(memory.clone());
    }
    let _ = executor_thread.join();
}

fn meta(opts: &ArgMatches) {
    if opts.is_present("generate-file") {
        let mut file = OpenOptions::new()
            .create(true)
            .write(true)
            .open(opts.value_of("generate-file").unwrap())
            .unwrap();

        let mut data = String::new();
        data.push_str(format!("$__rom_start = {:#010x}\n", ROM_START).as_str());
        data.push_str(format!("$__video_start = {:#010x}\n", VIDEO_START).as_str());
        // data.push_str(format!("$__video_buffer_0_start = {:#010x}\n", VIDEO_BUFFER_0).as_str());
        // data.push_str(format!("$__video_buffer_1_start = {:#010x}\n", VIDEO_BUFFER_1).as_str());
        // data.push_str(format!("$__video_buffer_size = {:#010x}\n", VIDEO_BUFFER_SIZE).as_str());

        file.write_all(data.as_bytes()).unwrap();
    }
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
        .subcommand(SubCommand::with_name("meta")
            .about("Metadata about VM")
            .arg(Arg::with_name("generate-file")
                .short("g")
                .long("generate-file")
                .value_name("FILE")
                .help("Generate meta file"))
        )
        .subcommand(SubCommand::with_name("run")
            .about("Run VM")
            .arg(
                Arg::with_name("ram")
                    .short("r")
                    .long("ram")
                    .value_name("RAM")
                    .default_value("STACK_SIZE + 256 Bytes")
                    .help("Amount of RAM"),
            )
            .arg(
                Arg::with_name("debug")
                    .short("d")
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
                Arg::with_name("no-screen")
                    .short("s")
                    .long("no-screen")
                    .help("No video")
            )
            .arg(
                Arg::with_name("r0")
                    .short("0")
                    .help("Initial value of r0")
                    .default_value("0")
                    .required(false)
                    .takes_value(true)
            )
        )
        .get_matches()
}
