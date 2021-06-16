all: test build_dev install_dev

release: test build_release install_release

test:
	cargo test

build_dev:
	cargo build

install_dev: build_dev
	sudo mv target/debug/thm /usr/local/bin/thm
	sudo chmod ugo+x /usr/local/bin/thm
	sudo mv target/debug/tha /usr/local/bin/tha
	sudo chmod ugo+x /usr/local/bin/tha

build_release:
	cargo build --release
	strip target/release/thm

install_release: build_release
	sudo mv target/release/thm /usr/local/bin/thm
	sudo chmod ugo+x /usr/local/bin/thm
	sudo mv target/release/tha /usr/local/bin/tha
	sudo chmod ugo+x /usr/local/bin/tha

clean:
	cargo clean