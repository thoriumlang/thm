use std::convert::TryFrom;

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Op {
    /// The no-op code
    NOP,

    /// Halts the machine
    HALT,

    /// Illegal instruction, halts the machine with an error
    PANIC,

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
}

impl Op {
    pub fn length(&self) -> u8 {
        return match self {
            Op::NOP => 1,
            Op::HALT => 1,
            Op::PANIC => 1,
            Op::LOAD => 6,
            Op::MOV => 3,
            Op::ADD => 3,
            Op::CMP => 3,
            Op::JMP => 5,
            Op::JE => 5,
        };
    }

    pub fn bytecode(&self) -> u8 {
        *self as u8
    }
}

impl From<u8> for Op {
    fn from(v: u8) -> Self {
        match v {
            0 => Self::NOP,
            1 => Self::HALT,
            2 => Self::PANIC,
            3 => Self::LOAD,
            4 => Self::MOV,
            5 => Self::ADD,
            6 => Self::CMP,
            7 => Self::JMP,
            8 => Self::JE,
            _ => Self::PANIC,
        }
    }
}

impl TryFrom<&str> for Op {
    type Error = String;

    fn try_from(value: &str) -> Result<Self, Self::Error> {
        match value {
            "NOP" => Ok(Self::NOP),
            "HALT" => Ok(Self::HALT),
            "PANIC" => Ok(Self::PANIC),
            "LOAD" => Ok(Self::LOAD),
            "MOV" => Ok(Self::MOV),
            "ADD" => Ok(Self::ADD),
            "CMP" => Ok(Self::CMP),
            "JMP" => Ok(Self::JMP),
            "JE" => Ok(Self::JE),
            _ => Err(format!("Invalid op: {}", value).to_string()),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_nop() {
        assert_eq!(Op::NOP, Op::from(Op::NOP.bytecode()));
        assert_eq!(Op::NOP, Op::try_from("NOP").unwrap());
    }

    #[test]
    fn test_halt() {
        assert_eq!(Op::HALT, Op::from(Op::HALT.bytecode()));
        assert_eq!(Op::HALT, Op::try_from("HALT").unwrap());
    }

    #[test]
    fn test_panic() {
        assert_eq!(Op::PANIC, Op::from(Op::PANIC.bytecode()));
        assert_eq!(Op::PANIC, Op::try_from("PANIC").unwrap());
    }

    #[test]
    fn test_load() {
        assert_eq!(Op::LOAD, Op::from(Op::LOAD.bytecode()));
        assert_eq!(Op::LOAD, Op::try_from("LOAD").unwrap());
    }

    #[test]
    fn test_mov() {
        assert_eq!(Op::MOV, Op::from(Op::MOV.bytecode()));
        assert_eq!(Op::MOV, Op::try_from("MOV").unwrap());
    }

    #[test]
    fn test_add() {
        assert_eq!(Op::ADD, Op::from(Op::ADD.bytecode()));
        assert_eq!(Op::ADD, Op::try_from("ADD").unwrap());
    }

    #[test]
    fn test_cmp() {
        assert_eq!(Op::CMP, Op::from(Op::CMP.bytecode()));
        assert_eq!(Op::CMP, Op::try_from("CMP").unwrap());
    }

    #[test]
    fn test_jmp() {
        assert_eq!(Op::JMP, Op::from(Op::JMP.bytecode()));
        assert_eq!(Op::JMP, Op::try_from("JMP").unwrap());
    }

    #[test]
    fn test_je() {
        assert_eq!(Op::JE, Op::from(Op::JE.bytecode()));
        assert_eq!(Op::JE, Op::try_from("JE").unwrap());
    }
}
