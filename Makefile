all: itest

release: install_release

build_dev:
	cargo build

test:
	cargo test

rom: test build_dev
	target/debug/tha src/rom.a target/rom.bin

itest: rom
	cargo run --bin tha examples/fibonacci.a target/fibonacci.bin
	cargo run --bin thm target/rom.bin target/fibonacci.bin

install_dev: build_dev
	sudo mv target/debug/thm /usr/local/bin/thm
	sudo chmod ugo+x /usr/local/bin/thm
	sudo mv target/debug/tha /usr/local/bin/tha
	sudo chmod ugo+x /usr/local/bin/tha

build_release: itest
	cargo build --release
	strip target/release/thm

install_release: build_release
	sudo mv target/release/thm /usr/local/bin/thm
	sudo chmod ugo+x /usr/local/bin/thm
	sudo mv target/release/tha /usr/local/bin/tha
	sudo chmod ugo+x /usr/local/bin/tha

clean:
	cargo clean