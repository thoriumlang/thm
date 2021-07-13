all: it

release: install_release

tha:
	cargo build --bin tha
thm:
	cargo build --bin thm

t_tha: tha
	cargo test --bin tha
t_thm: thm
	cargo test --bin thm

rom: target/rom.bin
target/rom.bin: target/debug/tha src/rom.a
	target/debug/tha src/rom.a target/rom.bin

examples: target/fibonacci.bin target/fibonacci_rec.bin target/fact.bin
target/fibonacci.bin: target/debug/tha examples/fibonacci.a
	target/debug/tha examples/fibonacci.a target/fibonacci.bin
target/fibonacci_rec.bin: target/debug/tha examples/fibonacci_rec.a
	target/debug/tha examples/fibonacci_rec.a target/fibonacci_rec.bin
target/fact.bin: target/debug/tha examples/fact.a
	target/debug/tha examples/fact.a target/fact.bin

it: t_tha t_thm rom examples
	target/debug/thm --mmap target/rom.bin target/fibonacci.bin
	target/debug/thm --mmap target/rom.bin target/fibonacci_rec.bin

install_dev:
	cargo build
	sudo mv target/debug/thm /usr/local/bin/thm
	sudo chmod ugo+x /usr/local/bin/thm
	sudo mv target/debug/tha /usr/local/bin/tha
	sudo chmod ugo+x /usr/local/bin/tha

install_release:
	cargo build --release
	strip target/release/thm
	sudo mv target/release/thm /usr/local/bin/thm
	sudo chmod ugo+x /usr/local/bin/thm
	sudo mv target/release/tha /usr/local/bin/tha
	sudo chmod ugo+x /usr/local/bin/tha

op:
	./generate_ops.sh src/vm-api/op.csv > src/vm-api/op.rs

clean:
	cargo clean