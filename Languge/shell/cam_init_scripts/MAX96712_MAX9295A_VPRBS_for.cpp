// HW, Mar 2019
// VPRBS Setup
// MAX9295A (VPRBS Gen) -> MAX96712 (DUT) Link A
// MAX9295A MFP2 and MFP0 connected


// ---------------- MAX96712 setup ---------------------
// Link A only GMSL2
0x04,0x52,0x00,0x06,0xF1,
// Pipe 0 stream Link A pipe X only
0x04,0x52,0x00,0xF0,0xF0,
0x04,0x52,0x00,0xF1,0xFF,

// Enable VPRBS24 checker on pipe 0
0x04,0x52,0x01,0xDC,0x90,


// ---------------- MAX9295A setup ---------------------
// Parallel mode and PCLK input enable
0x04,0x80,0x00,0x07,0xF7, // MFP0 - PCLK input
// Connect MFP2 to MFP0 if using internal PCLK. Otherwise feed external clock on MFP0 and comment out below.
// Enable PCLK output
0x04,0x80,0x00,0x06,0xBF,
// MFP2 - ref clock out
0x04,0x80,0x00,0x03,0x07, 
// Set ref PLL clock = 74.25MHz
0x04,0x80,0x03,0xF0,0x71,

// Enable VPRBS
0x04,0x80,0x01,0xE5,0x80,



/* ---------------- Test Method ---------------------
- Check PCLKDET on Ser pipe X (0x102);
- Check VIDEO_LOCK on Des pipe 0 (0x1DC);

Fault Injection
- Enable error generation on MAX9295A (TX1 0x29 bit 4 or 5);
- Error rate can be changed from TX1 0x2A[5:4];
//0x04,0x80,0x00,0x2A,0x33,
//0x04,0x80,0x00,0x29,0x10,

- Check ERRB will be triggered on DUT MAX96712;
- Check 0x1DB (VPRBS_ERR) error count will come up.
*****************************************************/