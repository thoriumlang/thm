use cpu::CPU;
use std::fs::OpenOptions;
use std::io::Read;

mod cpu;

fn main() {
    let mut file = OpenOptions::new()
        .create(false)
        .read(true)
        .open("target/fibonacci.bin")
        .unwrap();

    let mut bytes: Vec<u8> = vec![];
    file.read_to_end(&mut bytes).unwrap();

    let n = 5;

    let mut cpu = CPU::new();
    cpu.registers[0] = n;
    cpu.program = bytes;
    cpu.run();

    println!("fibo({}): {}", cpu.registers[0], cpu.registers[3])
}
