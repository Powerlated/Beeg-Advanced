use crate::helpers::readFileIntoVec;

pub struct Memory {
// main. non-IO memory
    pub BIOS: Vec <u8>,
    pub ROM:  Vec <u8>,
    pub eWRAM: [u8; 256 * 1024],
    pub iWRAM: [u8; 32  * 1024], 

// some IO registers that don't belong to peripherals
    pub ime: bool
}

impl Memory {
    pub fn new(romPath: String) -> Memory {
        return Memory {
            BIOS: readFileIntoVec(&"ROMs/NormattBIOS.gba".to_string()),
            ROM:  readFileIntoVec(&romPath),
            eWRAM: [0; 256 * 1024],
            iWRAM: [0; 32 * 1024],

            ime: false
        }
    }
}