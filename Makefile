all: test_tha test_thm demo_screen

release: install_release

#### tha
tha: src/asm/op.rs
	cargo build --bin tha

test_tha: src/asm/op.rs
	cargo test --bin tha

src/asm/op.rs: src/common/op.csv
	bin/generate_ops.sh src/common/op.csv > src/asm/op.rs

#### thm
thm: target/cmake-build-debug
	cmake --build target/cmake-build-debug

target/cmake-build-debug: src/vm/CMakeLists.txt
	cmake -DCMAKE_BUILD_TYPE=Debug -Wdev -Wdeprecated -S src/vm -B target/cmake-build-debug

test_thm: thm target/fact.bin target/fibonacci.bin target/fibonacci_rec.bin
	ctest --test-dir target/cmake-build-debug --output-on-failure
	bin/test-vm.sh target/cmake-build-debug/thm

target/%.bin: examples/%.a tha target/meta.a
	rm -f $@
	target/debug/tha -i target/meta.a -i $< -o $@

target/meta.a: target/cmake-build-debug/thm
	target/cmake-build-debug/thm --gen-header > target/meta.a

target/rom.bin: tha target/meta.a src/common/rom.a
	target/debug/tha -i target/meta.a -i src/common/rom.a -o target/rom.bin

#### Demo
demo_screen: target/screen.bin
	target/cmake-build-debug/thm --video master target/screen.bin

#### Maintenance stuff
clean:
	rm -rf target
	rm src/asm/op.rs