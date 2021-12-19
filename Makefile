all: test_tha test_thm demo_screen

release: install_release

#### tha
tha: src/asm/op.rs src/asm/constants.rs
	cargo build --bin tha

test_tha: src/asm/op.rs src/asm/constants.rs
	cargo test --bin tha

src/asm/op.rs: bin/*.lua bin/thi/*.lua src/common/instructions.thi
	bin/thi.lua src/common/instructions.thi

src/asm/constants.rs: bin/tha_generate_constants.sh src/common/registers.csv
	bin/tha_generate_constants.sh src/common/registers.csv > src/asm/constants.rs

#### thm
thm: src/vm/ops_array.h src/vm/cpu_internal_gen.h target/cmake-build-debug
	cmake --build target/cmake-build-debug
	ln -fs cmake-build-debug/thm target/thm

src/vm/ops_array.h: bin/*.lua bin/thi/*.lua src/common/instructions.thi
	bin/thi.lua src/common/instructions.thi

src/vm/cpu_internal_gen.h: bin/thm_generate_cpu_internal.sh src/common/registers.csv
	bin/thm_generate_cpu_internal.sh src/common/registers.csv > src/vm/cpu_internal_gen.h

target/cmake-build-debug: src/vm/CMakeLists.txt
	cmake -DCMAKE_BUILD_TYPE=Debug -Wdev -Wdeprecated -S src/vm -B target/cmake-build-debug

test_thm: thm target/rom.bin target/fact.bin target/fibonacci.bin target/fibonacci_rec.bin target/jumps.bin target/interrupts.bin target/call_convention.bin
	ctest --test-dir target/cmake-build-debug --output-on-failure
	bin/test-vm.sh target/cmake-build-debug/thm

target/%.bin: examples/%.a tha target/meta.a
	rm -f $@
	target/debug/tha -i target/meta.a -i $< -o $@

target/meta.a: thm
	target/cmake-build-debug/thm --gen-header > target/meta.a

target/rom.bin: tha target/meta.a src/common/rom.a
	target/debug/tha -i target/meta.a -i src/common/rom.a -o target/rom.bin

#### Demo
demo_screen: target/screen.bin target/rom.bin
	target/cmake-build-debug/thm --video master --rom target/rom.bin target/screen.bin

#### Maintenance stuff
clean_gen:
	rm -f src/asm/op.rs
	rm -f src/asm/constants.rs
	rm -f src/vm/cpu_internal_gen.h
	rm -f src/vm/ops_array.h
clean:
	rm -rf target
	rm -f src/asm/op.rs
	rm -f src/asm/constants.rs
	rm -f src/vm/cpu_internal_gen.h
	rm -f src/vm/ops_array.h
