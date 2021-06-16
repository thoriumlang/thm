#[derive(Debug, PartialEq)]
pub enum Opcode {
    /// The no-op code
    NOP,

    /// Halts the machine
    HALT,

    /// Load immediate integer value to register
    /// LOAD r, i32
    LOAD, // we may load integers from a direct address or from an address contained in a reg.

    /// Copies second's register value to first register
    /// Updates zero and negative flags according to the result
    /// MOV r0, r1
    MOV,

    /// Adds register values and stores result in r0
    /// Updates zero and negative flags according to the result
    /// ADD r0, r1
    ADD,

    // SUB,
    // DIV,
    // MUL,

    /// Compares register values and updates zero and negative flags accordingly.
    ///  - r0=r1 -> zero = true;  negative = false
    ///  - r0>r1 -> zero = false; negative = false
    ///  - r0<r1 -> zero = false; negative = true
    /// CMP r0, r1
    CMP,

    /// Jumps to address
    /// JMP address
    JMP, // we may jump to an address stored in a register

    /// Jumps to address is zero flag is set
    JE,

    // JNE,
    // JLT,
    // JGT,
    // JLE,
    // JGE,

    // PUSH
    // POP

    // CALL
    // RET

    /// Illegal instruction, halts the machine with an error
    PANIC,
}

impl From<u8> for Opcode {
    fn from(v: u8) -> Self {
        match v {
            0 => Self::NOP,
            1 => Self::HALT,
            2 => Self::LOAD,
            3 => Self::MOV,
            4 => Self::ADD,
            5 => Self::CMP,
            6 => Self::JMP,
            7 => Self::JE,
            _ => Self::PANIC,
        }
    }
}

impl From<Opcode> for u8 {
    fn from(v: Opcode) -> Self {
        v as u8
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_nop() {
        assert_eq!(Opcode::NOP, Opcode::from(u8::from(Opcode::NOP)))
    }

    #[test]
    fn test_halt() {
        assert_eq!(Opcode::HALT, Opcode::from(u8::from(Opcode::HALT)))
    }

    #[test]
    fn test_panic() {
        assert_eq!(Opcode::PANIC, Opcode::from(u8::from(Opcode::PANIC)))
    }

    #[test]
    fn test_load() {
        assert_eq!(Opcode::LOAD, Opcode::from(u8::from(Opcode::LOAD)))
    }

    #[test]
    fn test_mov() {
        assert_eq!(Opcode::MOV, Opcode::from(u8::from(Opcode::MOV)))
    }

    #[test]
    fn test_add() {
        assert_eq!(Opcode::ADD, Opcode::from(u8::from(Opcode::ADD)))
    }

    #[test]
    fn test_cmp() {
        assert_eq!(Opcode::CMP, Opcode::from(u8::from(Opcode::CMP)))
    }

    #[test]
    fn test_jmp() {
        assert_eq!(Opcode::JMP, Opcode::from(u8::from(Opcode::JMP)))
    }

    #[test]
    fn test_je() {
        assert_eq!(Opcode::JE, Opcode::from(u8::from(Opcode::JE)))
    }
}
