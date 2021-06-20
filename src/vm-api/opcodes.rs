use std::convert::TryFrom;

#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Opcode {
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

impl Opcode {
    pub fn length(&self) -> u8 {
        return match self {
            Opcode::NOP => 1,
            Opcode::HALT => 1,
            Opcode::PANIC => 1,
            Opcode::LOAD => 6,
            Opcode::MOV => 3,
            Opcode::ADD => 3,
            Opcode::CMP => 3,
            Opcode::JMP => 5,
            Opcode::JE => 5,
        };
    }
}

impl From<u8> for Opcode {
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

impl TryFrom<&str> for Opcode {
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
        assert_eq!(Opcode::NOP, Opcode::from(u8::from(Opcode::NOP)));
        assert_eq!(Opcode::NOP, Opcode::try_from("NOP").unwrap());
    }

    #[test]
    fn test_halt() {
        assert_eq!(Opcode::HALT, Opcode::from(u8::from(Opcode::HALT)));
        assert_eq!(Opcode::HALT, Opcode::try_from("HALT").unwrap());
    }

    #[test]
    fn test_panic() {
        assert_eq!(Opcode::PANIC, Opcode::from(u8::from(Opcode::PANIC)));
        assert_eq!(Opcode::PANIC, Opcode::try_from("PANIC").unwrap());
    }

    #[test]
    fn test_load() {
        assert_eq!(Opcode::LOAD, Opcode::from(u8::from(Opcode::LOAD)));
        assert_eq!(Opcode::LOAD, Opcode::try_from("LOAD").unwrap());
    }

    #[test]
    fn test_mov() {
        assert_eq!(Opcode::MOV, Opcode::from(u8::from(Opcode::MOV)));
        assert_eq!(Opcode::MOV, Opcode::try_from("MOV").unwrap());
    }

    #[test]
    fn test_add() {
        assert_eq!(Opcode::ADD, Opcode::from(u8::from(Opcode::ADD)));
        assert_eq!(Opcode::ADD, Opcode::try_from("ADD").unwrap());
    }

    #[test]
    fn test_cmp() {
        assert_eq!(Opcode::CMP, Opcode::from(u8::from(Opcode::CMP)));
        assert_eq!(Opcode::CMP, Opcode::try_from("CMP").unwrap());
    }

    #[test]
    fn test_jmp() {
        assert_eq!(Opcode::JMP, Opcode::from(u8::from(Opcode::JMP)));
        assert_eq!(Opcode::JMP, Opcode::try_from("JMP").unwrap());
    }

    #[test]
    fn test_je() {
        assert_eq!(Opcode::JE, Opcode::from(u8::from(Opcode::JE)));
        assert_eq!(Opcode::JE, Opcode::try_from("JE").unwrap());
    }
}
