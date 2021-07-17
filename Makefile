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

meta.a: target/meta.a
target/meta.a: t_thm
	rm -r target/meta.a
	target/debug/thm meta -g target/meta.a
rom: target/rom.bin
target/rom.bin: target/debug/tha src/rom.a
	target/debug/tha -i target/meta.a -i src/rom.a -o target/rom.bin

examples: target/fibonacci.bin target/fibonacci_rec.bin target/fact.bin
target/fibonacci.bin: target/debug/tha examples/fibonacci.a
	rm -f target/fibonacci.bin
	target/debug/tha -i target/meta.a -i examples/fibonacci.a -o target/fibonacci.bin
target/fibonacci_rec.bin: target/debug/tha examples/fibonacci_rec.a
	rm -f target/fibonacci_rec.bin
	target/debug/tha -i target/meta.a -i examples/fibonacci_rec.a -o target/fibonacci_rec.bin
target/fact.bin: target/debug/tha examples/fact.a
	rm -f target/fact.bin
	target/debug/tha -i target/meta.a -i examples/fact.a -o target/fact.bin

it: t_tha t_thm rom examples
	target/debug/thm run --mmap target/rom.bin target/fibonacci.bin     -016
	target/debug/thm run --mmap target/rom.bin target/fibonacci_rec.bin -016
	target/debug/thm run --mmap target/rom.bin target/fact.bin          -05

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